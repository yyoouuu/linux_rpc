#include <iostream>
#include "MprpcApplication.h"
#include "user.pb.h"


int main(int argc, char** argv)
{
    //整个程序启动后，想使用Mprpc框架来使用rpc服务调用，必须线调用框架的初始化函数
    MprpcApplication::Init(argc,argv);

    //演示调用远程发布的rpc发给发login
    fixbug::UserServiceRPC_Stub stub(new MprpcChannel());
    //请求参数构造
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    //响应参数构造
    fixbug::LoginResponse response;
    //发起rpc方法的调用， 同步的rpc调用过程  MpcChannel::callmethod
    stub.Login(nullptr, &request, &response, nullptr);//进入到--->rpcchannnnel->callmethod()

    // 响应处理
    if (response.result().errcode() == 0)
    {
        std::cout << "rpc login response success: " << response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login failed. errcode: " << response.result().errcode()
                  << ", errmsg: " << response.result().errmsg() << std::endl;
    }

    //请求参数构造
    fixbug::RegisterRequest registerrequest;
    registerrequest.set_id(2000);
    registerrequest.set_name("zhang san");
    registerrequest.set_pwd("123456");
    //响应参数构造
    fixbug::RegisterResponse registerresponse;
    //发起rpc方法的调用
    stub.Register(nullptr, &registerrequest, &registerresponse, nullptr);//进入到--->rpcchannnnel->callmethod()
    // 响应处理
    if (registerresponse.result().errcode() == 0)
    {
        std::cout << "rpc Register registerresponse success: " << registerresponse.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login failed. errcode: " << registerresponse.result().errcode()
                << ", errmsg: " << registerresponse.result().errmsg() << std::endl;
    }

    return 0;
}