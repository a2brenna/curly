#include "curly.h"

#include <cassert>
#include <unistd.h>
#include <string.h>
#include <jsoncpp/json/reader.h>

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    const size_t realsize = size * nmemb;

    struct MemoryStruct *mem = [](const size_t realsize, void *userp){
		struct MemoryStruct *mem = (struct MemoryStruct *)userp;
		if(mem->cursor + realsize >= mem->size){
			mem->size = std::max(mem->cursor + realsize + 1, mem->size * 2);
			mem->memory = (char *)realloc(mem->memory, mem->size);
			assert(mem->memory);
		}
		return mem;

	}(realsize, userp);

    memcpy(&(mem->memory[mem->cursor]), contents, realsize);
    mem->cursor += realsize;
    mem->memory[mem->cursor] = 0;

    return realsize;
}

Curl_Instance::Curl_Instance(const std::string &url, const size_t &recv_buffer_size){
	_buffer = [](const size_t &recv_buffer_size){
		struct MemoryStruct buffer;
		buffer.memory = (char *)malloc(recv_buffer_size);
		assert(buffer.memory);

		buffer.size = recv_buffer_size;
		buffer.cursor = 0;
		return buffer;
	}(recv_buffer_size);

    _curl_handle = [](const std::string &url, struct MemoryStruct *buffer) {
        auto handle = curl_easy_init();
        curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(handle, CURLOPT_TIMEOUT, 5);

        curl_easy_setopt(handle, CURLOPT_SSL_SESSIONID_CACHE, 1);

        /* send all data to this function  */
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)(buffer));
        return handle;
    }(url, &_buffer);

}

Curl_Instance::~Curl_Instance() {
    free(_buffer.memory);
    curl_easy_cleanup(_curl_handle);
}

Json::Value Curl_Instance::get_json() {
    Json::Value data;
    Json::Reader reader;

    //fill up _buffer
    const CURLcode res = curl_easy_perform(_curl_handle);

    if(res != CURLE_OK){
        const auto response_code = [](CURL *handle){
            long response_code;
            curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response_code);
            return response_code;
        }(_curl_handle);
        throw Curl_Error();
    }

    reader.parse(&(_buffer.memory[0]), &(_buffer.memory[_buffer.cursor]), data, false);

    //reset _buffer
    _buffer.memory[0] = 0;
    _buffer.cursor = 0;
    return data;
}
