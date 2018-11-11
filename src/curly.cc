#include "curly.h"

#include <cassert>
#include <unistd.h>
#include <string.h>
#include <jsoncpp/json/reader.h>

uint32_t get_response_code(CURL *handle){
    long response_code;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response_code);
    assert(response_code >= 0);
    return (uint32_t)response_code;
}

namespace curly {

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    const size_t realsize = size * nmemb;

    struct CMemoryStruct *mem = [](const size_t realsize, void *userp){
		struct CMemoryStruct *mem = (struct CMemoryStruct *)userp;
		if(mem->cursor + realsize >= mem->size){
            if(mem->resizable){
                //TODO: rejigger this so it only works in powers of 2
                mem->size = std::max(mem->cursor + realsize + 1, mem->size * 2);
                mem->memory = (char *)realloc(mem->memory, mem->size);
                assert(mem->memory);
            }
            else{
                //TODO: Investigate error handling
                //Cannot throw from here? This is executed by libcurl library code which is straight c...
                //Maybe should return 0;
                assert(false);
            }
		}
		return mem;

	}(realsize, userp);

    memcpy(&(mem->memory[mem->cursor]), contents, realsize);
    mem->cursor += realsize;
    mem->memory[mem->cursor] = 0;

    return realsize;
}

CURL *setup_handle(struct CMemoryStruct *buffer, char *curl_error_buffer, const size_t &timeout){
    auto handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, timeout);

    curl_easy_setopt(handle, CURLOPT_SSL_SESSIONID_CACHE, 1);

    /* set up buffer to provide error strings */
    curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, curl_error_buffer);

    /* send all data to this function  */
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)(buffer));
    return handle;
}

Curl_Instance::Curl_Instance(const size_t &recv_buffer_size){
	_buffer = [](const size_t &recv_buffer_size){
		struct CMemoryStruct buffer;

        if(recv_buffer_size > 0){
            buffer.memory = (char *)malloc(recv_buffer_size);
            assert(buffer.memory);
        }
        else{
            buffer.memory = nullptr;
        }

		buffer.size = recv_buffer_size;
		buffer.cursor = 0;
        buffer.resizable = true;
		return buffer;
	}(recv_buffer_size);

    _timeout = 60;

    _curl_handle = setup_handle(&_buffer, _curl_error_buffer, _timeout);

    return;
}

void Curl_Instance::set_timeout(const size_t &seconds){
    _timeout = seconds;
    curl_easy_setopt(_curl_handle, CURLOPT_TIMEOUT, _timeout);
    return;
}

void Curl_Instance::reset(){
    curl_easy_cleanup(_curl_handle);
    _curl_handle = setup_handle(&_buffer, _curl_error_buffer, _timeout);
    return;
}

Curl_Instance::~Curl_Instance() {
    free(_buffer.memory);
    curl_easy_cleanup(_curl_handle);
    return;
}

std::pair<uint32_t, std::string> Curl_Instance::get(const std::string &url){
    //reset _buffer
    _buffer.memory[0] = 0;
    _buffer.cursor = 0;

    //set url
    curl_easy_setopt(_curl_handle, CURLOPT_URL, url.c_str());

    //fill up _buffer
    _latest_CURLcode = curl_easy_perform(_curl_handle);

    const uint32_t response_code = get_response_code(_curl_handle);

    return std::pair<uint32_t, std::string>(response_code, std::string(&(_buffer.memory[0]), _buffer.cursor));
}

std::pair<uint32_t, std::string> Curl_Instance::get(const std::string &url, const std::vector<std::pair<std::string, std::string>> &headers){
    //reset _buffer
    _buffer.memory[0] = 0;
    _buffer.cursor = 0;

    //set url
    curl_easy_setopt(_curl_handle, CURLOPT_URL, url.c_str());

    //set headers
    struct curl_slist *http_headers = [](const std::vector<std::pair<std::string, std::string>> &headers){
        struct curl_slist *http_headers = nullptr;
        for(const auto &h: headers){
            const std::string header = h.first + ": " + h.second;
            http_headers = curl_slist_append(http_headers, header.c_str());
        }
        return http_headers;
    }(headers);

    curl_easy_setopt(_curl_handle, CURLOPT_HTTPHEADER, http_headers);

    //fill up _buffer
    _latest_CURLcode = curl_easy_perform(_curl_handle);
    curl_slist_free_all(http_headers);

    const uint32_t response_code = get_response_code(_curl_handle);

    return std::pair<uint32_t, std::string>(response_code, std::string(&(_buffer.memory[0]), _buffer.cursor));
}

