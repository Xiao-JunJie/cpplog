//
// Created by xjj-pc on 2024/9/28.
//

#include "miniLog.h"

using namespace Logging;

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

void Logger::log(LogLevel level, const std::string& message)
{
    std::ostringstream logStream;
    logStream << currentDateTime() << " [" << logLevelToString(level) << "] " << message << std::endl;
    std::string logEntry = logStream.str();

    std::lock_guard<std::mutex> guard(m_logMutex);
    m_strDeque.push_front(logEntry);
}

void Logger::readLogBuf()
{
    while(m_readThreadDone)
    {
        while(!m_strDeque.empty())
        {
            std::lock_guard<std::mutex> guard(m_logMutex);
            m_logFile << m_strDeque.back();
            m_strDeque.pop_back();
        }
        m_logFile.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 休眠100毫秒
    }
}

Logger::Logger() : m_logFile("logfile.log", std::ios::out | std::ios::trunc)
{
    m_readThreadDone = true;
    m_readBufThread = std::thread(&Logger::readLogBuf, this);
    if (!m_logFile.is_open()) {
        throw std::runtime_error("Unable to open log file.");
    }
}

Logger::~Logger() {
    m_logFile.close();
    m_readThreadDone = false;
    if (m_readBufThread.joinable()) {
        m_readBufThread.join(); // 或者根据需要调用 detach()
    }
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