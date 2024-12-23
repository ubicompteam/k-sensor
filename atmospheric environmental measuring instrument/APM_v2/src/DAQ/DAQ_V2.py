# -*- coding: utf-8 -*-
# pip install schedule
import socket
from datetime import datetime
import serial
import requests
import time
import json
import signal
import sys
import threading
from collections import OrderedDict

# 기존 URL 및 헤더 설정
apm2_url = 'http://114.71.220.59:2021/Mobius/Ksensor_ubicomp_2/data'
data_send_url = 'http://114.71.220.59:2021/Mobius/Ksensor_ubicomp_2/motor'
rpm_url = "http://114.71.220.59:2021/Mobius/Ksensor_ubicomp_2/RPMData"
FanSpeed_url = "http://114.71.220.59:2021/Mobius/Ksensor_ubicomp_2/FanSpeed"

# 세종대 주소
#sejong_url = "http://203.250.148.120:20519/Mobius/service12"

#실험용
#sejong_url = "http://114.71.220.59:2021/Mobius/justin"


# 세종대 명령어 올라오는 URL
sejong_url_motor = "http://114.71.220.59:2021/Mobius/Ksensor_ubicomp_2/motor/la"

apm2_headers = {
    'Accept': 'application/json',
    'X-M2M-RI': '12345',
    'X-M2M-Origin': 'SKsensor_ubicomp_2',
    'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
}

sejong_headers = {
    'Accept': 'application/json',
    'X-M2M-RI': '12345',
    'X-M2M-Origin': 'SCpDwFJHF71',
    'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
}

# 시리얼 포트 및 보드레이트 설정
serial_port_0 = '/dev/ttyM1'
serial_port_1 = '/dev/ttyM2'
serial_port_2 = '/dev/ttyM3'
serial_port_3 = '/dev/ttySensor'
serial_port_4 = '/dev/ttyNPM'

baudrate = 9600

# 소켓 설정
port = 65432

#전역 변수 설정
is_restoring_fan_state = False  # True: 복원 중, False: 복원 중이 아님
retry = True                    # 키보드 인터럽트 발생 시 서버 종료를 위한 변수

sejong_start_signal = [False, False, False, False, False, False, False, False, False]       # 세종대 데이터 수신 함수 시작 신호
sejong_data_counter = [0,0,0,0,0,0,0,0,0]                                                   # 세종대 데이터 양 카운터
trainDataPath = ["","","","","","","","",""]                                                # 세종대 train 데이터 경로
testDataPath = ["","","","","","","","",""]                                                 # 세종대 test 데이터 경로
state_path = ["","","","","","","","",""]                                                   # 세종대 상태 데이터 경로
global sensor_parameter                                                                     # 세종대 센서 파라미터
stopConditionValue = [0,0,0,0,0,0,0,0,0]                                                    # 세종대 데이터 수집 중지 조건 (갯수)
counter_init = [0,0,0,0,0,0,0,0,0]                                                          # test data와 train data를 나누기 위한 초기화 변수
change = [0,0,0,0,0,0,0,0,0]                                                                                 # test data와 train data를 나누기 위한 변수

# 임계영역 보호를 위한 Lock 생성 ( 데이터 수집과 제어 명령어 전송과의 충돌 방지 )
lock = threading.Lock()


# 시리얼 연결 초기화
try:
    ser0 = serial.Serial(serial_port_0, baudrate=baudrate, timeout=1)
    ser1 = serial.Serial(serial_port_1, baudrate=baudrate, timeout=1)
    ser2 = serial.Serial(serial_port_2, baudrate=baudrate, timeout=1)
    ser3 = serial.Serial(serial_port_3, baudrate=baudrate, timeout=1)
    ser4 = serial.Serial(serial_port_4, baudrate=baudrate, timeout=1)
    print(f'Connected to {ser0.name},{ser1.name},{ser2.name},{ser3.name},{ser4.name}')
except Exception as err:
    print(f"Serial error: {err}")
    sys.exit(1)

