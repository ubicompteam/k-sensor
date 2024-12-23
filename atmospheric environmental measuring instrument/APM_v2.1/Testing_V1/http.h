#ifndef HTTP_H
#define HTTP_H

#include <string>

//post data to server
void send_value_to_server(const std::string& url, const std::string& data);

// get data to server
std::string get_value_from_server(const std::string& url);

#endif //HTTP_H
