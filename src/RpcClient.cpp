#include "RpcClient.h"
#include "rpcheader.pb.h"
#include "RpcApplication.h"
#include "RpcContoller.h"
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <error.h>


using namespace amoureux;
/* 
header_size + service_name method_name args_size + args_str
*/
//所有通过stub代理对象调用的rpc方法，都走到了这里，统一做rpc方法的数据序列化和网络发送
void RpcClient::CallMethod(const google::protobuf::MethodDescriptor* method,
    google::protobuf::RpcController* controller,
    const google::protobuf::Message* request,
    google::protobuf::Message* response,
    google::protobuf::Closure* done)
{
    //获取由调用者选择的服务和方法名
    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    //获取参数的序列化字符串长度args_size，将序列化后的args放入args_str中
    int args_size = 0;
    std::string args_str;
    if(request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        std::cout<<"Serialize request error!" << std::endl;
        controller->SetFailed("Serialize request error!");
        return;
    }

    //定义rpc的请求header
    amoureux_proto::RpcHeader rpcheader;
    rpcheader.set_service_name(service_name);
    rpcheader.set_method_name(method_name);
    rpcheader.set_args_size(args_size);

    //将请求header序列化后放入rpc_header_str中
    uint32_t header_size = 0;
    std::string rpc_header_str;
    if(rpcheader.SerializeToString(&rpc_header_str))
    {   
        header_size = rpc_header_str.size();
    }
    else
    {
        std::cout<<"Serialize rpc_header_str error!" << std::endl;
        controller->SetFailed("Serialize rpc_header_str error!");
        return;
    }

    //组织待发送的rpc请求字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size,4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    //打印调试信息
    std::cout << "======================================" << std::endl;
    std::cout << "header_size: " << header_size <<  std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str <<  std::endl;
    std::cout << "service_name: " << service_name <<  std::endl;
    std::cout << "method_name: " << method_name <<  std::endl;
    std::cout << "args_str: " << args_str <<  std::endl;
    std::cout << "======================================" << std::endl;

    //使用tpc编程，完成rpc方法的远程调用
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd ==-1)
    {
        std::cout << "create socket error! errno:" << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt,"create socket error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }
    
    // 从配置文件中读取服务端 IP 和端口
    Endpoint endpoint = amoureux::RpcApplication::GetInstance().GetConfig().client_rbselect("/UserService/UserLogin");

    struct sockaddr_in serveraddr;
    socklen_t serveraddr_len = sizeof(serveraddr);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port =  htons(endpoint.port);
    serveraddr.sin_addr.s_addr = inet_addr(endpoint.host.c_str());
    //连接rpc服务节点
    if(connect(clientfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) ==-1)
    {
        std::cout << "connect error! errno:" << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt,"connect error! errno: %d", errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }
    //发送rpc请求
    if (send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0) == -1)
    {
        std::cout << "send error! errno:" << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt,"send error! errno: %d", errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }
    //接收rpc请求的响应值
    int recv_size = 0;
    char recv_buf[1024] = {0};
    if((recv_size=recv(clientfd, recv_buf, 1024, 0)) == -1)
    {
        std::cout << "recv error! errno:" << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt,"recv error! errno: %d", errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }

    //反序列化接收rpc请求的响应值
    std::string response_str(recv_buf, recv_size);
    if (!response->ParseFromString(response_str))
    {
        std::cout << "response parse error! response_str:" << response_str << std::endl;
        char errtxt[512] = {0};
        snprintf(errtxt, sizeof(errtxt), "response parse error! response_str: %s", response_str.c_str());
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }
    close(clientfd);
}
