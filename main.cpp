#include "Log/miniLog.h"
#include <iostream>

using namespace Logging;

int main() {
    std::cout << "Hello, World!" << std::endl;

    LOG_INFO("log info %d %s", 0 , "test");
    LOG_DEBUG("log debug %d %s", 1, "test");
    LOG_DEBUG("log error %d %s", 2, "test");

    return 0;
}
