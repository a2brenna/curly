#ifndef __CURLY_H__
#define __CURLY_H__

#include <jsoncpp/json/value.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <chrono>

namespace curly{

extern bool PROFILE;

struct CMemoryStruct {
    char *memory;
    size_t size;
    size_t cursor;
    bool resizable;
};

struct Perf_Data {
    std::chrono::high_resolution_clock::time_point request_start = std::chrono::high_resolution_clock::time_point(std::chrono::nanoseconds(0));
    std::chrono::high_resolution_clock::time_point request_end = std::chrono::high_resolution_clock::time_point(std::chrono::nanoseconds(0));
};

class Curl_Error {

    public:
        Curl_Error(const long &response_code, const CURLcode &res);

    private:
        long _response_code;
        CURLcode _res;

};

class Curl_Instance{

    public:
        Curl_Instance(const std::string &url, const size_t &recv_buffer_size, bool *PROFILE_SWITCH);
        ~Curl_Instance();
        Json::Value get_json();
        size_t get(char *target, const size_t &target_size);
        size_t post(const std::vector<std::pair<std::string, std::string>> &headers, const std::string &post_parameters, char *target, const size_t &target_size);
        struct Perf_Data perf_data() const;
        std::string serialized_perf_data() const;

    private:
        CURL *_curl_handle;
        CMemoryStruct _buffer;
        struct Perf_Data _perf;
        bool *_PROFILE;

};

};

#endif
