#pragma once
#include "LockQueue.h"
#include <string>
#include <iostream>
#include <sys/stat.h>  // mkdir
#include <sys/types.h> // mode_t
#include <unistd.h>    // access
#include <string.h>    // strerror

enum Loglevel
{
    INFO,
    ERROR,
};
//Mprpc框架提供的日志系统
class Logger
{
private:
    int m_loglevel;
    LockQueue<std::string> m_lockqueue;

    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
public:
    //获取日志的单例
    static Logger& GetInstance();
    void SetLogLevel(Loglevel level);
    void Log(std::string msg);
};

#define LOG_INFO(logmsgformat, ...)                        \
    do                                                     \
    {                                                      \
        Logger& logger = Logger::GetInstance();            \
        logger.SetLogLevel(INFO);                          \
        char c[1024] = {0};                                \
        snprintf(c, sizeof(c), "[INFO]  " logmsgformat, ##__VA_ARGS__);  \
        logger.Log(c);                                     \
    } while (0)

#define LOG_ERR(logmsgformat, ...)                         \
    do                                                     \
    {                                                      \
        Logger& logger = Logger::GetInstance();            \
        logger.SetLogLevel(ERROR);                         \
        char c[1024] = {0};                                \
        snprintf(c, sizeof(c), "[ERROR] " logmsgformat, ##__VA_ARGS__); \
        logger.Log(c);                                     \
    } while (0)




