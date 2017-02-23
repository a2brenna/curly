#ifndef __CURLY_H__
#define __CURLY_H__

#include <jsoncpp/json/value.h>
#include <curl/curl.h>
#include <curl/easy.h>

struct CMemoryStruct {
    char *memory;
    size_t size;
    size_t cursor;
};

class Curl_Error {};

class Curl_Instance{

    public:
        Curl_Instance(const std::string &url, const size_t &recv_buffer_size);
        ~Curl_Instance();
        Json::Value get_json();

    private:
        CURL *_curl_handle;
        CMemoryStruct _buffer;

};

#endif
