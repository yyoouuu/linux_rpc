#pragma once

#include "ConfigManager.h"//用于初始框架配置文件

namespace amoureux
{
    //全局唯一实例（单例）:构造函数私有化，禁用拷贝和移动构造，懒汉式加载(c++11后线程安全)，GetInstance()获取静态实例程序结束后自动释放
    class RpcApplication
    {
    private:
        static ConfigManager _config;
        RpcApplication() {}
        RpcApplication(const RpcApplication &) = delete;
        RpcApplication(RpcApplication &&) = delete;

    public:
        static void Init(int argc, char **argv);
        static RpcApplication &GetInstance();
        static ConfigManager &GetConfig();
    };
}
