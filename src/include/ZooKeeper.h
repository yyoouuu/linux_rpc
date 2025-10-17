#pragma once
#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

class ZkClient {
private:
    zhandle_t* m_zhandle;
    sem_t m_sem;            // 信号量用于异步同步
    std::string m_result;   // 用于保存回调结果
    int m_rc;               // 回调返回码

public:
    ZkClient();
    ~ZkClient();

    void Start();
    void Create(const char* path, const char* data, int datalen, int state = 0);
    std::string GetData(const char* path);

    // 使回调函数可以访问私有成员
    friend void exists_callback(int rc, const struct Stat* stat, const void* data);
    friend void create_callback(int rc, const char* value, const void* data);
    friend void get_callback(int rc, const char* value, int value_len, const struct Stat* stat, const void* data);
};


