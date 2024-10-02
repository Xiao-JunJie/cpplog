//
// Created by xjj-pc on 2024/9/28.
//

#ifndef _MINILOG_H
#define _MINILOG_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <deque>
#include <mutex>
#include <future>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>
// #include <cstdarg>

#define LOG_INFO(fmt, args...) do \
{                             \
    char buff[1024] = {0};    \
    snprintf(buff, sizeof(buff), "[%s:%d]" fmt, __FUNCTION__, __LINE__, ##args); \
    Logging::Logger::getInstance().log(Logging::LogLevel::INFO, buff);           \
} while(false);

#define LOG_DEBUG(fmt, args...) do \
{                             \
    char buff[1024] = {0};    \
    snprintf(buff, sizeof(buff), "[%s:%d]" fmt, __FUNCTION__, __LINE__, ##args); \
    Logging::Logger::getInstance().log(Logging::LogLevel::DEBUG, buff);          \
} while(false);

#define LOG_ERR(fmt, args...) do \
{                             \
    char buff[1024] = {0};    \
    snprintf(buff, sizeof(buff), "[%s:%d]" fmt, __FUNCTION__, __LINE__, ##args); \
    Logging::Logger::getInstance().log(Logging::LogLevel::ERROR, buff);          \
} while(false)

namespace Logging {

    enum class LogLevel {
        INFO,
        DEBUG,
        ERROR
    };

    class Logger {
    public:
        static Logger& getInstance();

        void log(LogLevel level, const std::string& message);

    private:
        Logger();

        ~Logger();

        std::string currentDateTime();

        std::string logLevelToString(LogLevel level);

        std::ofstream m_logFile;
        mutable std::mutex m_logMutex;  // 可变关键字允许在const成员函数中使用此mutex
        std::deque<std::string> m_strDeque;

        void readLogBuf();
        std::thread m_readBufThread;
        bool m_readThreadDone;
    };
} // namespace Logging

#endif //_MINILOG_H
