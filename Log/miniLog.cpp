//
// Created by xjj-pc on 2024/9/28.
//

#include "miniLog.h"

using namespace Logging;

RingChunkBuff::RingChunkBuff( int size ) : m_nProducePos(0), m_nConsumerPos(0), m_nBuffSize(size) {
    m_vecBuff.resize(size);
    sem_init(&m_semWriteToDisk, 0, 0);
}

RingChunkBuff::~RingChunkBuff() {
    sem_destroy(&m_semWriteToDisk);
}

int RingChunkBuff::getProducePos() {
    return m_nProducePos & (m_nBuffSize - 1);
}

int RingChunkBuff::getConsumerPos() {
    return m_nConsumerPos & (m_nBuffSize - 1);
}

void RingChunkBuff::incProducePos() {
    m_nProducePos = (m_nProducePos + 1) & (m_nBuffSize - 1);
}

void RingChunkBuff::incConsumerPos() {
    m_nConsumerPos = (m_nConsumerPos + 1) & (m_nBuffSize - 1);
}

void RingChunkBuff::appendToBuff( const char * data, const int length ) {
    if( length == 0 || data == nullptr ) {
        throw std::runtime_error(" appendToBuff fail ! check your length or data !");
    }

    // 全部的chunk都满时会发生写不进的问题
    if(m_vecBuff[m_nProducePos].m_u32Used + length <= m_vecBuff[m_nProducePos].m_u32Cap ) {
        memcpy(m_vecBuff[m_nProducePos].m_cMemory + m_vecBuff[m_nProducePos].m_u32Used, data, length);
        m_vecBuff[m_nProducePos].m_u32Used += length;
    } else {
        m_vecBuff[m_nProducePos].m_u32Flag = FULL;
        incProducePos();
        sem_post(&m_semWriteToDisk);  // 信号量 + 1
        if( m_vecBuff[m_nProducePos].m_u32Flag == FULL ) {
            throw std::runtime_error(" appendToBuff fail ! check your RingBUff size !");
        } else {
            memcpy(m_vecBuff[m_nProducePos].m_cMemory + m_vecBuff[m_nProducePos].m_u32Used, data, length);
            m_vecBuff[m_nProducePos].m_u32Used += length;
        }
        // appendToBuff(data, length);
    }
}

void RingChunkBuff::writeToDisk( FILE *fp ) {

    if(m_vecBuff[m_nConsumerPos].m_u32Flag == FULL ) {
        uint32_t wt_len = fwrite(m_vecBuff[m_nConsumerPos].m_cMemory, 1, m_vecBuff[m_nConsumerPos].m_u32Used, fp);
        if( wt_len != m_vecBuff[m_nConsumerPos].m_u32Used ) {
            throw std::runtime_error(" fwrite fail ! ");
        }

        fflush(fp);
        m_vecBuff[m_nConsumerPos].m_u32Flag = NOTFULL;
        m_vecBuff[m_nConsumerPos].m_u32Used = 0;
        incConsumerPos();
    }
}

void RingChunkBuff::forceWriteToDisk(FILE *fp) {
    // 防止程序结束后未标记为满的chunk 丢失，强制写入磁盘
    for(int i = 0; i < RINGBUFFSIZE; ++i )
    {
        if(m_vecBuff[m_nConsumerPos].m_u32Used != 0 ) {
            uint32_t wt_len = fwrite(m_vecBuff[m_nConsumerPos].m_cMemory, 1, m_vecBuff[m_nConsumerPos].m_u32Used, fp);
            if( wt_len != m_vecBuff[m_nConsumerPos].m_u32Used ) {
                throw std::runtime_error(" fwrite fail ! ");
            }
            fflush(fp);
            m_vecBuff[m_nConsumerPos].m_u32Used = 0;
            incConsumerPos();
        } else {
            // sem_close(&m_semWriteToDisk);
            break;
        }
    }
}

sem_t & RingChunkBuff::getSemWriteToDisk() {
    return m_semWriteToDisk;
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::log( LogLevel level, char * message) {

    std::lock_guard<std::mutex> guard(m_logMutex);
    currentDateTime(m_pTmpCache, 512);
    int len = strlen(message);
    memcpy(m_pTmpCache + 19, message, len);
    m_pRingChunkBuff->appendToBuff(m_pTmpCache, len + 19);
}

void Logger::readLogBuf() {
    while( m_readThreadDone ) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts); // 获取当前时间,与std::time 不同为纳秒级
        ts.tv_sec += 1;

        sem_timedwait(&m_pRingChunkBuff->getSemWriteToDisk(), &ts);   // 1 s 后强制解除等待，避免死等

        m_pRingChunkBuff->writeToDisk(m_pFilePoint);
    }
}

Logger::Logger() : m_readBufThread(&Logger::readLogBuf, this) {
    m_pRingChunkBuff = new RingChunkBuff();
    m_pTmpCache = new char[512];
    m_readThreadDone = true;
    m_pFilePoint = fopen("logfile.log", "w");
    if( m_pFilePoint == nullptr ) {
        throw std::runtime_error("Unable to open log file.");
    }
}

Logger::~Logger() {
    m_pRingChunkBuff->forceWriteToDisk(m_pFilePoint);
    m_readThreadDone = false;
    if ( m_readBufThread.joinable() ) {
        m_readBufThread.join(); // 或者根据需要调用 detach()
    }
    safe_delete(m_pRingChunkBuff);
    safe_delete(m_pTmpCache);
    fclose(m_pFilePoint);
}

void Logger::currentDateTime(char* buffer, int bufferSize) {
    std::time_t now = std::time(nullptr);
    if( m_lastTime && difftime(m_lastTime, now) <= 0.1 ) {
        return;
    }

    m_lastTime = now;
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, bufferSize, "%Y-%m-%d %X", timeinfo);
}

std::string Logger::logLevelToString( LogLevel & level ) {
    switch ( level ) {
        case LogLevel::INFO: return "[INFO]";
        case LogLevel::DEBUG: return "[DEBUG]";
        case LogLevel::ERROR: return "[ERROR]";
    }
    return "UNKNOWN";
}