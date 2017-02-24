#include <iostream>
#include "curly.h"
#include <chrono>
#include <thread>

int main(){
    std::cout << "Starting Benchmark..." << std::endl;

    Curl_Instance test("https://csclub.uwaterloo.ca", 16384);
    //TODO: Test with zero length buffer

    for(size_t i = 0; i < 50; i++){
        const auto start_time = std::chrono::high_resolution_clock::now();
        const auto data = test.get_json();
        const auto end_time = std::chrono::high_resolution_clock::now();
        std::cout << "Trial " << i << " Elapsed " << (end_time - start_time).count() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Finished Benchmark" << std::endl;
    return 0;
}
