#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include "ZKClient.h"
#include "LoadBalancer.h"
namespace amoureux
{
    class ConfigManager
    {
        private:
            std::vector<Endpoint> endpoints;
            std::unordered_map<std::string, std::vector<Endpoint>> _configMap;
            ZkClient _zkclient;
            RoundRobinLoadBalancer _rbloadbalancer;
        public:
            ConfigManager(ZkClient& zkclient);
            //负责解析加载配置文件
            void LoadConfigFile(const char* config_file);
            //查询配置信息
            amoureux::Endpoint client_select(const std::string key);
    };
}