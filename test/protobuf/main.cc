#include <iostream> 
#include "test.pb.h" // 包含 Protocol Buffers 生成的头文件
#include <string> 

using namespace fixbug;  // 使用 fixbug 命名空间


int main(){ 
    // 创建一个 LoginRequest 对象 req，用于表示登录请求
    LoginRequest req; 

    // 设置 LoginRequest 对象中的字段值
    req.set_name("张珊");  // 设置用户名
    req.set_pwd("123456");  // 设置密码

    // 定义一个字符串变量 send_str，用来保存序列化后的数据
    std::string send_str;

    // 将 LoginRequest 对象 req 序列化为字符串并存储到 send_str 中
    if (req.SerializeToString(&send_str)) {
        std::cout << send_str << std::endl;  // 输出序列化后的字符串
    }

    // 创建一个新的 LoginRequest 对象 reqB，用于反序列化
    LoginRequest reqB;

    // 使用 ParseFromString 函数将 send_str 反序列化为 LoginRequest 对象 reqB
    if (reqB.ParseFromString(send_str)) {
        std::cout << reqB.name() << std::endl;  // 输出反序列化后的 name 字段
        std::cout << reqB.pwd() << std::endl;   // 输出反序列化后的 pwd 字段
    }


    LoginResponse rsp;
    ResultCode* rc = rsp.mutable_result();
    rc->set_errcode(1);
    rc->set_errmsg("登陆处理失败了");

    GetFriendListsResponse rsp1;
    ResultCode* rc1 = rsp1.mutable_result();
    rc->set_errcode(0);

    User* user1 = rsp1.add_friend_list();
    user1->set_name("zhangsan");
    user1->set_age(18);
    user1->set_sex(User::Man);

    User* user2 = rsp1.add_friend_list();
    user2->set_name("lisi");
    user2->set_age(18);
    user2->set_sex(User::Women);

    std::cout<< rsp1.friend_list_size() << std::endl;


    return 0;  // 程序正常结束
}