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

        std::string service_method = src_buf.substr(0, idx);
        std::string ip = src_buf.substr(idx + 1, idy - idx - 1);
        uint16_t port = static_cast<uint16_t>(std::stoi(src_buf.substr(idy + 1)));

        Endpoint endpoint{ip, port};

        // 插入或追加
        _configMap[service_method].push_back(endpoint);
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
    std::vector<Endpoint> eps = _zkclient.List(service_method.c_str());
    if(!eps.empty()) _configMap[service_method] = eps;

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