#pragma once
#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>
#include <vector>
namespace amoureux
{
    struct Endpoint
    {
        std::string host; // 主机名或 IP
        uint16_t port{0}; // 端口
    };
    class ZkClient
    {
    private:
        zhandle_t *_zhandle;
        sem_t _sem;          // 信号量用于异步同步
        std::string _result; // 用于保存回调结果
        std::vector<std::string> _children;
        Endpoint _endpoints;
        int _rc; // 回调返回码

    public:
        ZkClient();
        ~ZkClient();

        void Start();
        // 创建节点
        void Create(const char *path, const char *data, int datalen, int state = 0);
        // 列出某个服务的所有节点
        std::vector<Endpoint> List(const char *path);
        std::vector<std::string> GetChildren(const char *path);
        // 使回调函数可以访问私有成员
        friend void exists_callback(int rc, const struct Stat *stat, const void *data);
        friend void create_callback(int rc, const char *value, const void *data);
        friend void get_callback(int rc, const char *value, int value_len, const struct Stat *stat, const void *data);
        friend void get_children_callback(int rc,
                                          const struct String_vector *strings,
                                          const struct Stat *stat,
                                          const void *data);
    };
}
