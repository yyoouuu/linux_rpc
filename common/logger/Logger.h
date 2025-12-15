#pragma once
#include "LockQueue.h"
#include <string>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

enum Loglevel
{
    INFO,
    WARN,
    ERROR,
    FATAL,
};

namespace amoureux
{
    class Logger
    {
    private:
        LockQueue<std::string> _lockqueue;
        int _loglevel;
        Logger();
        //单例模式
        // 禁止拷贝构造
        Logger(const Logger&) = delete;

        // 禁止拷贝赋值
        Logger& operator=(const Logger&) = delete;

        // 禁止移动构造
        Logger(Logger&&) = delete;

        // 禁止移动赋值
        Logger& operator=(Logger&&) = delete;
    public:
        // 获取日志的单例
        static Logger &GetInstance();
        void SetLogLevel(Loglevel level);
        void Log(std::string msg);
    };
};

#define LOG_INFO(logmsgformat, ...)                        \
    do                                                     \
    {                                                      \
        Logger& logger = Logger::GetInstance();            \
        logger.SetLogLevel(INFO);                          \
        char c[1024] = {0};                                \
        snprintf(c, sizeof(c), "[INFO]  " logmsgformat, ##__VA_ARGS__);  \
        logger.Log(c);                                     \
    } while (0) \

#define LOG_WARN(logmsgformat, ...)                         \
    do                                                     \
    {                                                      \
        Logger& logger = Logger::GetInstance();            \
        logger.SetLogLevel(WARN);                         \
        char c[1024] = {0};                                \
        snprintf(c, sizeof(c), "[WARN] " logmsgformat, ##__VA_ARGS__); \
        logger.Log(c);                                     \
    } while (0) \

#define LOG_ERROR(logmsgformat, ...)                        \
    do                                                     \
    {                                                      \
        Logger& logger = Logger::GetInstance();            \
        logger.SetLogLevel(ERROR);                          \
        char c[1024] = {0};                                \
        snprintf(c, sizeof(c), "[ERROR]  " logmsgformat, ##__VA_ARGS__);  \
        logger.Log(c);                                     \
    } while (0) \

#define LOG_FATAL(logmsgformat, ...)                         \
    do                                                     \
    {                                                      \
        Logger& logger = Logger::GetInstance();            \
        logger.SetLogLevel(FATAL);                         \
        char c[1024] = {0};                                \
        snprintf(c, sizeof(c), "[FATAL] " logmsgformat, ##__VA_ARGS__); \
        logger.Log(c);                                     \
    } while (0)\