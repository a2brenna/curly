#ifndef __CURLY_H__
#define __CURLY_H__

#include <jsoncpp/json/value.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <chrono>

namespace curly{

struct CMemoryStruct {
    char *memory;
    size_t size;
    size_t cursor;
    bool resizable;
};

class Curl_Error {

    public:
        Curl_Error(const long &response_code, const CURLcode &res);
        long response_code() const;
        CURLcode res() const;
        std::string str() const;

    private:
        long _response_code;
        CURLcode _res;

};

class Curl_Instance{

    public:
        Curl_Instance(const std::string &url, const size_t &recv_buffer_size);
        ~Curl_Instance();

        std::pair<uint32_t, Json::Value> get_json();
        std::pair<uint32_t, Json::Value> post_json(const std::vector<std::pair<std::string, std::string>> &headers, const std::string &post_parameters);

        size_t get(char *target, const size_t &target_size);
        size_t get(const std::vector<std::pair<std::string, std::string>> &headers, char *target, const size_t &target_size);
        size_t post(const std::vector<std::pair<std::string, std::string>> &headers, const std::string &post_parameters, char *target, const size_t &target_size);

    private:
        CURL *_curl_handle;
        CMemoryStruct _buffer;

};

};

#endif