std::pair<uint32_t, size_t> Curl_Instance::get(const std::string &url, const std::vector<std::pair<std::string, std::string>> &headers, char *target, const size_t &target_size){
    const auto old_buffer = _buffer;

    _buffer.resizable = false;
    _buffer.memory = target;
    _buffer.cursor = 0;
    _buffer.size = target_size;

    //set url
    curl_easy_setopt(_curl_handle, CURLOPT_URL, url.c_str());

    struct curl_slist *http_headers = [](const std::vector<std::pair<std::string, std::string>> &headers){
        struct curl_slist *http_headers = nullptr;
        for(const auto &h: headers){
            const std::string header = h.first + ": " + h.second;
            http_headers = curl_slist_append(http_headers, header.c_str());
        }
        return http_headers;
    }(headers);

    curl_easy_setopt(_curl_handle, CURLOPT_HTTPHEADER, http_headers);

    //fill up _buffer
    _latest_CURLcode = curl_easy_perform(_curl_handle);
    curl_slist_free_all(http_headers);

    const uint32_t response_code = get_response_code(_curl_handle);

    const size_t bytes_fetched = _buffer.cursor;
    _buffer = old_buffer;
    return std::pair<uint32_t, size_t>(response_code, bytes_fetched);
}

std::pair<uint32_t, size_t> Curl_Instance::get(const std::string &url, char *target, const size_t &target_size){
    const std::vector<std::pair< std::string, std::string>> headers;
    return get(url, headers, target, target_size);
}

std::pair<uint32_t, size_t> Curl_Instance::post(const std::string &url, const std::vector<std::pair<std::string, std::string>> &headers, const std::string &post_parameters, char *target, const size_t &target_size){
    const auto old_buffer = _buffer;

    _buffer.resizable = false;
    _buffer.memory = target;
    _buffer.cursor = 0;
    _buffer.size = target_size;

    //set url
    curl_easy_setopt(_curl_handle, CURLOPT_URL, url.c_str());

    struct curl_slist *http_headers = [](const std::vector<std::pair<std::string, std::string>> &headers){
        struct curl_slist *http_headers = nullptr;
        for(const auto &h: headers){
            const std::string header = h.first + ": " + h.second;
            http_headers = curl_slist_append(http_headers, header.c_str());
        }
        return http_headers;
    }(headers);

    curl_easy_setopt(_curl_handle, CURLOPT_HTTPHEADER, http_headers);

    curl_easy_setopt(_curl_handle, CURLOPT_POST, 1L);
    curl_easy_setopt(_curl_handle, CURLOPT_POSTFIELDS, post_parameters.c_str());
    curl_easy_setopt(_curl_handle, CURLOPT_POSTFIELDSIZE, post_parameters.size());

    //fill up _buffer
    _latest_CURLcode = curl_easy_perform(_curl_handle);
    curl_slist_free_all(http_headers);

    const uint32_t response_code = get_response_code(_curl_handle);

    const size_t bytes_fetched = _buffer.cursor;
    _buffer = old_buffer;
    return std::pair<uint32_t, size_t>(response_code, bytes_fetched);
}

std::pair<uint32_t, std::string> Curl_Instance::post(const std::string &url, const std::vector<std::pair<std::string, std::string>> &headers, const std::string &post_parameters){
    //reset _buffer
    _buffer.memory[0] = 0;
    _buffer.cursor = 0;

    struct curl_slist *http_headers = [](const std::vector<std::pair<std::string, std::string>> &headers){
        struct curl_slist *http_headers = nullptr;
        for(const auto &h: headers){
            const std::string header = h.first + ": " + h.second;
            http_headers = curl_slist_append(http_headers, header.c_str());
        }
        return http_headers;
    }(headers);

    //set url
    curl_easy_setopt(_curl_handle, CURLOPT_URL, url.c_str());

    curl_easy_setopt(_curl_handle, CURLOPT_HTTPHEADER, http_headers);
    curl_easy_setopt(_curl_handle, CURLOPT_POST, 1L);
    curl_easy_setopt(_curl_handle, CURLOPT_POSTFIELDS, post_parameters.c_str());
    curl_easy_setopt(_curl_handle, CURLOPT_POSTFIELDSIZE, post_parameters.size());

    //fill up _buffer
    _latest_CURLcode = curl_easy_perform(_curl_handle);
    curl_slist_free_all(http_headers);

    const uint32_t response_code = get_response_code(_curl_handle);

    return std::pair<uint32_t, std::string>(response_code, std::string(&(_buffer.memory[0]), _buffer.cursor));
}

std::pair<std::string, std::string> Curl_Instance::error() const{
    const std::string code(curl_easy_strerror(_latest_CURLcode));

    //This rigamarole is because I don't want to trust that the _curl_error_buffer is in a valid state, i.e. null terminated, but it probably will be
    char buff[CURL_ERROR_SIZE + 1];
    buff[CURL_ERROR_SIZE] = '\0';
    strncpy(buff, _curl_error_buffer, CURL_ERROR_SIZE);
    const std::string error_buffer(buff);

    return std::pair<std::string, std::string>(code, error_buffer);
}

}
