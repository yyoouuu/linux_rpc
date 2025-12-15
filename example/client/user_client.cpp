#include <iostream>
#include "RpcApplication.h"
#include "user.pb.h"
#include "RpcClient.h"

int main(int argc, char **argv)
{
    // 整个程序启动以后，想使用mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    amoureux::RpcApplication::Init(argc, argv);

    amoureux_proto::UserService_Stub stub(new amoureux::RpcClient());
    // rpc方法的请求参数
    amoureux_proto::UserRegisterReq request;
    request.set_request_id("123456");
    request.set_nickname("amoureux");
    request.set_password("123456");

    // rpc方法的响应
    amoureux_proto::UserRegisterRsp response;
    // 发起rpc方法的调用  同步的rpc调用过程  MprpcChannel::callmethod
    //同步阻塞的方式
    // RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送
    stub.UserRegister(nullptr, &request, &response, nullptr); 

    // 一次rpc调用完成，读调用的结果
    if (0 == response.result().errcode())
    {
        //调用正确
        std::cout << "rpc login response success:" << response.success() << std::endl;
    }
    else
    {
        //调用失败
        std::cout << "rpc login response error : " << response.result().errcode() << std::endl;
    }
    return 0;
}