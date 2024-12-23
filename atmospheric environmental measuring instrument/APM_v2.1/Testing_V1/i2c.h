#ifndef I2C_H
#define I2C_H

#include <string>

// I2C 장치 열기
int openI2CDevice(int address);

// 데이터를 읽어오는 함수
std::string readData(int file, int address);

// 사용자 명령어 전송 함수
void sendCommand(int file, const std::string &command);

#endif // I2C_H

