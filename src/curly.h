#ifndef __CURLY_H__
#define __CURLY_H__

#include <curl/curl.h>
#include <curl/easy.h>
#include <vector>
#include <string>
#include <chrono>

namespace curly{

struct CMemoryStruct {
    char *memory;
    size_t size;
    size_t cursor;
    bool resizable;
};

class Curl_Instance {

    public:
        Curl_Instance(const size_t &recv_buffer_size);
        ~Curl_Instance();

        std::pair<uint32_t, std::string> get(const std::string &url);
        std::pair<uint32_t, std::string> get(const std::string &url, const std::vector<std::pair<std::string, std::string>> &headers);
        std::pair<uint32_t, std::string> post(const std::string &url, const std::vector<std::pair<std::string, std::string>> &headers, const std::string &post_parameters);

        std::pair<uint32_t, size_t> get(const std::string &url, char *target, const size_t &target_size);
        std::pair<uint32_t, size_t> get(const std::string &url, const std::vector<std::pair<std::string, std::string>> &headers, char *target, const size_t &target_size);
        std::pair<uint32_t, size_t> post(const std::string &url, const std::vector<std::pair<std::string, std::string>> &headers, const std::string &post_parameters, char *target, const size_t &target_size);

        void set_timeout(const size_t &seconds);
        void reset();

        std::pair<std::string, std::string> error() const;

    private:
        CURL *_curl_handle;
        CURLcode _latest_CURLcode;
        char _curl_error_buffer[CURL_ERROR_SIZE];

        size_t _timeout = 60;
        CMemoryStruct _buffer;

};

};

#endif
