#ifndef __CURLY_H__
#define __CURLY_H__

#include <jsoncpp/json/value.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <chrono>

namespace curly{

extern bool *PROFILE;

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

class Curl_Error {};

class Curl_Instance{

    public:
        Curl_Instance(const std::string &url, const size_t &recv_buffer_size);
        ~Curl_Instance();
        Json::Value get_json();
        size_t get(char *target, const size_t &target_size);

    private:
        CURL *_curl_handle;
        CMemoryStruct _buffer;
        struct Perf_Data _perf;

};

};

#endif