# 아두이노 버퍼 비워주는 함수
def clear_serial_buffer():
    ser0.reset_input_buffer()
    ser1.reset_input_buffer()
    ser2.reset_input_buffer()
    ser3.reset_input_buffer()
    ser4.reset_input_buffer()
    print("Serial buffers cleared")
    
# 아두이노 각각의 버퍼 비워주는 함수    
def each_serial_buffer(each_serial_port):
    each_serial_port.reset_input_buffer()
    print("Serial buffers cleared")

# 아두이노 처음 시작했을 때 초기화함수
def init_serial():
    ser0.write(b'0')
    ser1.write(b'0')
    ser2.write(b'0')
    ser3.write(b'0')
    ser4.write(b'0')
    clear_serial_buffer()
    print("loading....")
    time.sleep(5)

# 아두이노에서 데이터 수신 오류시 재시도 함수    
def try_send_data(try_serial_port):
    try:
        each_serial_buffer(try_serial_port)
        try_serial_port.write(b'start')
        time.sleep(5)
        data = try_serial_port.readline().decode('utf-8').strip()
        if data == "":
            return "null"
        return data
    except Exception as try_err:
        print(f"Serial send error: {try_err}")
        return "null"
    
# 아두이노에서 피드백 수신 오류시 재시도 함수
def try_send_feedback(try_serial_port, data):
    try:
        each_serial_buffer(try_serial_port)
        try_serial_port.write(data.encode('utf-8'))
        time.sleep(10)
        data = try_serial_port.readline().decode('utf-8').strip()
        if data == "":
            return "check the fan go to 7 floor"
        return data
    except Exception as try_err:
        print(f"Serial send error: {try_err}")
        return "check the fan go to 7 floor"
    
# 시리얼 데이터 수신 함수
def read_serial_data():
    try:
        # 각 시리얼 포트에 대해 데이터가 수신되었는지 확인
        if ser0.in_waiting > 0:
            data0 = ser0.readline().decode('utf-8').strip()
        else:
            # 만약 데이터가 없다면 재시도
            data0 = try_send_data(ser0)

        if ser1.in_waiting > 0:
            data1 = ser1.readline().decode('utf-8').strip()
        else:
            data1 = try_send_data(ser1)

        if ser2.in_waiting > 0:
            data2 = ser2.readline().decode('utf-8').strip()
        else:
            data2 = try_send_data(ser2)

        if ser3.in_waiting > 0:
            data3 = ser3.readline().decode('utf-8').strip()
        else:
            data3 = try_send_data(ser3)

        if ser4.in_waiting > 0:
            data4 = ser4.readline().decode('utf-8').strip()
        else:
            data4 = try_send_data(ser4)

        # 수신된 데이터를 포맷팅하여 반환
        data = "{}{}{}{},{}".format(data0, data1, data2, data3, data4)
        if data0 == " , " or data1 == " , " or data2 == " , " or data3 == " , " or data4 == " , ":
            return "null"
        elif data == "null":
            return "null"
        return data
    except Exception as read_err:
        print(f"Serial read error: {read_err}")
        return "error"

