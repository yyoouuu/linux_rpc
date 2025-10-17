#include "ZooKeeper.h"
#include "MprpcApplication.h"
#include <iostream>

// 全局 Watcher 回调函数
void global_watcher(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx) {
    if (type == ZOO_SESSION_EVENT && state == ZOO_CONNECTED_STATE) {
        sem_t* sem = (sem_t*)zoo_get_context(zh);
        sem_post(sem);
    }
}

// 异步 exists 回调
void exists_callback(int rc, const struct Stat* stat, const void* data) {
    ZkClient* client = (ZkClient*)data;
    client->m_rc = rc;
    sem_post(&client->m_sem);
}

// 异步 create 回调
void create_callback(int rc, const char* value, const void* data) {
    ZkClient* client = (ZkClient*)data;
    client->m_rc = rc;
    if (value != nullptr) {
        client->m_result = value;
    }
    sem_post(&client->m_sem);
}

// 异步 get 回调
void get_callback(int rc, const char* value, int value_len, const struct Stat* stat, const void* data) {
    ZkClient* client = (ZkClient*)data;
    client->m_rc = rc;
    if (rc == ZOK && value != nullptr) {
        client->m_result = std::string(value, value_len);
    }
    sem_post(&client->m_sem);
}

// 构造函数
ZkClient::ZkClient() : m_zhandle(nullptr), m_rc(-1) {
    sem_init(&m_sem, 0, 0);
}

// 析构函数
ZkClient::~ZkClient() {
    if (m_zhandle != nullptr) {
        zookeeper_close(m_zhandle);
    }
    sem_destroy(&m_sem);
}

// 启动 Zookeeper 客户端
void ZkClient::Start() {
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeper_ip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeper_port");
    std::string connstr = host + ":" + port;

    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (m_zhandle == nullptr) {
        std::cout << "zookeeper_init error" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);
    sem_wait(&sem);
    sem_destroy(&sem);

    std::cout << "zookeeper_init success!" << std::endl;
}

// 异步创建节点
void ZkClient::Create(const char* path, const char* data, int datalen, int state) {
    m_rc = -1;
    m_result.clear();

    zoo_aexists(m_zhandle, path, 0, exists_callback, this);
    sem_wait(&m_sem);

    if (m_rc == ZNONODE) {
        m_rc = -1;
        m_result.clear();

        zoo_acreate(m_zhandle, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE, state, create_callback, this);
        sem_wait(&m_sem);

        if (m_rc == ZOK) {
            std::cout << "zoo_acreate success... path:" << path << std::endl;
        } else {
            std::cout << "zoo_acreate error... path:" << path << ", rc:" << m_rc << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

// 异步获取节点数据
std::string ZkClient::GetData(const char* path) {
    m_rc = -1;
    m_result.clear();

    zoo_aget(m_zhandle, path, 0, get_callback, this);
    sem_wait(&m_sem);

    if (m_rc != ZOK) {
        std::cout << "zoo_aget error... path:" << path << ", rc:" << m_rc << std::endl;
        return "";
    }

    return m_result;
}
