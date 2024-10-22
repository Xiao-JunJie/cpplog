//
// Created by xjj-pc on 2024/9/28.
//

#ifndef _MINILOG_H
#define _MINILOG_H

#include "SpinLock.h"
#include <iostream>
#include <string.h>
#include <vector>
#include <mutex>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <semaphore.h>

#define LOG_INFO(fmt, args...) do        \
{                                        \
    char buff[512] = {0};                \
    snprintf(buff, sizeof(buff),  "%s[%s:%d]" fmt "\n", "[INFO]", __FUNCTION__, __LINE__, ##args);   \
    Logging::Logger::getInstance().log( buff );                                                      \
} while(false);

#define LOG_DEBUG(fmt, args...) do      \
{                                       \
    char buff[512] = {0};               \
    snprintf(buff, sizeof(buff),  "%s[%s:%d]" fmt "\n", "[DEBUG]", __FUNCTION__, __LINE__, ##args);   \
    Logging::Logger::getInstance().log( buff );                                                       \
} while(false);

#define LOG_ERR(fmt, args...) do        \
{                                       \
    char buff[512] = {0};               \
    snprintf(buff, sizeof(buff),  "%s[%s:%d]" fmt "\n", "[ERR]", __FUNCTION__, __LINE__, ##args);      \
    Logging::Logger::getInstance().log( buff );                                                        \
} while(false)

#define SET_LOG(filename, maxsize) Logging::Logger::getInstance(filename, maxsize)

#define safe_delete(p) do       \
{                               \
     if( (p) != nullptr ) {     \
         delete (p);            \
         (p) = nullptr;         \
     }                          \
} while(false)

#define FULL 1
#define NOTFULL 0

#define CHUNKMEMSIZE (1024 * 256)

// 一定是2的n次幂
#define RINGBUFFSIZE 64

namespace Logging
{
    enum class LogLevel {
        INFO,
        DEBUG,
        ERROR
    };

    struct Chunk {
        uint32_t m_u32Cap;
        uint32_t m_u32Used;
        uint32_t m_u32Flag;
        char * m_cMemory;

        Chunk() : m_u32Cap(CHUNKMEMSIZE), m_u32Used(0), m_u32Flag(NOTFULL) {
            m_cMemory = new char[CHUNKMEMSIZE];
        }
        ~Chunk() {
            safe_delete(m_cMemory);
        }
    };

    class RingChunkBuff {
    public:

        explicit RingChunkBuff( const int size = RINGBUFFSIZE );

        ~RingChunkBuff();

        int getProducePos();

        int getConsumerPos();

        void incProducePos();

        void incConsumerPos();

        void appendToBuff( const char * data, const int length );

        void writeToDisk(FILE *fp);

        void forceWriteToDisk(FILE *fp);

        sem_t & getSemWriteToDisk();

    private:
        std::vector<Chunk> m_vecBuff;
        int m_nProducePos;     // 更改只在appendToBuf,而该函数进入前是上锁的
        int m_nConsumerPos;    // 单线程改变
        int m_nBuffSize;

        sem_t m_semWriteToDisk;
    };

    class Logger {
    public:
        Logger(const Logger & ) = delete;

        Logger & operator = (const Logger & ) = delete;

        static Logger& getInstance(std::string filename = "logfile.log", int maxsize = 200);

        void log( char * message);

        void currentDateTime(char* buffer, int bufferSize);

    private:
        explicit Logger(std::string & filename, int & maxsize);

        ~Logger();

        std::string logLevelToString( LogLevel &level );

        SpinLock m_spinLock;
        mutable std::mutex m_logMutex;          // 允许在const成员函数中使用此mutex
        FILE *m_pFilePoint;
        RingChunkBuff *m_pRingChunkBuff;

        void readLogBuf();
        std::thread m_readBufThread;
        bool m_readThreadDone;

        char * m_pTmpCache;        // log数据的组装缓存

        std::time_t m_lastTime;    // 更近的日志时间

        std::string m_logFileName;

        int m_logMaxSize;         // mb
    };
} // namespace Logging

#endif //_MINILOG_H
