#include "ConfigManager.h"

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

void amoureux::ConfigManager::LoadConfigFile(const char* config_file) 
{
    FILE *pf = fopen(config_file, "r");
    if(!pf)
    {
        std::cout << config_file << " is not exist" << std::endl;
        exit(EXIT_FAILURE);
    }
    _config_file = config_file;
    char buf[512];
    while(fgets(buf, sizeof(buf), pf) != nullptr)
    {
        std::string src_buf(buf);
        // 去掉空格、换行、回车
        src_buf.erase(std::remove_if(src_buf.begin(), src_buf.end(),
            [](char c){ return c == ' ' || c == '\n' || c == '\r'; }), src_buf.end());
        // 跳过空行或注释
        if(src_buf.empty() || src_buf[0] == '#' || src_buf.find("//") == 0) 
            continue;
        // 找到 '=' 和 ':'
        size_t idx = src_buf.find('=');
        size_t idy = src_buf.find(':');
        if(idx == std::string::npos || idy == std::string::npos)
            continue;

        //将服务方法名和ip插入映射表
        std::string service_method = src_buf.substr(0, idx);
        std::string ip = src_buf.substr(idx + 1, idy - idx - 1);
        uint16_t port = static_cast<uint16_t>(std::stoi(src_buf.substr(idy + 1)));
        Endpoint endpoint{ip, port};
        // 插入或追加
        _configMap[service_method].push_back(endpoint);
    }

    fclose(pf);
}

void amoureux::ConfigManager::WriteConfigFile(const std::string service_method) 
{
    FILE *pf = fopen(_config_file, "a");
    if(!pf)
    {
        std::cout << _config_file << " is not exist" << std::endl;
        exit(EXIT_FAILURE);
    }
    auto it = _configMap.find(service_method);
    if (it == _configMap.end())
    {
        fclose(pf);
        return;
    }
    for (const auto& ep : it->second)
    {
        // 拼接一行：service_method=host:port\n
        std::string line = service_method + "=" +
                           ep.host + ":" +
                           std::to_string(ep.port) + "\n";

        fputs(line.c_str(), pf);
    }
    fclose(pf);
}

//查询配置信息
amoureux::Endpoint amoureux::ConfigManager::client_rbselect(const std::string service_method)
{
    auto it = _configMap.find(service_method);
    if(it != _configMap.end() && !it->second.empty())
    {
        return _rbloadbalancer.Select(service_method, _configMap[service_method]);
    }

    // 本地没有，查询 ZooKeeper
    ZkClient _zkclient; // 创建一个 ZkClient 实例，用于与 ZooKeeper 服务进行通信。
    _zkclient.Start(); 
    std::vector<Endpoint> eps = _zkclient.List(service_method.c_str());
    if(!eps.empty()) 
    {
            _configMap[service_method] = eps;
            WriteConfigFile(service_method);
    }

    if(!eps.empty())
        return _rbloadbalancer.Select(service_method, _configMap[service_method]);
    return Endpoint{};
}

amoureux::Endpoint amoureux::ConfigManager::server_select(const std::string service_method)
{
    auto it = _configMap.find(service_method);
    if(it != _configMap.end() && !it->second.empty())
    {
        const Endpoint& ep = it->second.front();
        return ep;
    }

    return Endpoint{};
}

amoureux::Endpoint amoureux::ConfigManager::zk_ip_select(const std::string service_method)
{
    auto it = _configMap.find(service_method);
    if(it != _configMap.end() && !it->second.empty())
    {
        const Endpoint& ep = it->second.front();
        return ep;
    }

    return Endpoint{};
}