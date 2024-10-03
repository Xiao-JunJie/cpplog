#include "Log/miniLog.h"
#include <iostream>

#define EPOCH 1000000
using namespace Logging;

void pressureOneThreadTest() {
    std::string msg(30,'*');
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0;i < EPOCH;++i)
    {
        LOG_INFO("log info %d %s", i , msg.c_str());
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> result = end - start;

    double res = static_cast<double>(result.count());
    printf("OneThread costTime: %f,  %0.0f log/s", res, EPOCH / res );
}

int main() {
    pressureOneThreadTest();

    return 0;
}
