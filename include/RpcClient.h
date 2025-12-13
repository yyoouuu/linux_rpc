#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include <string>
#include "LoadBalancer.h"
namespace amoureux
{
    class MprpcChannel : public google::protobuf::RpcChannel
    {
    private:

    public:
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller,
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response,
                    google::protobuf::Closure* done) override;
    };
}
