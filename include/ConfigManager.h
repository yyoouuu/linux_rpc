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
            RoundRobinLoadBalancer _rbloadbalancer;
            const char* _config_file;
        public:
            //负责解析加载配置文件
            void LoadConfigFile(const char* config_file);
            void WriteConfigFile(const std::string service_method);
            //查询配置信息
            amoureux::Endpoint client_rbselect(const std::string key);
            amoureux::Endpoint server_select(const std::string key);
            amoureux::Endpoint zk_ip_select(const std::string key);
    };
}