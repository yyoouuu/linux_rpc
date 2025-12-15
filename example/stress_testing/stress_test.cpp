#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>

#include "RpcApplication.h"
#include "RpcClient.h"
#include "user.pb.h"

using namespace std::chrono;

static std::atomic<uint64_t> total_requests{0};
static std::atomic<uint64_t> success_requests{0};

std::vector<int64_t> latencies; // 微秒
std::mutex latency_mutex;

void rpc_worker(int thread_id, int request_count)
{
    amoureux_proto::UserService_Stub stub(new amoureux::RpcClient());

    for (int i = 0; i < request_count; ++i)
    {
        amoureux_proto::UserRegisterReq request;
        request.set_request_id("123456");
        request.set_nickname("amoureux");
        request.set_password("123456");

        amoureux_proto::UserRegisterRsp response;

        auto start = high_resolution_clock::now();

        stub.UserRegister(nullptr, &request, &response, nullptr);

        auto end = high_resolution_clock::now();
        int64_t cost = duration_cast<microseconds>(end - start).count();

        total_requests++;
        if (response.result().errcode() == 0)
        {
            success_requests++;
        }

        {
            std::lock_guard<std::mutex> lock(latency_mutex);
            latencies.push_back(cost);
        }
    }
}

int main(int argc, char **argv)
{
    // 初始化 RPC 框架
    amoureux::RpcApplication::Init(argc, argv);

    int thread_num = 50;        // 并发线程数
    int req_per_thread = 1000; // 每线程请求数

    std::vector<std::thread> threads;

    auto test_start = steady_clock::now();

    for (int i = 0; i < thread_num; ++i)
    {
        threads.emplace_back(rpc_worker, i, req_per_thread);
    }

    for (auto &t : threads)
    {
        t.join();
    }

    auto test_end = steady_clock::now();
    double seconds = duration_cast<duration<double>>(test_end - test_start).count();

    // 统计
    uint64_t total = total_requests.load();
    uint64_t success = success_requests.load();
    double qps = total / seconds;

    std::sort(latencies.begin(), latencies.end());

    auto avg_latency = std::accumulate(latencies.begin(), latencies.end(), 0LL) / latencies.size();
    auto p95 = latencies[latencies.size() * 95 / 100];
    auto p99 = latencies[latencies.size() * 99 / 100];

    std::cout << "========== RPC Benchmark ==========\n";
    std::cout << "Threads        : " << thread_num << "\n";
    std::cout << "Total Requests : " << total << "\n";
    std::cout << "Success        : " << success << "\n";
    std::cout << "Duration(s)    : " << seconds << "\n";
    std::cout << "QPS            : " << qps << "\n";
    std::cout << "Avg Latency(us): " << avg_latency << "\n";
    std::cout << "P95 Latency(us): " << p95 << "\n";
    std::cout << "P99 Latency(us): " << p99 << "\n";
}