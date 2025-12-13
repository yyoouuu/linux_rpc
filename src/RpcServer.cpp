#include "RpcServer.h"
#include "RpcApplication.h"
#include "rpcheader.pb.h"
#include "Logger.h"
#include "ZKClient.h"

namespace amoureux
{
    void RpcServer::NotifyService(google::protobuf::Service *service)
    {
        ServiceInfo service_info;

        // 获取服务对象的描述信息（服务名、方法列表等）
        const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();

        // 获取服务名称
        std::string service_name = pserviceDesc->name();

        // 获取该服务提供的方法数量
        int methodCnt = pserviceDesc->method_count();

        LOG_INFO("service_name : %s", service_name.c_str());
        // 遍历所有方法，构建方法名和方法描述的映射
        for (int i = 0; i < methodCnt; ++i)
        {
            // 获取第 i 个方法的描述信息
            const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
            std::string method_name = pmethodDesc->name();

            LOG_INFO("method_name : %s", method_name.c_str());

            // 保存方法名和方法描述的映射
            service_info._methodMap.insert({method_name, pmethodDesc});
        }

        // 保存服务对象本身
        service_info._service = service;

        // 将服务名与其对应的服务信息映射保存到类的成员变量中
        _serviceMap.insert({service_name, service_info});
    }

