//
// Created by xjj-pc on 2024/9/28.
//

#include "miniLog.h"

using namespace Logging;

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::log(LogLevel level, const std::string& message) {
    std::ostringstream logStream;
    logStream << currentDateTime() << " [" << logLevelToString(level) << "] " << message << std::endl;
    std::string logEntry = logStream.str();
    std::cout << logEntry;

    // 使用 std::async 启动异步任务
    std::async(std::launch::async, [this, logEntry]() {
        std::lock_guard<std::mutex> guard(logMutex);
        logFile << logEntry;
        logFile.flush(); // 确保立即写入磁盘
    });
}

Logger::Logger() : logFile("logfile.log", std::ios_base::app) {
    if (!logFile.is_open()) {
        throw std::runtime_error("Unable to open log file.");
    }
}

Logger::~Logger() {
    logFile.close();
}

std::string Logger::currentDateTime() {
    std::time_t now = std::time(nullptr);
    char buf[80];
    struct tm* timeinfo;
    timeinfo = localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", timeinfo);
    return std::string(buf);
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::ERROR: return "ERROR";
    }
    return "UNKNOWN";
}