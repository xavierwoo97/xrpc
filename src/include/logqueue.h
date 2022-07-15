#pragma once
#include <queue>
#include<thread>
#include<mutex>
#include<condition_variable>


//异步写日志的队列
template<typename T>
class LogQue
{
public:
    //多个worker线程都会写日志queue
    void Push(const T&data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_condvariable.notify_one(); //只有一个线程在写
    }

    //一个线程读logqueue,写日志文件
    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // 防止线程虚假唤醒
        while(m_queue.empty())
        {
            //日志队列为空，线程进入wait状态
            m_condvariable.wait(lock);
        }

        T data = m_queue.front();
        m_queue.pop();
        return data;
    }
private:
    std::queue<T> m_queue;  // 缓冲区
    std::mutex m_mutex;  // 互斥锁
    std::condition_variable m_condvariable;  // 信号量
};

