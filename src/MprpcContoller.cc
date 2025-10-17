#include "MprpcContoller.h"



MprpcContoller::MprpcContoller()
{
    m_failed = false;
    m_errText = "";
}
void MprpcContoller::Reset()
{
    m_failed = false;
    m_errText = "";
}
bool MprpcContoller::Failed() const
{
    return m_failed;
}
std::string MprpcContoller::ErrorText() const
{
    return m_errText;
}
void MprpcContoller::SetFailed(const std::string& reason)
{
    m_failed = true;
    m_errText = reason;
}


void MprpcContoller::StartCancel(){}

bool MprpcContoller::IsCanceled() const{ return false; }

void MprpcContoller::NotifyOnCancel(google::protobuf::Closure* callback){}