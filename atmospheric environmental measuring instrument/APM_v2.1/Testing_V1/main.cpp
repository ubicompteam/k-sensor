#include <iostream>
#include <string>
#include <signal.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>
#include <jsoncpp/json/json.h>
#include "i2c.h"
#include "http.h"
#include "tcp.h"

#define SLAVE_ADDRESS_1 0x57    				// contain 1 I2C 주소
#define SLAVE_ADDRESS_2 0x58    				// contain 2 I2C 주소
#define SLAVE_ADDRESS_3 0x59    				// contain 3 I2C 주소
#define SLAVE_ADDRESS_4 0x5A    				// Sensors I2C 주소
#define SLAVE_ADDRESS_5 0x5B    				// npm I2C 주소

// sejong
std::vector<bool> sejong_start_signal(9, false);		// 세종대 데이터 수신 함수 시작 신호
std::vector<int> sejong_data_counter(9, 0);			// 세종대 데이터 양 카운터
std::vector<std::string> trainDataPath(9, "");  		// 세종대 train 데이터 경로
std::vector<std::string> testDataPath(9, ""); 		// 세종대 test 데이터 경로
std::vector<std::string> state_path(9, ""); 			// 세종대 상태 데이터 경로
std::string sensor_parameter;  				// 세종대 센서 파라미터
std::vector<int> stopConditionValue(9, 0);  			// 세종대 데이터 수집 중지 조건 (갯수)
std::vector<int> counter_init(9, 0);  			// test data와 train data를 나누기 위한 초기화 변수
std::vector<int> change(9, 0);  				// test data와 train data를 나누기 위한 변수

// 프로그램 종료 시 클린업
void cleanup(int signum);
// 명령어 파싱 함수
bool parseCommand(const std::string &command, int &arduinoNum, std::string &action, const std::string& command_url);
// 현재 시간을 "YYYYMMDDhhmmss" 형식으로 반환하는 함수
std::string getCurrentTime();
// 1분마다 data send function
void SendDataEveryMinute(const std::string& apm2_url, const std::string& rpm_url);
// data parsing
void sejong_motor_data_get(const std::string& command_url);
// 세종대 데이터 전송 함수
void send_sejong_data(const std::string& data, int moduleNum);
// 세종대 요청 보내기
void send_sejong_request(const std::string& data, const std::string& path);
// 모듈 데이터 처리 함수
std::vector<std::string> process_module_data(const std::vector<std::string>& module);
// 모듈 데이터를 JSON 문자열로 변환
std::string create_sejong_json(const std::vector<std::string>& module_data);

