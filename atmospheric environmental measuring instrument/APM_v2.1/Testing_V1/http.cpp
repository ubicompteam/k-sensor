#include "http.h"
#include <iostream>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <string>
#include <thread>  // std::this_thread::sleep_for
#include <sstream>


// CURL 응답을 처리할 콜백 함수
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// 서버에 값 보내는 함수 (POST 요청)
void send_value_to_server(const std::string& url, const std::string& data) {
    CURL* curl;
    CURLcode res;

    // 헤더 설정
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "X-M2M-RI: 12345");
    headers = curl_slist_append(headers, "X-M2M-Origin: SYbj2CzynVR");
    headers = curl_slist_append(headers, "Content-Type: application/vnd.onem2m-res+json; ty=4");

    // JSON 데이터 설정
    Json::Value jsonData;
    jsonData["m2m:cin"]["con"] = data;
    Json::StreamWriterBuilder writer;
    std::string requestData = Json::writeString(writer, jsonData);

    // libcurl 초기화
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        // URL과 헤더 설정
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestData.c_str());

        // POST 요청 보내기
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "Data sent successfully." << std::endl;
        }

        // 리소스 해제
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

std::string get_value_from_server(const std::string& url) {
    CURL* curl;
    CURLcode res;

    // 헤더 설정
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "X-M2M-RI: 12345");
    headers = curl_slist_append(headers, "X-M2M-Origin: SYbj2CzynVR");
    headers = curl_slist_append(headers, "Content-Type: application/vnd.onem2m-res+json; ty=4");

    // libcurl 초기화
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    std::string response_data;  // 응답을 저장할 변수

    if (curl) {
        // 응답을 저장할 버퍼 설정
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        // GET 요청 보내기
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        // 리소스 해제
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    // JSON 응답 처리
    Json::CharReaderBuilder readerBuilder;
    Json::Value jsonData;
    std::string errs;

    std::istringstream sstream(response_data); // 응답을 스트림으로 변환
    if (Json::parseFromStream(readerBuilder, sstream, &jsonData, &errs)) {
        // JSON에서 'm2m:cin'의 'con' 값 반환
        if (jsonData.isMember("m2m:cin") && jsonData["m2m:cin"].isMember("con")) {
            const Json::Value& conValue = jsonData["m2m:cin"]["con"];
            if (conValue.isString()) {
                return conValue.asString();  // 'con'이 문자열인 경우
            } else if (conValue.isObject()) {
                Json::StreamWriterBuilder writer;
                return Json::writeString(writer, jsonData);  // 'con'이 객체인 경우 문자열로 변환
            } else {
                std::cerr << "Error: 'con' is not a string or object." << std::endl;
                return "";
            }
        } else {
            std::cerr << "Error: 'm2m:cin' or 'con' not found in the response." << std::endl;
            return "";  // 빈 문자열을 반환
        }
    } else {
        std::cerr << "Error parsing JSON: " << errs << std::endl;
        return "";  // 파싱 실패 시 빈 문자열 반환
    }
}


