#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

//异步写日志的日志队列
template<typename T>//C++ 模板只有在编译时实例化（生成具体代码），所以编译器在用模板时必须能“看到”模板的完整定义，而不仅仅是声明。
class LockQueue
{
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable;
public:
    void Push(const T& data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);//加锁出括号释放
        m_queue.push(data);
        m_condvariable.notify_one();
    }
    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_queue.empty())
        {
            //日志队列为空，线程进入wait状态
            m_condvariable.wait(lock);
        }

        T data = m_queue.front();
        m_queue.pop();
        return data;
    }

};