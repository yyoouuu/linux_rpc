#include "RpcContoller.h"


MprpcContoller::MprpcContoller()
{
    _failed = false;
    _errText = "";
}
void MprpcContoller::Reset()
{
    _failed = false;
    _errText = "";
}
bool MprpcContoller::Failed() const
{
    return _failed;
}
std::string MprpcContoller::ErrorText() const
{
    return _errText;
}
void MprpcContoller::SetFailed(const std::string& reason)
{
    _failed = true;
    _errText = reason;
}


void MprpcContoller::StartCancel(){}

bool MprpcContoller::IsCanceled() const{ return false; }

void MprpcContoller::NotifyOnCancel(google::protobuf::Closure* callback){}