# 팬 속도 제어 함수        
def CommandFanControl(data):
    
    lock.acquire()
    clear_serial_buffer()
    if data[1] == '1':                  # 모듈 번호 1
        ser0.write(data.encode('utf-8'))
        time.sleep(10)                  # fanSpeed가 명령한 speed에 도달하고 피드백을 주기위한 시간 지연
        # 피드백이 없는 경우 다시 명령어를 보내 피드백을 받기 위해 시도, 10초 후에도 없으면 "check the fan go to 7 floor" 반환
        FanSpeed_data = ser0.readline().decode('utf-8').strip() if ser0.in_waiting > 0 else try_send_feedback(ser0, data)
    elif data[1] == '2':                # 모듈 번호 2
        ser1.write(data.encode('utf-8'))
        time.sleep(10)                  # fanSpeed가 명령한 speed에 도달하고 피드백을 주기위한 시간 지연
        # 피드백이 없는 경우 다시 명령어를 보내 피드백을 받기 위해 시도, 10초 후에도 없으면 "check the fan go to 7 floor" 반환
        FanSpeed_data = ser1.readline().decode('utf-8').strip() if ser1.in_waiting > 0 else try_send_feedback(ser1, data)
    elif data[1] == '3':                # 모듈 번호 3
        ser2.write(data.encode('utf-8'))
        time.sleep(10)                  # fanSpeed가 명령한 speed에 도달하고 피드백을 주기위한 시간 지연
        # 피드백이 없는 경우 다시 명령어를 보내 피드백을 받기 위해 시도, 10초 후에도 없으면 "check the fan go to 7 floor" 반환
        FanSpeed_data = ser2.readline().decode('utf-8').strip() if ser2.in_waiting > 0 else try_send_feedback(ser2, data)

    print(f"Received Fanspeed data: {FanSpeed_data}")
    send_apm_data(FanSpeed_data, FanSpeed_url)

    print(f"lock released")
    lock.release()

# APM 데이터 전송 함수
def send_apm_data(data, url):
    data_apm = json.dumps({
        "m2m:cin": {
            "con": data
        }
    })
    
    try:
        r_apm2 = requests.post(url, headers=apm2_headers, data=data_apm)
        r_apm2.raise_for_status()
    except requests.exceptions.RequestException as req_err:
        print("APM request error:", req_err)
    except requests.exceptions.HTTPError as http_err:
        print("APM HTTP error:", http_err)
    except Exception as exc:
        print(f'APM problem occurred: {exc}')
        
# motor에서 세종대 명령 데이터 가져오기
def get_data(url):
    commend_data = requests.get(url, headers = sejong_headers)
        
    try:
        commend_data.raise_for_status()
        commend_data_json = commend_data.json()
        commend_data_original = commend_data_json.get("m2m:cin", {}).get("con", None)
    except Exception as exc:
        print(f"There was a problem: {exc}")
        commend_data_original = None
        
    return commend_data_original

