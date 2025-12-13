#pragma once

#include "ConfigManager.h"//用于初始框架配置文件

namespace amoureux
{
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
