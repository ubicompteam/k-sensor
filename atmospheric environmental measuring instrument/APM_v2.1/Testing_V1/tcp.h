// tcp.h
#ifndef TCP_H
#define TCP_H

#include <string>
#include <mutex>
#include <thread>

class TCPServer {
public:
    TCPServer(int port);
    ~TCPServer();

    // TCP 서버 시작
    void start();

    // 수신된 데이터 가져오기
    std::string getReceivedData();

private:
    void tcp_server(); // 내부 TCP 서버 스레드 함수

    int port;
    int server_fd;
    std::string received_data;
    std::mutex data_mutex;
    std::thread server_thread; // 서버 스레드
};

#endif // TCP_H