# 세종대 명령 데이터 가져오기 (모터 제어 및 데이터 수집)
def sejong_motor_data_get():
    global sejong_start_signal, sejong_data_counter, trainDataPath, testDataPath, state_path, sensor_parameter, stopConditionValue, counter_init, change
    
    # 세종대 명령어 데이터 가져오는 부분
    getData = get_data(sejong_url_motor)

    sensor_parameter = getData['sensor_parameter']
    
    # 모터 제어 명령어 전송
    CommandFanControl(sensor_parameter)
    
    if sensor_parameter[1] == '1':
        if sensor_parameter[4] == '1':
            trainDataPath[0] = getData['trainDataPath']
            testDataPath[0] = getData['testDataPath']
            state_path[0] = getData['state_path'][0]
            stopConditionValue[0] = getData['sensingStopCondition']['stopConditionValue']
            
            sejong_data_counter[0] = counter_init[0] = stopConditionValue[0]
            change[0] = 0
            
            # 데이터 수집 시작 신호
            sejong_start_signal[0] = True
        
        elif sensor_parameter[4] == '2':
            trainDataPath[1] = getData['trainDataPath']
            testDataPath[1] = getData['testDataPath']
            state_path[1] = getData['state_path'][0]
            stopConditionValue[1] = getData['sensingStopCondition']['stopConditionValue']
            
            sejong_data_counter[1] = counter_init[1] = stopConditionValue[1]
            change[1] = 0
            
            # 데이터 수집 시작 신호
            sejong_start_signal[1] = True
            
        elif sensor_parameter[4] == '3':
            trainDataPath[2] = getData['trainDataPath']
            testDataPath[2] = getData['testDataPath']
            state_path[2] = getData['state_path'][0]
            stopConditionValue[2] = getData['sensingStopCondition']['stopConditionValue']
            
            sejong_data_counter[2] = counter_init[2] = stopConditionValue[2]
            change[2] = 0
            
            # 데이터 수집 시작 신호
            sejong_start_signal[2] = True
            
    elif sensor_parameter[1] == '2':
        if sensor_parameter[4] == '1':
            trainDataPath[3] = getData['trainDataPath']
            testDataPath[3] = getData['testDataPath']
            state_path[3] = getData['state_path'][0]
            stopConditionValue[3] = getData['sensingStopCondition']['stopConditionValue']
            
            sejong_data_counter[3] = counter_init[3] = stopConditionValue[3]
            change[3] = 0
            
            # 데이터 수집 시작 신호
            sejong_start_signal[3] = True
        
        elif sensor_parameter[4] == '2':
            trainDataPath[4] = getData['trainDataPath']
            testDataPath[4] = getData['testDataPath']
            state_path[4] = getData['state_path'][0]
            stopConditionValue[4] = getData['sensingStopCondition']['stopConditionValue']
            
            sejong_data_counter[4] = counter_init[4] = stopConditionValue[4]
            change[4] = 0
            
            # 데이터 수집 시작 신호
            sejong_start_signal[4] = True
            
        elif sensor_parameter[4] == '3':
            trainDataPath[5] = getData['trainDataPath']
            testDataPath[5] = getData['testDataPath']
            state_path[5] = getData['state_path'][0]
            stopConditionValue[5] = getData['sensingStopCondition']['stopConditionValue']
            
            sejong_data_counter[5] = counter_init[5] = stopConditionValue[5]
            change[5] = 0
            
            # 데이터 수집 시작 신호
            sejong_start_signal[5] = True
            
    elif sensor_parameter[1] == '3':
        if sensor_parameter[4] == '1':
            trainDataPath[6] = getData['trainDataPath']
            testDataPath[6] = getData['testDataPath']
            state_path[6] = getData['state_path'][0]
            stopConditionValue[6] = getData['sensingStopCondition']['stopConditionValue']
            
            sejong_data_counter[6] = counter_init[6] = stopConditionValue[6]
            change[6] = 0
            
            # 데이터 수집 시작 신호
            sejong_start_signal[6] = True
        
        elif sensor_parameter[4] == '2':
            trainDataPath[7] = getData['trainDataPath']
            testDataPath[7] = getData['testDataPath']
            state_path[7] = getData['state_path'][0]
            stopConditionValue[7] = getData['sensingStopCondition']['stopConditionValue']
            
            sejong_data_counter[7] = counter_init[7] = stopConditionValue[7]
            change[7] = 0
            
            # 데이터 수집 시작 신호
            sejong_start_signal[7] = True
            
        elif sensor_parameter[4] == '3':
            trainDataPath[8] = getData['trainDataPath']
            testDataPath[8] = getData['testDataPath']
            state_path[8] = getData['state_path'][0]
            stopConditionValue[8] = getData['sensingStopCondition']['stopConditionValue']
            
            sejong_data_counter[8] = counter_init[8] = stopConditionValue[8]
            change[8] = 0
            
            # 데이터 수집 시작 신호
            sejong_start_signal[8] = True
    
    print("sejong_motor_data_get() finished")

# 컨테이너 마다의 데이터 처리 함수
def process_module_data(module):
    module_arr = []
    for item in module:
        try:
            module_arr.append(float(item))
        except ValueError:
            module_arr.append(item)
            
    #print(f"module_arr: {module_arr}")
    return module_arr

# 모듈 JSON 데이터 생성 함수
def create_sejong_json(module_data):
    sejong_json = module_data
    return json.dumps(sejong_json)

# 세종 서버로 데이터 전송 함수
def send_sejong_request(data, url):
    data_sejong = json.dumps({
        "m2m:cin": {
            "con": data
        }
    })
    try:
        response = requests.post(url, headers=sejong_headers, data=data_sejong)
    except requests.exceptions.RequestException as req_err:
        print("Sejong request error:", req_err)
    except requests.exceptions.HTTPError as http_err:
        print("Sejong HTTP error:", http_err)
    except Exception as exc:
        print(f'Sejong problem occurred: {exc}')
        
    return response

