#pragma once

#include <chrono>
#include <mutex>

class TokenBucket {
public:
    /**
     * @param rate  每秒生成的令牌数（QPS）
     * @param burst 最大桶容量（允许的突发量）
     */
    TokenBucket(double rate, double burst)
        : rate_(rate),
          capacity_(burst),
          tokens_(burst),
          last_refill_(Clock::now()) {}

    /**
     * 尝试获取 n 个令牌
     * @return true 获取成功；false 被限流
     */
    bool tryAcquire(double n = 1.0) {
        std::lock_guard<std::mutex> lock(mutex_);
        refill();

        if (tokens_ >= n) {
            tokens_ -= n;
            return true;
        }
        return false;
    }

private:
    using Clock = std::chrono::steady_clock;

    void refill() {
        auto now = Clock::now();
        std::chrono::duration<double> elapsed = now - last_refill_;
        last_refill_ = now;

        tokens_ += elapsed.count() * rate_;
        if (tokens_ > capacity_) {
            tokens_ = capacity_;
        }
    }

private:
    double rate_;            // tokens per second
    double capacity_;        // bucket size
    double tokens_;          // current tokens
    Clock::time_point last_refill_;
    std::mutex mutex_;
};