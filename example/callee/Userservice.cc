#include <iostream>
#include <string>

#include "user.pb.h"
#include "MprpcApplication.h"
#include "RpcProvider.h"

using namespace fixbug;

/* 
Userservivce原来是一个本地服务，提供了两个进程内的本地方法， login和getfriendlists
*/
class Userservice : public UserServiceRPC  //使用
{
private:
    /* data */
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name:" << name << "pwd:" << pwd << std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service: Register" << std::endl;
        std::cout << "id:" << id << "name:" << name << "pwd:" << pwd << std::endl;
        return true;
    }
   /*
   重写父类UserServiceRPC的虚函数，下面这些方法是框架直接调用的
   1. caller  --->   login()    =>  muduo  =>   callee
   2. callee  --->   login()    =>  交到下面重写的login方法上了
   */
    void Login(::google::protobuf::RpcController* controller,
               const ::fixbug::LoginRequest* request,
               ::fixbug::LoginResponse* response,
               ::google::protobuf::Closure* done)
    {
        //框架业务上报了请求参数LoginRequest，应用获取数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        //做本地业务
        bool login_result = Login(name, pwd);

        //把相应写入
        fixbug::ResultCode* code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        //执行回调操作
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                  const ::fixbug::RegisterRequest* request,
                  ::fixbug::RegisterResponse* response,
                  ::google::protobuf::Closure* done)
    {
        //框架业务上报了请求参数LoginRequest，应用获取数据做本地业务
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool register_result = Register(id, name, pwd);

        //把相应写入
        fixbug::ResultCode* code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(register_result);

        //执行回调操作
        done->Run();
    }
};

int main(int argc, char** argv)
{
    //调用框架的初始化操作  
    MprpcApplication::Init(argc, argv);
    //provider是一个rpc网络服务对象，把Uservice对象发布到rpc节点上
    RpcProvider provider;

    //将Userservice()类中的函数接口注册到service中
    provider.NotifyService(new Userservice());

    //启动一个rpc服务发布节点，，run以后，进程进入阻塞状态，等待远程rpc的调用请求
    provider.run();

    return 0;
}