#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

template <typename T>
class LockQueue
{
private:
    std::queue<T> _lockqueue;
    std::mutex _mutex;
    std::condition_variable _cond;
    bool _stop;

public:
    LockQueue() : _stop(false) {}

    // 禁止拷贝
    LockQueue(const LockQueue &) = delete;
    LockQueue &operator=(const LockQueue &) = delete;

    // 允许移动
    LockQueue(LockQueue &&) = default;
    LockQueue &operator=(LockQueue &&) = default;

    // 业务线程：添加日志任务
    void push(const T &data)
    {
        {
            std::lock_guard<std::mutex> lk(_mutex);
            _lockqueue.push(data);
        }
        _cond.notify_one();
    }

    // 支持移动 push，减少拷贝
    void push(T &&data)
    {
        {
            std::lock_guard<std::mutex> lk(_mutex);
            _lockqueue.push(std::move(data));
        }
        _cond.notify_one();
    }

    // 工作线程：取日志任务
    // 若 stop() 被调用，则返回 false 终止线程循环
    bool pop(T &out)
    {
        std::unique_lock<std::mutex> lk(_mutex);
        _cond.wait(lk, [this]
                   { return !_lockqueue.empty() || _stop; });

        if (_stop && _lockqueue.empty())
            return false;

        out = std::move(_lockqueue.front());
        _lockqueue.pop();
        return true;
    }

    // -------------------------------
    // 停止队列（如程序退出）
    // 所有等待的线程都会被唤醒并退出
    // -------------------------------
    void stop()
    {
        {
            std::lock_guard<std::mutex> lk(_mutex);
            _stop = true;
        }
        _cond.notify_all();
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lk(_mutex);
        return _lockqueue.empty();
    }
};
