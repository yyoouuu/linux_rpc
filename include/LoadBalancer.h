#pragma once

#include <cstddef>
#include <mutex>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "ZKClient.h"

namespace amoureux {

class LoadBalancer {
public:
    virtual ~LoadBalancer() = default;
    // 选择一个可用的服务节点，若不可用返回空 Endpoint
    virtual Endpoint Select(const std::string& service, const std::vector<Endpoint>& endpoints) = 0;
};

class RoundRobinLoadBalancer final : public LoadBalancer {
public:
    // 按服务维度轮询选择节点
    Endpoint Select(const std::string& service, const std::vector<Endpoint>& endpoints) override;

private:
    std::mutex mutex_;                                  // 保护偏移量的锁
    std::unordered_map<std::string, size_t> offsets_;   // 各服务的当前轮询偏移
};

class RandomLoadBalancer final : public LoadBalancer {
public:
    // 随机选择一个节点
    Endpoint Select(const std::string& service, const std::vector<Endpoint>& endpoints) override;

private:
    std::mt19937 rng_{std::random_device{}()};          // 随机数引擎
};

}  // namespace sparkpush::rpc
