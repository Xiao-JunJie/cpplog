//
// Created by xjj-pc on 2024/9/28.
//

#ifndef _MINILOG_H
#define _MINILOG_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <thread>
#include <stdio.h>
// #include <cstdarg>

#define LOG_INFO(fmt, args...) do        \
{                                        \
    char buff[1024] = {0};               \
    snprintf(buff, sizeof(buff), "[%s:%d]" fmt, __FUNCTION__, __LINE__, ##args); \
    Logging::Logger::getInstance().log(Logging::LogLevel::INFO, buff);           \
} while(false);

#define LOG_DEBUG(fmt, args...) do      \
{                                       \
    char buff[1024] = {0};              \
    snprintf(buff, sizeof(buff), "[%s:%d]" fmt, __FUNCTION__, __LINE__, ##args); \
    Logging::Logger::getInstance().log(Logging::LogLevel::DEBUG, buff);          \
} while(false);

#define LOG_ERR(fmt, args...) do        \
{                                       \
    char buff[1024] = {0};              \
    snprintf(buff, sizeof(buff), "[%s:%d]" fmt, __FUNCTION__, __LINE__, ##args); \
    Logging::Logger::getInstance().log(Logging::LogLevel::ERROR, buff);          \
} while(false)

#define safe_delete(p) do       \
{                               \
     if( (p) != nullptr ) {     \
         delete (p);            \
         (p) = nullptr;         \
     }                          \
} while(false)

#define FULL 1
#define NOTFULL 0
#define RINGBUFFSIZE 4

namespace Logging
{
    enum class LogLevel {
        INFO,
        DEBUG,
        ERROR
    };

    struct Chunk {
        uint32_t cap;
        uint32_t used;
        uint32_t flag;
        char * memory;

        Chunk() : cap(1024 * 1024 * 4), used(0), flag(NOTFULL) {
            memory = new char[1024 * 1024 * 4];
        }
        ~Chunk() {
            safe_delete(memory);
        }
    };

    class RingChunkBuff {
    public:

        RingChunkBuff( const int size = RINGBUFFSIZE );

        ~RingChunkBuff() = default;

        int getProducePos();

        int getConsumerPos();

        void incProducePos();

        void incConsumerPos();

        void appendToBuff( const std::string &data, const int length );

        void writeToDisk(FILE *fp);

        void forceWriteToDisk(FILE *fp);

    private:
        std::vector<Chunk> m_vecBuff;
        int m_nProducePos;
        int m_nConsumerPos;
        int m_nBuffSize;
    };

    class Logger {
    public:
        static Logger& getInstance();

        void log( LogLevel level, const std::string& message );

    private:
        Logger();

        ~Logger();

        std::string currentDateTime();

        std::string logLevelToString( LogLevel level );

        mutable std::mutex m_logMutex;       // 可变关键字允许在const成员函数中使用此mutex
        FILE *m_pFilePoint;
        RingChunkBuff *m_pRingChunkBuff;

        void readLogBuf();
        std::thread m_readBufThread;
        bool m_readThreadDone;
    };
} // namespace Logging

#endif //_MINILOG_H
