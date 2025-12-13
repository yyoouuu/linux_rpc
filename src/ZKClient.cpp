#include "ZKClient.h"
#include "RpcApplication.h"
#include <iostream>

// 全局 Watcher 回调函数：服务发现、节点监听、上下线实时同步、心跳与重连检测
void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT && state == ZOO_CONNECTED_STATE)
    {
        sem_t *sem = (sem_t *)zoo_get_context(zh);
        sem_post(sem);
    }
}

namespace amoureux
{

    void exists_callback(int rc, const struct Stat *stat, const void *data)
    {
        ZkClient *client = (ZkClient *)data;
        client->_rc = rc;
        sem_post(&client->_sem);
    }

    void create_callback(int rc, const char *value, const void *data)
    {
        ZkClient *client = (ZkClient *)data;
        client->_rc = rc;
        if (value != nullptr)
            client->_result = value;
        sem_post(&client->_sem);
    }

    void get_callback(int rc, const char *value, int value_len, const struct Stat *stat, const void *data)
    {
        ZkClient *client = (ZkClient *)data;
        client->_rc = rc;
        if (rc == ZOK && value != nullptr)
            client->_result = std::string(value, value_len);
        sem_post(&client->_sem);
    }

    void get_children_callback(int rc,
                               const struct String_vector *strings,
                               const struct Stat *stat,
                               const void *data)
    {
        ZkClient *client = (ZkClient *)data;
        client->_rc = rc;
        client->_children.clear();

        if (rc == ZOK && strings != nullptr)
        {
            for (int i = 0; i < strings->count; ++i)
            {
                client->_children.emplace_back(strings->data[i]);
            }
        }

        sem_post(&client->_sem);
    }

}

// 构造函数
amoureux::ZkClient::ZkClient() : _zhandle(nullptr), _rc(-1)
{
    sem_init(&_sem, 0, 0);
}

// 析构函数
amoureux::ZkClient::~ZkClient()
{
    if (_zhandle != nullptr)
    {
        zookeeper_close(_zhandle);
    }
    sem_destroy(&_sem);
}

// 启动 Zookeeper 客户端
void amoureux::ZkClient::Start()
{
    // 从配置文件中获取zookeeper的ip地址和端口号
    Endpoint endpoint = amoureux::RpcApplication::GetInstance().GetConfig().server_select("/server1/method1");
        
    std::string connstr = endpoint.host + ":" + std::to_string(endpoint.port);
    // 初始化
    _zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (_zhandle == nullptr)
    {
        std::cout << "zookeeper_init error" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(_zhandle, &sem);
    sem_wait(&sem);
    sem_destroy(&sem);

    std::cout << "zookeeper_init success!" << std::endl;
}

// 异步创建节点
void amoureux::ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    _rc = -1;
    _result.clear();
    zoo_aexists(_zhandle, path, 0, exists_callback, this);
    sem_wait(&_sem);

    if (_rc == ZNONODE)
    {
        _rc = -1;
        _result.clear();

        zoo_acreate(_zhandle, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE, state, create_callback, this);
        sem_wait(&_sem);

        if (_rc == ZOK)
        {
            std::cout << "zoo_acreate success... path:" << path << std::endl;
        }
        else
        {
            std::cout << "zoo_acreate error... path:" << path << ", rc:" << _rc << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

std::vector<amoureux::Endpoint>
amoureux::ZkClient::List(const char *path)
{
    std::vector<Endpoint> result;
    std::vector<std::string> children = GetChildren(path);

    for (const auto &child : children)
    {
        size_t pos = child.find(':');
        if (pos == std::string::npos)
            continue;

        Endpoint ep;
        ep.host = child.substr(0, pos);
        ep.port = static_cast<uint16_t>(std::stoi(child.substr(pos + 1)));

        result.push_back(std::move(ep));
    }

    return result;
}

std::vector<std::string> amoureux::ZkClient::GetChildren(const char *path)
{
    _rc = -1;
    _children.clear();

    zoo_aget_children2(
        _zhandle,
        path,
        1, // 不设置 watcher（后面可扩展）
        get_children_callback,
        this);

    sem_wait(&_sem);

    if (_rc != ZOK)
    {
        std::cout << "zoo_aget_children2 error, path: "
                  << path << ", rc: " << _rc << std::endl;
        return {};
    }

    return _children;
}