#include "LoadBalancer.h"

#include <chrono>
#include <random>


namespace amoureux {

Endpoint RoundRobinLoadBalancer::Select(const std::string& service, const std::vector<Endpoint>& endpoints) {
    if (endpoints.empty()) {
        return {};
    }
    std::lock_guard<std::mutex> lock(mutex_);
    // 针对每个 service 维护独立偏移，实现轮询分发
    size_t& offset = offsets_[service];
    Endpoint ep = endpoints[offset % endpoints.size()];
    offset = (offset + 1) % endpoints.size();
    return ep;
}

Endpoint RandomLoadBalancer::Select(const std::string& /*service*/, const std::vector<Endpoint>& endpoints) {
    if (endpoints.empty()) {
        return {};
    }
    // 等概率随机挑选一个节点
    std::uniform_int_distribution<size_t> dist(0, endpoints.size() - 1);
    return endpoints[dist(rng_)];
}

} 