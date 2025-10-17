#include "Logger.h"

//获取日志的单例
Logger& Logger::GetInstance()
{
    static Logger logger;
    return logger;
}


Logger::Logger()
{
    std::thread writeLogTask([this]() {
        for (;;) {
            // 创建日志目录（Log），如果不存在就创建
            const char* log_dir = "./Log";
            if (access(log_dir, F_OK) == -1) {
                if (mkdir(log_dir, 0777) == -1) {
                    std::cerr << "Failed to create log directory: " << strerror(errno) << std::endl;
                    exit(EXIT_FAILURE);
                }
            }

            // 获取当前日期，构造日志文件名
            time_t now = time(nullptr);
            tm* nowtm = localtime(&now);
            char file_path[256];
            sprintf(file_path, "%s/%04d-%02d-%02d-log.txt",
                    log_dir,
                    nowtm->tm_year + 1900,
                    nowtm->tm_mon + 1,
                    nowtm->tm_mday);

            FILE* pf = fopen(file_path, "a+");
            if (pf == nullptr) {
                std::cerr << "file_path: " << file_path << " open error: " << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            }

            // 阻塞等待日志信息
            std::string msg = m_lockqueue.Pop();

            // 获取当前时间（用于打日志前缀）
            now = time(nullptr);
            nowtm = localtime(&now);
            char time_buf[64] = {0};
            sprintf(time_buf, "[%04d-%02d-%02d %02d:%02d:%02d]",
                    nowtm->tm_year + 1900,
                    nowtm->tm_mon + 1,
                    nowtm->tm_mday,
                    nowtm->tm_hour,
                    nowtm->tm_min,
                    nowtm->tm_sec);

            // 拼接日志消息，包含时间戳和日志级别
            std::string log_line = time_buf + msg + "\n";
            fputs(log_line.c_str(), pf);
            fclose(pf);
        }
    });

    writeLogTask.detach(); // 分离线程
}


void Logger::SetLogLevel(Loglevel level)
{
    m_loglevel = level;
}

//把日志信息写入lockqueue缓冲区中
void Logger::Log(std::string msg)
{
    m_lockqueue.Push(msg);
}