int main() {
    signal(SIGINT, cleanup);

    // TCP 서버 시작
    TCPServer server(65432);
    server.start();

    std::string apm2_url = "http://114.71.220.59:2021/Mobius/justin/ss/data";
    std::string rpm_url = "http://114.71.220.59:2021/Mobius/justin/ss/RPMdata/la";
    std::string FanSpeed_url = "http://114.71.220.59:2021/Mobius/justin/ss/fanspeed";
    std::string command_url = "http://114.71.220.59:2021/Mobius/justin/ss/motor/la";
    
    std::cout << "start" << std::endl;

    // SendDataEveryMinute 함수를 별도 스레드로 실행
    std::thread dataSender(SendDataEveryMinute, apm2_url, rpm_url);

    int address, arduinoNum;
    std::string action;

    while (true) {
        std::string command = server.getReceivedData();
        command.erase(std::remove(command.begin(), command.end(), '\n'), command.end());
        command.erase(std::remove(command.begin(), command.end(), '\r'), command.end());

        if (command.empty()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        std::cout << "Received command: " << command << std::endl;

        if (!parseCommand(command, arduinoNum, action, command_url)) {
            std::cerr << "잘못된 명령 형식입니다. 형식: M?_ACTION" << std::endl;
            continue;
        }

        if (arduinoNum == 1) {
            address = SLAVE_ADDRESS_1;
        } else if (arduinoNum == 2) {
            address = SLAVE_ADDRESS_2;
        } else {
            std::cerr << "잘못된 아두이노 번호입니다. 1 또는 2를 입력하십시오." << std::endl;
            continue;
        }

        int file = openI2CDevice(address);
        if (file < 0) continue;

        if (action == "P1" || action == "P2" || action == "P3") {
            sendCommand(file, command);
            std::this_thread::sleep_for(std::chrono::seconds(10));
            std::string feedbackData = readData(file, 1);
            
            if( feedbackData == " "){
                std::cerr << "error one more" << std::endl; 
            	feedbackData = readData(file, 1);
            	std::this_thread::sleep_for(std::chrono::seconds(10));
            }
            
            send_value_to_server(FanSpeed_url, feedbackData);
        } else if ( action == "sejong"){
            sendCommand(file, sensor_parameter);
            std::this_thread::sleep_for(std::chrono::seconds(10));
            std::string feedbackData = readData(file, 1);
        } else {
            std::cerr << "알 수 없는 명령입니다: " << action << std::endl;
        }

        close(file);
    }

    // dataSender 스레드가 종료될 때까지 대기
    dataSender.join();
    cleanup(0);
    return 0;
}
// 프로그램 종료 시 클린업
void cleanup(int signum) {
    std::cout << "\n프로그램을 종료합니다." << std::endl;
    exit(0);
}

// 명령어 파싱 함수
bool parseCommand(const std::string &command, int &arduinoNum, std::string &action, const std::string& command_url) {
    if (command[0] == 'M'){
    	arduinoNum = command[1] - '0';
    	if (command.substr(3, 1) == "P") {
            action = command.substr(3, 2); // P1 or P2 or P3
	} else {
	    action = command.substr(3);
	}
    }else if(command[0] == '['){
        sejong_motor_data_get(command_url);
        arduinoNum = sensor_parameter[1]- '0';
        std::cerr << arduinoNum << std::endl;
    	action = "sejong";
    }
    else {
        return false;
    }

    return true;
}

// 현재 시간을 "YYYYMMDDhhmmss" 형식으로 반환하는 함수
std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    // tm 구조체로 변환
    struct tm timeInfo;
    localtime_r(&time, &timeInfo);
    
    // 시간 형식 맞추기: 년월일시분초
    std::stringstream timeStream;
    timeStream << std::setfill('0') << std::setw(4) << (1900 + timeInfo.tm_year)
               << std::setw(2) << (timeInfo.tm_mon + 1)
               << std::setw(2) << timeInfo.tm_mday
               << std::setw(2) << timeInfo.tm_hour
               << std::setw(2) << timeInfo.tm_min
               << std::setw(2) << timeInfo.tm_sec;
    
    return timeStream.str();
}