    // 启动 RPC 网络服务，开始对外提供远程调用服务
    void RpcServer::run()
    {
        // 从配置文件中读取服务端 IP 和端口
        Endpoint endpoint = amoureux::RpcApplication::GetInstance().GetConfig().server_select("/server1/method1");
        
        muduo::net::InetAddress address(endpoint.host, endpoint.port); // 封装地址信息

        // 创建 Muduo 网络库中的 TcpServer 对象
        muduo::net::TcpServer server(&_eventloop, address, "RpcProvider");

        // 注册连接建立/断开时的回调函数
        server.setConnectionCallback(
            std::bind(&RpcServer::OnConnection, this, std::placeholders::_1));

        // 注册消息到达时的回调函数（负责接收 RPC 请求）
        server.setMessageCallback(
            std::bind(&RpcServer::OnMessage, this,
                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        // 设置工作线程数量（业务线程 + IO线程）
        server.setThreadNum(4);

        ZkClient zkCli; // 创建一个 ZkClient 实例，用于与 ZooKeeper 服务进行通信。
        zkCli.Start();  // 启动 ZkClient，建立与 ZooKeeper 的连接。

        for (auto &sp : _serviceMap) // 遍历本地服务发布表，每个 sp 表示一个服务的名称和其对应的服务描述信息（如方法列表）。
        {
            // 构造服务在 ZooKeeper 中的路径，例如 /UserServiceRpc
            std::string service_path = "/" + sp.first;

            zkCli.Create(service_path.c_str(), nullptr, 0);
            for (auto &mp : sp.second._methodMap) // 遍历该服务的所有方法
            {
                // 构造服务方法路径，例如：/UserServiceRpc/Login
                std::string method_path = service_path + "/" + mp.first;

                // 先确保 method_path 存在（持久节点）
                zkCli.Create(method_path.c_str(), nullptr, 0); // ZOO_PERSISTENT
                // Create 方法内部应当判断节点是否已存在，不重复创建。

                // 第三级目录，把 ip 和 port 拼成唯一节点名字
                char child_node[128] = {0};
                sprintf(child_node, "%s:%d", endpoint.host.c_str(), endpoint.port);

                // 构造最终的节点路径：/Service/Method/ip:port
                std::string endpoint_path = method_path + "/" + child_node;
                // 创建 EPHEMERAL 子节点
                // 代表当前服务器提供该方法，服务器宕机会自动消失
                zkCli.Create(endpoint_path.c_str(), nullptr, 0, ZOO_EPHEMERAL);
            }
        }

        std::cout << "RpcProvider start service at ip :" << endpoint.host << " port :" << endpoint.port << std::endl;

        // 启动网络服务，监听端口，接收请求
        server.start();

        // 启动事件循环（核心 IO 模块）
        _eventloop.loop();
    }

    // 新的 TCP 连接建立或断开时会触发该回调
    void RpcServer::OnConnection(const muduo::net::TcpConnectionPtr &conn)
    {
        // 这里可加逻辑处理连接断开（如 conn->shutdown()）
        if (!conn->connected())
        {
            conn->shutdown();
        }
    }

    /*
    在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
    service_name   method_name  args  服务名  方法名  参数
    | header_size(4字节) | header_str(service_name,method_name,args_size) | args_str |
    */
    // 接收到网络消息时触发（即 RPC 请求到达）回调RPC TODO: 这里应该进行 RPC 请求解析、参数反序列化、方法调用等
    void RpcServer::OnMessage(const muduo::net::TcpConnectionPtr &conn,
                                muduo::net::Buffer *buffer,
                                muduo::Timestamp timestamp)
    {
        // 读取远端传入的buffer
        std::string recv_buf = buffer->retrieveAllAsString();

        // 从数据流读取header_size(4字节)
        uint32_t header_size = 0;
        recv_buf.copy((char *)&header_size, 4, 0);

        // 根据header_size读取数据头的 header_str(service_name,method_name,args_size)  得到rpc请求的详细信息
        std::string rpc_header_str = recv_buf.substr(4, header_size);
        amoureux_proto::RpcHeader rpcheader;
        std::string service_name;
        std::string method_name;
        uint32_t args_size;
        if (rpcheader.ParseFromString(rpc_header_str))
        {
            // 数据头反序列化成功
            service_name = rpcheader.service_name();
            method_name = rpcheader.method_name();
            args_size = rpcheader.args_size();
        }
        else
        {
            // 数据头反序列化成功失败
            std::cout << "rpc_header_str" << rpc_header_str << "parse error!" << std::endl;
            return;
        }

        // 打印调试信息
        std::cout << "======================================" << std::endl;
        std::cout << "header_size: " << header_size << std::endl;
        std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
        std::cout << "service_name: " << service_name << std::endl;
        std::cout << "method_name: " << method_name << std::endl;
        std::cout << "======================================" << std::endl;

        // 从Map表中根据service_name和method_name获取获取service对象和method描述
        auto it = _serviceMap.find(service_name);
        if (it == _serviceMap.end())
        {
            std::cout << service_name << " is not exist" << std::endl;
            return;
        }
        auto mit = it->second._methodMap.find(method_name);
        if (mit == it->second._methodMap.end())
        {
            std::cout << service_name << " is not exist" << std::endl;
            return;
        }
        google::protobuf::Service *service = it->second._service;      // 获取service对象  new Userservice
        const google::protobuf::MethodDescriptor *method = mit->second; // 获取对象方法  Login

        // 生成rpc方法的调用的请求request参数和响应response参数
        std::string args_str = recv_buf.substr(4 + header_size, args_size);
        google::protobuf::Message *request = service->GetRequestPrototype(method).New();
        if (!request->ParseFromString(args_str))
        {
            std::cout << " request->ParseFromString() error, content" << args_str << std::endl;
            return;
        }
        google::protobuf::Message *response = service->GetResponsePrototype(method).New();

        // 给下面method方法的调用，绑定一个Closure的回调函数
        google::protobuf::Closure *done =
            google::protobuf::NewCallback<RpcServer, const muduo::net::TcpConnectionPtr &, google::protobuf::Message *>(
                this, &RpcServer::SendRpcResponse, conn, response);

        // 调用方法，执行请求
        service->CallMethod(method, nullptr, request, response, done);
    }

    // 向客户端发送 RPC 调用响应
    void RpcServer::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
    {
        std::string response_str;
        if (response->SerializeToString(&response_str))
        {
            conn->send(response_str);
        }
        else
        {
            std::cout << "Serialize response failed!" << std::endl;
        }
        conn->shutdown(); // 模拟短连接，发送完就关闭
    }

}