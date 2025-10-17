#pragma once  // 防止头文件被多次包含

#include "google/protobuf/service.h"  

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Timestamp.h>
#include <functional>
#include <string>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>  // 👈 加上这个！

#include <unordered_map>

// 框架提供的用于服务发布的类
class RpcProvider
{
private:
    muduo::net::EventLoop m_eventloop;

    //service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };
    //存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;
    //新的socket连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr& conn);
    //已建立用户的读写事件回调
    void OnMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer, muduo::Timestamp timestamp);

    //Closure的回调函数,用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr& ,google::protobuf::Message*);
public:
    // 注册服务方法
    // 参数 service 是一个继承自 google::protobuf::Service 的业务服务对象
    // 所有业务服务都应继承自 Service，因此该方法可以注册任意类型的服务
    void NotifyService(google::protobuf::Service* service);

    void run();
};

