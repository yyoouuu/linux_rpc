#pragma once

#include "MprpcConfig.h"
#include "MprpcChannel.h"
#include "MprpcContoller.h"

//mprpc框架的基础类
class MprpcApplication
{
private:
    static MprpcConfig m_config;
    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
    
public:
    static void Init(int argc, char **argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();
};

