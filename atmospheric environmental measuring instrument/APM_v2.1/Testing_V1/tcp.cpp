// tcp.cpp
#include "tcp.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

TCPServer::TCPServer(int port) : port(port), server_fd(-1) {}

TCPServer::~TCPServer() {
    if (server_fd != -1) {
        close(server_fd);
    }
    if (server_thread.joinable()) {
        server_thread.join();
    }
}

void TCPServer::start() {
    server_thread = std::thread(&TCPServer::tcp_server, this);
}

void TCPServer::tcp_server() {
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // 소켓 생성
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return;
    }

    // 포트 재사용 설정
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // 바인딩
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return;
    }

    // 대기 시작
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        return;
    }

    while (true) {
        std::cout << "Waiting for connection on port " << port << "..." << std::endl;

        int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("accept failed");
            continue;
        }

        int valread = read(new_socket, buffer, 1024);
        if (valread > 0) {
            std::lock_guard<std::mutex> lock(data_mutex);
            received_data = std::string(buffer, valread);
            std::cout << "Data received: " << received_data << std::endl;
        }

        close(new_socket);
    }
}

std::string TCPServer::getReceivedData() {
    std::lock_guard<std::mutex> lock(data_mutex);
    std::string data = received_data;
    received_data.clear(); // 데이터 읽은 후 클리어
    return data;
}

