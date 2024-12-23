#include "i2c.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cerrno>

#define I2C_DEVICE "/dev/i2c-1"  // Jetson Nano의 I2C 포트
#define WRITE_REGISTER  0x01     // 사용자 명령어를 보내기 위한 레지스터 주소
#define READ_REGISTER   0x02     // start 신호를 보내 슬레이브에게 데이터를 읽어오라고 요청하기 위한 레지스터 주소
#define START_SIGNAL    0xA0     // start 신호를 보내기 위한 명령어
#define GET_FEEDBACK	0xA1     // Fan Speed Feedback command

// I2C 장치 열기
int openI2CDevice(int address) {
    int f = open(I2C_DEVICE, O_RDWR);
    if (f < 0) {
        std::cerr << "I2C 장치를 열 수 없습니다: " << strerror(errno) << std::endl;
        return -1;
    }

    // 슬레이브 주소 설정
    if (ioctl(f, I2C_SLAVE, address) < 0) {
        std::cerr << "슬레이브 주소를 설정할 수 없습니다: " << strerror(errno) << std::endl;
        close(f);
        return -1;
    }
    return f;
}

std::string readData(int file, int address) {
    unsigned char command_register;
    
    if (address == 0){
	command_register = START_SIGNAL;
	
    }else {
	command_register = GET_FEEDBACK;
    }

    unsigned char buffer[2] = {READ_REGISTER, command_register};

    if (write(file, buffer, sizeof(buffer)) != sizeof(buffer)) {
        std::cerr << "데이터 요청 전송에 실패했습니다: " << strerror(errno) << std::endl;
        return "error";
    }

    char readBuffer[32] = {0};  // 읽을 버퍼 초기화
    ssize_t bytesRead = read(file, readBuffer, sizeof(readBuffer) - 1);
    if (bytesRead <= 0) {
        std::cerr << "데이터 수신에 실패했습니다: " << strerror(errno) << std::endl;
        return "error";
    }

    readBuffer[bytesRead] = '\0';  // 종료 문자 설정
    //std::cout << "받은 데이터: " << readBuffer << std::endl;
    return readBuffer;
}

// 사용자 명령어 전송 함수
void sendCommand(int file, const std::string &command) {
    unsigned char buffer[1 + command.size()];
    buffer[0] = WRITE_REGISTER;  // 첫 번째 바이트에 레지스터 주소 저장
    std::memcpy(buffer + 1, command.c_str(), command.size());

    if (write(file, buffer, sizeof(buffer)) != sizeof(buffer)) {
        std::cerr << "명령어 전송에 실패했습니다: " << strerror(errno) << std::endl;
    } else {
        std::cout << "명령어 '" << command << "'을(를) 보냈습니다." << std::endl;
    }
}