// 1분마다 data send function
void SendDataEveryMinute(const std::string& apm2_url, const std::string& rpm_url) {
    while (true) {
        std::string APM2_time = getCurrentTime();
        
        //rpm_data_get
        std::string rpm_data_original = get_value_from_server(rpm_url);
        std::cout << "RPM data fetched: " << rpm_data_original << std::endl;
        
// --------------------------------------------------------------------------------------------
        //rpm data process
        std::string RPM_time = "null", RPM_pm_data_b = "null", RPM_pm_data_a = "null";
    
        // rpm_data_original이 비어있지 않으면 데이터를 처리
        if (!rpm_data_original.empty()) {
	    std::stringstream ss(rpm_data_original);  // 문자열을 스트림으로 변환
	    std::string token;  // 쉼표로 구분된 데이터 조각을 저장할 변수
	    std::vector<std::string> rpm_data_original_list;  // 나눠진 데이터를 저장할 벡터

	    // 쉼표를 기준으로 데이터를 나누어 벡터에 추가
	    while (std::getline(ss, token, ',')) {
	        rpm_data_original_list.push_back(token);  // 벡터에 데이터를 추가
	    } 

	    // 벡터의 크기를 확인하고 각 변수에 값 할당
	    if (rpm_data_original_list.size() > 0) RPM_time = rpm_data_original_list[0];  // 첫 번째 데이터는 RPM_time에 할당
	    if (rpm_data_original_list.size() > 5) RPM_pm_data_b = rpm_data_original_list[5];  // 여섯 번째 데이터는 RPM_pm_data_b에 할당
	    if (rpm_data_original_list.size() > 6) RPM_pm_data_a = rpm_data_original_list[6];  // 일곱 번째 데이터는 RPM_pm_data_a에 할당
        }
        
// --------------------------------------------------------------------------------------------
        //read data to arduino (start)
        int file_1 = openI2CDevice(SLAVE_ADDRESS_1);
        std::string receiveData_1 = readData(file_1, 0);
        close(file_1);
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        //std::string receiveData_2 = "0,0,0,0,0,0";
	//std::string receiveData_3 = "0,0,0,0,0,0";
	//std::string receiveData_4 = "0,0,0,0,0,0,0,0";
	//std::string receiveData_5 = "0";
	int file_2 = openI2CDevice(SLAVE_ADDRESS_2);
	std::string receiveData_2 = readData(file_2, 0);
	close(file_2);
	int file_3 = openI2CDevice(SLAVE_ADDRESS_3);
	std::string receiveData_3 = readData(file_3, 0);
	close(file_3);
	int file_4 = openI2CDevice(SLAVE_ADDRESS_4);
	std::string receiveData_4 = readData(file_4, 0);
	close(file_4);
	int file_5 = openI2CDevice(SLAVE_ADDRESS_5);
	std::string receiveData_5 = readData(file_5, 0);
	close(file_5);
	// Combined data
	std::string combined_data = APM2_time + "," + RPM_time + "," + receiveData_1 + "," + receiveData_2 + "," + receiveData_3  +  "," + receiveData_4 + "," + receiveData_5  + "," + RPM_pm_data_b + "," + RPM_pm_data_a;
	std::cout << "Combined data: " << combined_data << std::endl;

	// 원래 순서와 원하는 순서를 정의
	std::vector<std::string> original_order = {
		"APM_datetime", "RPM_datetime", "pwm1_1", "pwm1_2", "pwm1_3", "pm1_1", "pm1_2", "pm1_3",
		"pwm2_1", "pwm2_2", "pwm2_3", "pm2_1", "pm2_2", "pm2_3",
		"pwm3_1", "pwm3_2", "pwm3_3", "pm3_1", "pm3_2", "pm3_3",
		"temp", "humi", "o3", "co", "no2", "so2",
		"wind_d", "wind_s", "npm", "rpm before correction", "rpm after correction"
	};

	std::vector<std::string> desired_order = {
		"APM_datetime", "RPM_datetime", "rpm before correction", "rpm after correction",
		"pwm1_1", "pwm1_2", "pwm1_3", "pwm2_1", "pwm2_2", "pwm2_3",
		"pwm3_1", "pwm3_2", "pwm3_3", "npm", "pm1_1", "pm1_2", "pm1_3",
		"pm2_1", "pm2_2", "pm2_3", "pm3_1", "pm3_2", "pm3_3",
		"temp", "humi", "o3", "co", "no2", "so2", "wind_d", "wind_s"
	};

	// original_order의 각 항목의 인덱스를 저장하는 맵 생성
	std::map<std::string, int> index_map;
	for (int i = 0; i < original_order.size(); ++i) {
		index_map[original_order[i]] = i;
	}

	// combined_data를 쉼표로 나누어 데이터 리스트를 생성
	std::vector<std::string> data_list;
	std::stringstream ss(combined_data);
	std::string item;
	while (std::getline(ss, item, ',')) {
		data_list.push_back(item);
	}

	// desired_order에 따라 데이터를 재정렬
	std::vector<std::string> reordered_data;
	for (const auto& col : desired_order) {
		if (index_map.find(col) != index_map.end() && index_map[col] < data_list.size()) {
		    reordered_data.push_back(data_list[index_map[col]]);
		} else {
		    reordered_data.push_back("null"); // 데이터를 찾을 수 없으면 'null' 추가
		}
	}

	// 재정렬된 데이터를 하나의 문자열로 합침
	std::string reordered_combined_data;
	for (size_t i = 0; i < reordered_data.size(); ++i) {
		reordered_combined_data += reordered_data[i];
		if (i < reordered_data.size() - 1) reordered_combined_data += ",";
	}

	// 결과 출력
	std::cout << "Reordered data: " << reordered_combined_data << std::endl;

// --------------------------------------------------------------------------------------------
	//apm2 data send
	send_value_to_server(apm2_url, reordered_combined_data);
	
	for(int i = 0; i < 9; i++){
		if(sejong_start_signal[i] == true){
			send_sejong_data(reordered_combined_data, i);
		}
	}
	
        // 1분(60초) 동안 대기
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
}


void sejong_motor_data_get(const std::string& command_url) {
    std::string json_str = get_value_from_server(command_url);
    
    // JSON 파싱을 위한 Json::CharReaderBuilder 사용
    Json::CharReaderBuilder readerBuilder;
    Json::Value root;
    std::istringstream s(json_str);
    std::string errs;

    // JSON 파싱
    if (Json::parseFromStream(readerBuilder, s, &root, &errs)) {
        // JSON 파싱 후 처리
        sensor_parameter = root["m2m:cin"]["con"]["sensor_parameter"].asString();

        // 파싱된 값 출력
        std::cout << "Sensor Parameter: " << sensor_parameter << std::endl;
        std::cout << "Train Data Path: " << root["m2m:cin"]["con"]["trainDataPath"].asString() << std::endl;
        std::cout << "Test Data Path: " << root["m2m:cin"]["con"]["testDataPath"].asString() << std::endl;
        std::cout << "State Path: " << root["m2m:cin"]["con"]["state_path"].asString() << std::endl;
        std::cout << "Stop Condition: " << root["m2m:cin"]["con"]["sensingStopCondition"]["stopCondition"].asString() << std::endl;
        std::cout << "Stop Condition Value: " << root["m2m:cin"]["con"]["sensingStopCondition"]["stopConditionValue"].asInt() << std::endl;


        if (sensor_parameter[1] == '1') {
            if (sensor_parameter[4] == '1') {
                trainDataPath[0] = root["m2m:cin"]["con"]["trainDataPath"].asString();
                testDataPath[0] = root["m2m:cin"]["con"]["testDataPath"].asString();
                state_path[0] = root["m2m:cin"]["con"]["state_path"].asString();
                stopConditionValue[0] = root["m2m:cin"]["con"]["sensingStopCondition"]["stopConditionValue"].asInt();

                sejong_data_counter[0] = counter_init[0] = stopConditionValue[0];
                change[0] = 0;

                // 데이터 수집 시작 신호
                sejong_start_signal[0] = true;
            }
            else if (sensor_parameter[4] == '2') {
                trainDataPath[1] = root["m2m:cin"]["con"]["trainDataPath"].asString();
                testDataPath[1] = root["m2m:cin"]["con"]["testDataPath"].asString();
                state_path[1] = root["m2m:cin"]["con"]["state_path"].asString();
                stopConditionValue[1] = root["m2m:cin"]["con"]["sensingStopCondition"]["stopConditionValue"].asInt();

                sejong_data_counter[1] = counter_init[1] = stopConditionValue[1];
                change[1] = 0;

                // 데이터 수집 시작 신호
                sejong_start_signal[1] = true;
            }
            else if (sensor_parameter[4] == '3') {
                trainDataPath[2] = root["m2m:cin"]["con"]["trainDataPath"].asString();
                testDataPath[2] = root["m2m:cin"]["con"]["testDataPath"].asString();
                state_path[2] = root["m2m:cin"]["con"]["state_path"].asString();
                stopConditionValue[2] = root["m2m:cin"]["con"]["sensingStopCondition"]["stopConditionValue"].asInt();

                sejong_data_counter[2] = counter_init[2] = stopConditionValue[2];
                change[2] = 0;

                // 데이터 수집 시작 신호
                sejong_start_signal[2] = true;
            }
        }
        else if (sensor_parameter[1] == '2') {
            if (sensor_parameter[4] == '1') {
                trainDataPath[3] = root["m2m:cin"]["con"]["trainDataPath"].asString();
                testDataPath[3] = root["m2m:cin"]["con"]["testDataPath"].asString();
                state_path[3] = root["m2m:cin"]["con"]["state_path"].asString();
                stopConditionValue[3] = root["m2m:cin"]["con"]["sensingStopCondition"]["stopConditionValue"].asInt();

                sejong_data_counter[3] = counter_init[3] = stopConditionValue[3];
                change[3] = 0;

                // 데이터 수집 시작 신호
                sejong_start_signal[3] = true;
            }
            else if (sensor_parameter[4] == '2') {
                trainDataPath[4] = root["m2m:cin"]["con"]["trainDataPath"].asString();
                testDataPath[4] = root["m2m:cin"]["con"]["testDataPath"].asString();
                state_path[4] = root["m2m:cin"]["con"]["state_path"].asString();
                stopConditionValue[4] = root["m2m:cin"]["con"]["sensingStopCondition"]["stopConditionValue"].asInt();

                sejong_data_counter[4] = counter_init[4] = stopConditionValue[4];
                change[4] = 0;

                // 데이터 수집 시작 신호
                sejong_start_signal[4] = true;
            }
            else if (sensor_parameter[4] == '3') {
                trainDataPath[5] = root["m2m:cin"]["con"]["trainDataPath"].asString();
                testDataPath[5] = root["m2m:cin"]["con"]["testDataPath"].asString();
                state_path[5] = root["m2m:cin"]["con"]["state_path"].asString();
                stopConditionValue[5] = root["m2m:cin"]["con"]["sensingStopCondition"]["stopConditionValue"].asInt();

                sejong_data_counter[5] = counter_init[5] = stopConditionValue[5];
                change[5] = 0;

                // 데이터 수집 시작 신호
                sejong_start_signal[5] = true;
            }
        }
        else if (sensor_parameter[1] == '3') {
            if (sensor_parameter[4] == '1') {
                trainDataPath[6] = root["m2m:cin"]["con"]["trainDataPath"].asString();
                testDataPath[6] = root["m2m:cin"]["con"]["testDataPath"].asString();
                state_path[6] = root["m2m:cin"]["con"]["state_path"].asString();
                stopConditionValue[6] = root["m2m:cin"]["con"]["sensingStopCondition"]["stopConditionValue"].asInt();

                sejong_data_counter[6] = counter_init[6] = stopConditionValue[6];
                change[6] = 0;

                // 데이터 수집 시작 신호
                sejong_start_signal[6] = true;
            }
            else if (sensor_parameter[4] == '2') {
                trainDataPath[7] = root["m2m:cin"]["con"]["trainDataPath"].asString();
                testDataPath[7] = root["m2m:cin"]["con"]["testDataPath"].asString();
                state_path[7] = root["m2m:cin"]["con"]["state_path"].asString();
                stopConditionValue[7] = root["m2m:cin"]["con"]["sensingStopCondition"]["stopConditionValue"].asInt();

                sejong_data_counter[7] = counter_init[7] = stopConditionValue[7];
                change[7] = 0;

                // 데이터 수집 시작 신호
                sejong_start_signal[7] = true;
            }
            else if (sensor_parameter[4] == '3') {
                trainDataPath[8] = root["m2m:cin"]["con"]["trainDataPath"].asString();
                testDataPath[8] = root["m2m:cin"]["con"]["testDataPath"].asString();
                state_path[8] = root["m2m:cin"]["con"]["state_path"].asString();
                stopConditionValue[8] = root["m2m:cin"]["con"]["sensingStopCondition"]["stopConditionValue"].asInt();

                sejong_data_counter[8] = counter_init[8] = stopConditionValue[8];
                change[8] = 0;

                // 데이터 수집 시작 신호
                sejong_start_signal[8] = true;
            }
        }

        std::cout << "sejong_motor_data_get() finished" << std::endl;
    } else {
        std::cout << "Failed to parse JSON" << std::endl;
    }
}
// 세종대 요청 보내기
void send_sejong_request(const std::string& data, const std::string& path) {
    send_value_to_server(path, data);
    std::cout << "Sending data: " << data << " to " << path << std::endl;
}



// 모듈 데이터 처리 함수
std::vector<std::string> process_module_data(const std::vector<std::string>& module) {
    std::vector<std::string> module_arr;
    for (const auto& item : module) {
        module_arr.push_back(item);  // 변환이 필요 없다면 그대로 추가
    }
    return module_arr;
}

// 모듈 데이터를 JSON 문자열로 변환
std::string create_sejong_json(const std::vector<std::string>& module_data) {
    Json::Value sejong_json;
    for (const auto& item : module_data) {
        sejong_json.append(item);  // JSON 배열에 데이터 추가
    }
    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, sejong_json);  // JSON 문자열 반환
}

