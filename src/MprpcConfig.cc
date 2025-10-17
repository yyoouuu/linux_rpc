#include "MprpcConfig.h"

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

void MprpcConfig::LoadConfigFile(const char* config_file) 
{
    FILE *pf = fopen(config_file, "r");
    if(pf == nullptr)
    {
        std::cout << config_file << " is not exist" << std::endl;
        exit(EXIT_FAILURE);
    }
    //1去掉注释   2正确的配置项   3 去掉开头多余的空格
    while(!feof(pf))
    {
        //按行读取配置像文件
        char buf[512] = {0};
        fgets(buf, 512, pf);

        std::string src_buf(buf);
        size_t start = 0;
        size_t end = src_buf.size();

        // 去掉所有空格和换行符
        src_buf.erase(std::remove(src_buf.begin(), src_buf.end(), ' '), src_buf.end());
        src_buf.erase(std::remove(src_buf.begin(), src_buf.end(), '\n'), src_buf.end());


        // 跳过空行或注释行
        if (src_buf.empty() || src_buf[0] == '#' || src_buf.find("//") == 0) continue;

        //解析配置项
        size_t idx = src_buf.find("=");
        if(idx == -1)
        {
            continue;
        }

        std::string key;
        std::string value;
        key = src_buf.substr(0, idx);
        value = src_buf.substr(idx+1, end-idx);
        m_configMap.insert({key, value});
    }
    fclose(pf);
}
//查询配置信息
std::string MprpcConfig::Load(const std::string key)
{
    auto it = m_configMap.find(key);
    if(it == m_configMap.end())
    {
        return "";
    }
    return it->second;
}