# 세종대 데이터 전송 함수 (모듈별로 데이터 전송) 
def send_sejong_data(data, moduleNum):
    global sejong_start_signal, sejong_data_counter, trainDataPath, testDataPath, state_path, sensor_parameter, stopConditionValue, counter_init, change
    print(sejong_start_signal)
    where_list = data.split(',')
    
    if(moduleNum == 0):
        if(sejong_data_counter[moduleNum] == 0):
            sejong_start_signal[moduleNum] = False
            send_sejong_request( "Ready" ,state_path[moduleNum])
            print(f"send_sejong_data {moduleNum}moudle finished")
            return
        module_1 =  where_list[:1] + where_list[4:5] + where_list[14:15] + where_list[23:] + where_list[3:4]
        module_1_arr = process_module_data(module_1)
        con0_1_sejong_str = create_sejong_json(module_1_arr)
        
        if counter_init[moduleNum] == 0:
            change[moduleNum] = 1
            
        if change[moduleNum] == 0:
            send_sejong_request(con0_1_sejong_str, trainDataPath[moduleNum])
            counter_init[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, train remainremaining data: {counter_init[moduleNum]}")
        elif change[moduleNum] == 1:
            send_sejong_request(con0_1_sejong_str, testDataPath[moduleNum])
            sejong_data_counter[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, test remainremaining data: {sejong_data_counter[moduleNum]}")
        
    elif(moduleNum == 1):
        #print(f"send_sejong_data {moduleNum}moudle start----------------")
        
        if(sejong_data_counter[moduleNum] == 0):
            sejong_start_signal[moduleNum] = False
            send_sejong_request( "Ready" ,state_path[moduleNum])
            print(f"send_sejong_data {moduleNum}moudle finished")
            return
        
        #print("-----------------")
        module_2 =  where_list[:1] + where_list[5:6] + where_list[15:16] + where_list[23:] + where_list[3:4]
        #print("22222222222222") # ['20241027234859', '1200', '58', '13.60', '95.30', '0.020', '0.006', '0.000', '0.003', '185.00', '0.44', ' 62'] 
        #print(module_2)
        module_2_arr = process_module_data(module_2)
        #print("33333333333333") # [20241027234859.0, 1200.0, 58.0, 13.6, 95.3, 0.02, 0.006, 0.0, 0.003, 185.0, 0.44, 62.0]

        #print(module_2_arr)
        con0_2_sejong_str = create_sejong_json(module_2_arr)
        #print("44444444444444")
        #print(con0_2_sejong_str) # [20241027234859.0, 1200.0, 58.0, 13.6, 95.3, 0.02, 0.006, 0.0, 0.003, 185.0, 0.44, 62.0]
        
        if counter_init[moduleNum] == 0:
            change[moduleNum] = 1
            
        if change[moduleNum] == 0:
            #print("come train")
            
            send_sejong_request(con0_2_sejong_str, trainDataPath[moduleNum])
            counter_init[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, train remainremaining data: {counter_init[moduleNum]}")
        elif change[moduleNum] == 1:
            send_sejong_request(con0_2_sejong_str, testDataPath[moduleNum])
            sejong_data_counter[moduleNum] -= 1  
            print(f"send_sejong_data {moduleNum}moudle finished, test remaining data: {sejong_data_counter[moduleNum]}")
        
    elif(moduleNum == 2):
        if(sejong_data_counter[moduleNum] == 0):
            sejong_start_signal[moduleNum] = False
            send_sejong_request( "Ready" ,state_path[moduleNum])
            print(f"send_sejong_data {moduleNum}moudle finished")
            return
        module_3 =  where_list[:1] + where_list[6:7] + where_list[16:17] + where_list[23:] + where_list[3:4]
        module_3_arr = process_module_data(module_3)
        con0_3_sejong_str = create_sejong_json(module_3_arr)
        
        if counter_init[moduleNum] == 0:
            change[moduleNum] = 1
            
        if change[moduleNum] == 0:
            send_sejong_request(con0_3_sejong_str, trainDataPath[moduleNum])
            counter_init[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, train remainremaining data: {counter_init[moduleNum]}")
        elif change[moduleNum] == 1:
            send_sejong_request(con0_3_sejong_str, testDataPath[moduleNum])
            sejong_data_counter[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, test remaining data: {sejong_data_counter[moduleNum]}")
        
        
    elif(moduleNum == 3):
        if(sejong_data_counter[moduleNum] == 0):
            sejong_start_signal[moduleNum] = False
            send_sejong_request( "Ready" ,state_path[moduleNum])
            print(f"send_sejong_data {moduleNum}moudle finished")
            return
        module_4 =  where_list[:1] + where_list[7:8] + where_list[17:18] + where_list[23:] + where_list[3:4]
        module_4_arr = process_module_data(module_4)
        con0_4_sejong_str = create_sejong_json(module_4_arr)
        
        if counter_init[moduleNum] == 0:
            change[moduleNum] = 1
            
        if change[moduleNum] == 0:
            send_sejong_request(con0_4_sejong_str, trainDataPath[moduleNum])
            counter_init[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, train remainremaining data: {counter_init[moduleNum]}")
        elif change[moduleNum] == 1:
            send_sejong_request(con0_4_sejong_str, testDataPath[moduleNum])
            sejong_data_counter[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, test remaining data: {sejong_data_counter[moduleNum]}")
        
    elif(moduleNum == 4):
        if(sejong_data_counter[moduleNum] == 0):
            sejong_start_signal[moduleNum] = False
            send_sejong_request( "Ready" ,state_path[moduleNum])
            print(f"send_sejong_data {moduleNum}moudle finished")
            return
        module_5 =  where_list[:1] + where_list[8:9] + where_list[18:19] + where_list[23:] + where_list[3:4]
        module_5_arr = process_module_data(module_5)
        con0_5_sejong_str = create_sejong_json(module_5_arr)
        
        if counter_init[moduleNum] == 0:
            change[moduleNum] = 1
            
        if change[moduleNum] == 0:
            send_sejong_request(con0_5_sejong_str, trainDataPath[moduleNum])
            counter_init[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, train remainremaining data: {counter_init[moduleNum]}")
        elif change[moduleNum] == 1:
            send_sejong_request(con0_5_sejong_str, testDataPath[moduleNum])
            sejong_data_counter[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, test remaining data: {sejong_data_counter[moduleNum]}")
        
    elif(moduleNum == 5):
        if(sejong_data_counter[moduleNum] == 0):
            sejong_start_signal[moduleNum] = False
            send_sejong_request( "Ready" ,state_path[moduleNum])
            print(f"send_sejong_data {moduleNum}moudle finished")
            return
        module_6 =  where_list[:1] + where_list[9:10] + where_list[19:20] + where_list[23:] + where_list[3:4]
        module_6_arr = process_module_data(module_6)
        con0_6_sejong_str = create_sejong_json(module_6_arr)
        
        if counter_init[moduleNum] == 0:
            change[moduleNum] = 1
            
        if change[moduleNum] == 0:
            send_sejong_request(con0_6_sejong_str, trainDataPath[moduleNum])
            counter_init[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, train remainremaining data: {counter_init[moduleNum]}")
        elif change[moduleNum] == 1:
            send_sejong_request(con0_6_sejong_str, testDataPath[moduleNum])
            sejong_data_counter[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, test remaining data: {sejong_data_counter[moduleNum]}")
        
    elif(moduleNum == 6):
        if(sejong_data_counter[moduleNum] == 0):
            sejong_start_signal[moduleNum] = False
            send_sejong_request( "Ready" ,state_path[moduleNum])
            print(f"send_sejong_data {moduleNum}moudle finished")
            return
        module_7 =  where_list[:1] + where_list[10:11] + where_list[20:21] + where_list[23:] + where_list[3:4]
        module_7_arr = process_module_data(module_7)
        con0_7_sejong_str = create_sejong_json(module_7_arr)
        
        if counter_init[moduleNum] == 0:
            change[moduleNum] = 1
            
        if change[moduleNum] == 0:
            send_sejong_request(con0_7_sejong_str, trainDataPath[moduleNum])
            counter_init[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, train remainremaining data: {counter_init[moduleNum]}")
        elif change[moduleNum] == 1:
            send_sejong_request(con0_7_sejong_str, testDataPath[moduleNum])
            sejong_data_counter[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, test remaining data: {sejong_data_counter[moduleNum]}")
        
    elif(moduleNum == 7):
        if(sejong_data_counter[moduleNum] == 0):
            sejong_start_signal[moduleNum] = False
            send_sejong_request( "Ready" ,state_path[moduleNum])
            print(f"send_sejong_data {moduleNum}moudle finished")
            return
        module_8 =  where_list[:1] + where_list[11:12] + where_list[21:22] + where_list[23:] + where_list[3:4]
        module_8_arr = process_module_data(module_8)
        con0_8_sejong_str = create_sejong_json(module_8_arr)
        
        if counter_init[moduleNum] == 0:
            change[moduleNum] = 1
            
        if change[moduleNum] == 0:
            send_sejong_request(con0_8_sejong_str, trainDataPath[moduleNum])
            counter_init[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, train remainremaining data: {counter_init[moduleNum]}")
        elif change[moduleNum] == 1:
            send_sejong_request(con0_8_sejong_str, testDataPath[moduleNum])
            sejong_data_counter[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, test remaining data: {sejong_data_counter[moduleNum]}")
        
    elif(moduleNum == 8):
        if(sejong_data_counter[moduleNum] == 0):
            sejong_start_signal[moduleNum] = False
            send_sejong_request( "Ready" ,state_path[moduleNum])
            print(f"send_sejong_data {moduleNum}moudle finished")
            return
        module_9 =  where_list[:1] + where_list[12:13] + where_list[22:23] + where_list[23:] + where_list[3:4]
        module_9_arr = process_module_data(module_9)
        con0_9_sejong_str = create_sejong_json(module_9_arr)
        
        if counter_init[moduleNum] == 0:
            change[moduleNum] = 1
            
        if change[moduleNum] == 0:
            send_sejong_request(con0_9_sejong_str, trainDataPath[moduleNum])
            counter_init[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, train remainremaining data: {counter_init[moduleNum]}")
        elif change[moduleNum] == 1:
            send_sejong_request(con0_9_sejong_str, testDataPath[moduleNum])
            sejong_data_counter[moduleNum] -= 1
            print(f"send_sejong_data {moduleNum}moudle finished, test remaining data: {sejong_data_counter[moduleNum]}")
        
        
# 1분마다 실행되는 함수
def OneMinute(url):
    global RPM_pm_data_b, RPM_pm_data_a, RPM_time

    print("Acquiring lock")
    lock.acquire()
    
    try:
        # APM2 시간 수집
        APM2_time = datetime.now().strftime('%Y%m%d%H%M%S')
        
        # RPM 데이터 수집
        print("Fetching RPM data")
        rpm_data_get = requests.get(url, headers=apm2_headers)
        rpm_data_get.raise_for_status()
        rpm_data_get_json = rpm_data_get.json()
        rpm_data_original = rpm_data_get_json.get("m2m:cin", {}).get("con", None)
        print(f"RPM data fetched: {rpm_data_original}")

        # RPM 데이터 처리
        if rpm_data_original:
            rpm_data_original_list = rpm_data_original.split(',')
            RPM_time = rpm_data_original_list[0] if len(rpm_data_original_list) > 0 else 'null'
            RPM_pm_data_b = rpm_data_original_list[5] if len(rpm_data_original_list) > 5 else 'null'
            RPM_pm_data_a = rpm_data_original_list[6] if len(rpm_data_original_list) > 6 else 'null'
        else:
            RPM_time = RPM_pm_data_b = RPM_pm_data_a = 'null'

    # APM2 데이터 수집 및 전송
        clear_serial_buffer()
        print("Sending start command to serial ports")
        for ser in [ser0, ser1, ser2, ser3, ser4]:
            ser.write(b'start')
        time.sleep(5)
        
        APM2_Data = read_serial_data()
        combined_data = f"{APM2_time},{RPM_time},{APM2_Data},{RPM_pm_data_b},{RPM_pm_data_a}"
        print(f"Combined data: {combined_data}")
        
        # 데이터 재정렬
        original_order = [
            "APM_datetime", "RPM_datetime", "pwm1_1", "pwm1_2", "pwm1_3", "pm1_1", "pm1_2", "pm1_3",
            "pwm2_1", "pwm2_2", "pwm2_3", "pm2_1", "pm2_2", "pm2_3",
            "pwm3_1", "pwm3_2", "pwm3_3", "pm3_1", "pm3_2", "pm3_3",
            "temp", "humi", "o3", "co", "no2", "so2",
            "wind_d", "wind_s", "npm", "rpm before correction", "rpm after correction"
        ]
        
        desired_order = [
            "APM_datetime", "RPM_datetime", "rpm before correction", "rpm after correction",
            "pwm1_1", "pwm1_2", "pwm1_3", "pwm2_1", "pwm2_2", "pwm2_3",
            "pwm3_1", "pwm3_2", "pwm3_3", "npm", "pm1_1", "pm1_2", "pm1_3",
            "pm2_1", "pm2_2", "pm2_3", "pm3_1", "pm3_2", "pm3_3",
            "temp", "humi", "o3", "co", "no2", "so2", "wind_d", "wind_s"
        ]

        index_map = {original_order[i]: i for i in range(len(original_order))}
        data_list = combined_data.split(',')
        reordered_data = [data_list[index_map[col]] for col in desired_order]
        combined_data = ','.join(reordered_data)
        print(f"Reordered data: {combined_data}")
        
        # APM2서버로 데이터 전송
        send_apm_data(combined_data, apm2_url)
        
        for i in range(0, 9):
            if(sejong_start_signal[i] == True):
                #print(f"Sending data to Sejong server : {i}...")
                send_sejong_data(combined_data, i)
                
            
    except Exception as exc:
        print(f"Error occurred: {exc}")
    finally:
        print("Releasing lock")
        lock.release()



# TCP통신 함수
def handle_connection(conn, addr):
    print(f'Connected to {addr}')
    while True:
        data = conn.recv(1024)
        #print(data)
        if not data:
            break
        data = data.decode('utf-8').strip()
        if data.startswith('M'):
            CommandFanControl(data)
        elif(data.startswith('[')):
            sejong_motor_data_get()
    conn.close()

# 스케줄링 함수
def schedule_tasks():
    while True:
        time.sleep(60)
        print("time's up")
        OneMinute(rpm_url + '/la')
        
# 서버 함수
def server():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(('localhost', port))
        s.listen()
        print(f"Listening on port {port}...")

        # 스케줄 작업을 별도의 스레드에서 실행
        scheduler_thread = threading.Thread(target=schedule_tasks, daemon=True)
        scheduler_thread.start()

        while True:
            conn, addr = s.accept()
            client_thread = threading.Thread(target=handle_connection, args=(conn, addr))
            client_thread.start()

# 키보드 인터럽트 핸들러     
def signal_handler(sig, frame):
    global running
    print('Safely shutting down...')
    running = False
    ser0.close()
    ser1.close()
    ser2.close()
    ser3.close()
    ser4.close()
    #who are you dont touch
    sys.exit(0)

# 키보드 인터럽트 핸들러 등록
signal.signal(signal.SIGINT, signal_handler)


# 코드 실행 시작 부분
init_serial()
clear_serial_buffer()

# 서버 실행
try:
    server()
except KeyboardInterrupt:
    print("KeyboardInterrupt")
    ser0.close()
    ser1.close()
    ser2.close()
    ser3.close()
    ser4.close()
    sys.exit(0)
