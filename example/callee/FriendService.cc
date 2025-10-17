#include <iostream>
#include <string>
#include <vector>

#include "friend.pb.h"
#include "MprpcApplication.h"
#include "RpcProvider.h"
#include "Logger.h"

using namespace fixbug;
/* 
Userservivce原来是一个本地服务，提供了两个进程内的本地方法， login和getfriendlists
*/
class FriendService : public FriendServiceRPC //使用
{
private:
    /* data */
public:
    std::vector<std::string> GetFriendList(uint32_t userid)
    {
        std::cout << "doing local service: GetFriendList" << std::endl;
        std::vector<std::string> friendList;
        friendList.push_back("wang1");
        friendList.push_back("wang2");
        friendList.push_back("wang3");
        return friendList;
    }

    void GetFriendList(::google::protobuf::RpcController* controller,
        const ::fixbug::GetFriendListRequest* request,
        ::fixbug::GetFriendListResponse* response,
        ::google::protobuf::Closure* done)
        {
            uint32_t userid = request->userid();

            std::vector<std::string> friendList =  GetFriendList(userid);

                //把相应写入
            fixbug::ResultCode* code = response->mutable_result();
            code->set_errcode(0);
            code->set_errmsg("");

            for(std::string& friendname : friendList)
            {
                std::string* p = response->add_friends();
                *p = friendname;
            }

            //执行回调操作
            done->Run();
        }

};



int main(int argc, char** argv)
{
    LOG_INFO("first log message!");

    LOG_ERR("%s-%s-%d", __FILE__, __FUNCTION__, __LINE__);
    //调用框架的初始化操作  
    MprpcApplication::Init(argc, argv);
    //provider是一个rpc网络服务对象，把Uservice对象发布到rpc节点上
    RpcProvider provider;

    //将Userservice()类中的函数接口注册到service中
    provider.NotifyService(new FriendService());

    //启动一个rpc服务发布节点，，run以后，进程进入阻塞状态，等待远程rpc的调用请求
    provider.run();

    return 0;
}