// 세종대 데이터 전송 함수
void send_sejong_data(const std::string& data, int moduleNum) {
    std::vector<std::string> where_list;
    std::stringstream ss(data);
    std::string temp;
    while (std::getline(ss, temp, ',')) {
        where_list.push_back(temp);  // ','로 구분된 데이터를 분리하여 리스트에 저장
    }

    // 모듈 0에 대한 처리
    if (moduleNum == 0) {
        if (sejong_data_counter[moduleNum] == 0) {
            sejong_start_signal[moduleNum] = false;
            send_sejong_request("Ready", state_path[moduleNum]);
            std::cout << "send_sejong_data " << moduleNum << " module finished" << std::endl;
            return;
        }
        std::vector<std::string> module_1 = { where_list[0], where_list[4], where_list[14], where_list[23], where_list[24], where_list[25], where_list[26], where_list[27], where_list[28], where_list[29], where_list[30], where_list[3] };
        std::vector<std::string> module_1_arr = process_module_data(module_1);
        std::string con0_1_sejong_str = create_sejong_json(module_1_arr);
        if (counter_init[moduleNum] == 0) {
            change[moduleNum] = 1;
        }
        if (change[moduleNum] == 0) {
            send_sejong_request(con0_1_sejong_str, trainDataPath[moduleNum]);
            counter_init[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, train remaining data: " << counter_init[moduleNum] << std::endl;
        } else {
            send_sejong_request(con0_1_sejong_str, testDataPath[moduleNum]);
            sejong_data_counter[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, test remaining data: " << sejong_data_counter[moduleNum] << std::endl;
        }
    }

    // 1번 모듈에 대한 처리
    else if (moduleNum == 1) {
        if (sejong_data_counter[moduleNum] == 0) {
            sejong_start_signal[moduleNum] = false;
            send_sejong_request("Ready", state_path[moduleNum]);
            std::cout << "send_sejong_data " << moduleNum << " module finished" << std::endl;
            return;
        }
        std::vector<std::string> module_2 = { where_list[0], where_list[5], where_list[15], where_list[23], where_list[24], where_list[25], where_list[26], where_list[27], where_list[28], where_list[29], where_list[30], where_list[3] };
        std::vector<std::string> module_2_arr = process_module_data(module_2);
        std::string con0_2_sejong_str = create_sejong_json(module_2_arr);
        if (counter_init[moduleNum] == 0) {
            change[moduleNum] = 1;
        }
        if (change[moduleNum] == 0) {
            send_sejong_request(con0_2_sejong_str, trainDataPath[moduleNum]);
            counter_init[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, train remaining data: " << counter_init[moduleNum] << std::endl;
        } else {
            send_sejong_request(con0_2_sejong_str, testDataPath[moduleNum]);
            sejong_data_counter[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, test remaining data: " << sejong_data_counter[moduleNum] << std::endl;
        }
    }
    // 2번 모듈에 대한 처리
    else if (moduleNum == 2) {
        if (sejong_data_counter[moduleNum] == 0) {
            sejong_start_signal[moduleNum] = false;
            send_sejong_request("Ready", state_path[moduleNum]);
            std::cout << "send_sejong_data " << moduleNum << " module finished" << std::endl;
            return;
        }
        std::vector<std::string> module_3 = { where_list[0], where_list[6], where_list[16], where_list[23], where_list[24], where_list[25], where_list[26], where_list[27], where_list[28], where_list[29], where_list[30], where_list[3] };
        std::vector<std::string> module_3_arr = process_module_data(module_3);
        std::string con0_3_sejong_str = create_sejong_json(module_3_arr);
        if (counter_init[moduleNum] == 0) {
            change[moduleNum] = 1;
        }
        if (change[moduleNum] == 0) {
            send_sejong_request(con0_3_sejong_str, trainDataPath[moduleNum]);
            counter_init[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, train remaining data: " << counter_init[moduleNum] << std::endl;
        } else {
            send_sejong_request(con0_3_sejong_str, testDataPath[moduleNum]);
            sejong_data_counter[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, test remaining data: " << sejong_data_counter[moduleNum] << std::endl;
        }
    }
    // 3번 모듈에 대한 처리
    else if (moduleNum == 3) {
        if (sejong_data_counter[moduleNum] == 0) {
            sejong_start_signal[moduleNum] = false;
            send_sejong_request("Ready", state_path[moduleNum]);
            std::cout << "send_sejong_data " << moduleNum << " module finished" << std::endl;
            return;
        }
        std::vector<std::string> module_4 = { where_list[0], where_list[7], where_list[17], where_list[23], where_list[24], where_list[25], where_list[26], where_list[27], where_list[28], where_list[29], where_list[30], where_list[3] };
        std::vector<std::string> module_4_arr = process_module_data(module_4);
        std::string con0_4_sejong_str = create_sejong_json(module_4_arr);
        if (counter_init[moduleNum] == 0) {
            change[moduleNum] = 1;
        }
        if (change[moduleNum] == 0) {
            send_sejong_request(con0_4_sejong_str, trainDataPath[moduleNum]);
            counter_init[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, train remaining data: " << counter_init[moduleNum] << std::endl;
        } else {
            send_sejong_request(con0_4_sejong_str, testDataPath[moduleNum]);
            sejong_data_counter[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, test remaining data: " << sejong_data_counter[moduleNum] << std::endl;
        }
    }
    // 4번 모듈에 대한 처리
    else if (moduleNum == 4) {
        if (sejong_data_counter[moduleNum] == 0) {
            sejong_start_signal[moduleNum] = false;
            send_sejong_request("Ready", state_path[moduleNum]);
            std::cout << "send_sejong_data " << moduleNum << " module finished" << std::endl;
            return;
        }
        std::vector<std::string> module_5 = { where_list[0], where_list[8], where_list[18], where_list[23], where_list[24], where_list[25], where_list[26], where_list[27], where_list[28], where_list[29], where_list[30], where_list[3] };
        std::vector<std::string> module_5_arr = process_module_data(module_5);
        std::string con0_5_sejong_str = create_sejong_json(module_5_arr);
        if (counter_init[moduleNum] == 0) {
            change[moduleNum] = 1;
        }
        if (change[moduleNum] == 0) {
            send_sejong_request(con0_5_sejong_str, trainDataPath[moduleNum]);
            counter_init[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, train remaining data: " << counter_init[moduleNum] << std::endl;
        } else {
            send_sejong_request(con0_5_sejong_str, testDataPath[moduleNum]);
            sejong_data_counter[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, test remaining data: " << sejong_data_counter[moduleNum] << std::endl;
        }
    }
    // 5번 모듈에 대한 처리
    else if (moduleNum == 5) {
        if (sejong_data_counter[moduleNum] == 0) {
            sejong_start_signal[moduleNum] = false;
            send_sejong_request("Ready", state_path[moduleNum]);
            std::cout << "send_sejong_data " << moduleNum << " module finished" << std::endl;
            return;
        }
        std::vector<std::string> module_6 = { where_list[0], where_list[9], where_list[19], where_list[23], where_list[24], where_list[25], where_list[26], where_list[27], where_list[28], where_list[29], where_list[30], where_list[3] };
        std::vector<std::string> module_6_arr = process_module_data(module_6);
        std::string con0_6_sejong_str = create_sejong_json(module_6_arr);
        if (counter_init[moduleNum] == 0) {
            change[moduleNum] = 1;
        }
        if (change[moduleNum] == 0) {
            send_sejong_request(con0_6_sejong_str, trainDataPath[moduleNum]);
            counter_init[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, train remaining data: " << counter_init[moduleNum] << std::endl;
        } else {
            send_sejong_request(con0_6_sejong_str, testDataPath[moduleNum]);
            sejong_data_counter[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, test remaining data: " << sejong_data_counter[moduleNum] << std::endl;
        }
    }
    // 6번 모듈에 대한 처리
    else if (moduleNum == 6) {
        if (sejong_data_counter[moduleNum] == 0) {
            sejong_start_signal[moduleNum] = false;
            send_sejong_request("Ready", state_path[moduleNum]);
            std::cout << "send_sejong_data " << moduleNum << " module finished" << std::endl;
            return;
        }
        std::vector<std::string> module_7 = { where_list[0], where_list[10], where_list[20], where_list[23], where_list[24], where_list[25], where_list[26], where_list[27], where_list[28], where_list[29], where_list[30], where_list[3] };
        std::vector<std::string> module_7_arr = process_module_data(module_7);
        std::string con0_7_sejong_str = create_sejong_json(module_7_arr);
        if (counter_init[moduleNum] == 0) {
            change[moduleNum] = 1;
        }
        if (change[moduleNum] == 0) {
            send_sejong_request(con0_7_sejong_str, trainDataPath[moduleNum]);
            counter_init[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, train remaining data: " << counter_init[moduleNum] << std::endl;
        } else {
            send_sejong_request(con0_7_sejong_str, testDataPath[moduleNum]);
            sejong_data_counter[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, test remaining data: " << sejong_data_counter[moduleNum] << std::endl;
        }
    }
    // 7번 모듈에 대한 처리

    else if (moduleNum == 7) {
        if (sejong_data_counter[moduleNum] == 0) {
            sejong_start_signal[moduleNum] = false;
            send_sejong_request("Ready", state_path[moduleNum]);
            std::cout << "send_sejong_data " << moduleNum << " module finished" << std::endl;
            return;
        }
        std::vector<std::string> module_8 = { where_list[0], where_list[11], where_list[21], where_list[23], where_list[24], where_list[25], where_list[26], where_list[27], where_list[28], where_list[29], where_list[30], where_list[3] };
        std::vector<std::string> module_8_arr = process_module_data(module_8);
        std::string con0_8_sejong_str = create_sejong_json(module_8_arr);
        if (counter_init[moduleNum] == 0) {
            change[moduleNum] = 1;
        }
        if (change[moduleNum] == 0) {
            send_sejong_request(con0_8_sejong_str, trainDataPath[moduleNum]);
            counter_init[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, train remaining data: " << counter_init[moduleNum] << std::endl;
        } else {
            send_sejong_request(con0_8_sejong_str, testDataPath[moduleNum]);
            sejong_data_counter[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, test remaining data: " << sejong_data_counter[moduleNum] << std::endl;
        }
    }
    // 8번 모듈에 대한 처리
    else if (moduleNum == 8) {
        if (sejong_data_counter[moduleNum] == 0) {
            sejong_start_signal[moduleNum] = false;
            send_sejong_request("Ready", state_path[moduleNum]);
            std::cout << "send_sejong_data " << moduleNum << " module finished" << std::endl;
            return;
        }
        std::vector<std::string> module_9 = { where_list[0], where_list[12], where_list[22], where_list[23], where_list[24], where_list[25], where_list[26], where_list[27], where_list[28], where_list[29], where_list[30], where_list[3] };
        std::vector<std::string> module_9_arr = process_module_data(module_9);
        std::string con0_9_sejong_str = create_sejong_json(module_9_arr);
        if (counter_init[moduleNum] == 0) {
            change[moduleNum] = 1;
        }
        if (change[moduleNum] == 0) {
            send_sejong_request(con0_9_sejong_str, trainDataPath[moduleNum]);
            counter_init[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, train remaining data: " << counter_init[moduleNum] << std::endl;
        } else {
            send_sejong_request(con0_9_sejong_str, testDataPath[moduleNum]);
            sejong_data_counter[moduleNum]--;
            std::cout << "send_sejong_data " << moduleNum << " module finished, test remaining data: " << sejong_data_counter[moduleNum] << std::endl;
        }
    }
}
