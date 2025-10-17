#include <iostream>
#include "MprpcApplication.h"
#include "friend.pb.h"

int main(int argc, char** argv)
{
    //整个程序启动后，想使用Mprpc框架来使用rpc服务调用，必须线调用框架的初始化函数
    MprpcApplication::Init(argc,argv);

    //演示调用远程发布的rpc发给发GetFriendList
    fixbug::FriendServiceRPC_Stub stub(new MprpcChannel());
    //请求参数构造
    MprpcContoller controller;
    fixbug::GetFriendListRequest request;
    request.set_userid(2000);
    //响应参数构造
    fixbug::GetFriendListResponse response;
    //发起rpc方法的调用， 同步的rpc调用过程  MpcChannel::callmethod
    stub.GetFriendList(&controller, &request, &response, nullptr);//进入到--->rpcchannnnel->callmethod()
    if(controller.Failed())
    {
        std::cout << controller.ErrorText() <<std::endl;
    }
    else
    {
            // 响应处理u
        if (response.result().errcode() == 0)
        {
            std::cout << "rpc GetFriendList response success: " << std::endl;
            int size = response.friends_size();
            for(int i = 0; i < size; ++i)
            {
                std::cout << "index: " << (i+1) << " name: " << response.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc GetFriendList failed. errcode: " << response.result().errcode()
                    << ", errmsg: " << response.result().errmsg() << std::endl;
        }
    }

    return 0;
}