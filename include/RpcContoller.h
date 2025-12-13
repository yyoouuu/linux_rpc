#pragma once

#include <google/protobuf/service.h>
#include <string>
 
class MprpcContoller : public google::protobuf::RpcController
{
private:
    bool _failed;//RPC方法执行过程中的状态
    std::string _errText;//RPC方法执行过程中的错误信息
public:
    MprpcContoller();
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string& reason);


    void StartCancel();

    bool IsCanceled() const;

    void NotifyOnCancel(google::protobuf::Closure* callback);
    
};