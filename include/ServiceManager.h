#pragma once
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>

namespace amoureux
{
    struct Endpoint
    {
        std::string host; // 主机名或 IP
        uint16_t port{0}; // 端口
    };
    class ServiceManager
    {
    private:
        mutable std::mutex mutex_;                                        // 保护 services_ 的锁
        std::unordered_map<std::string, std::vector<Endpoint>> services_; // 服务 -> 节点列表
    public:
        // 注册服务节点（若已存在则忽略）
        void Register(const std::string& service,const std::string& method, const Endpoint &endpoint);
        // 取消注册指定节点
        void Unregister(const std::string& service,const std::string& method, const Endpoint &endpoint);
        // 列出某个服务的所有节点
        std::vector<Endpoint> List(const std::string &service) const;
    };
} // namespace amoureux
