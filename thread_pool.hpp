#pragma once

#include <thread>
#include <future>
#include <functional>
#include <vector>

template <typename ThreadRet>
class ThreadPool final 
{
public:
    enum class State_ {RUNNING, SHUTDOWN};

    template<typename ...Args> 
    void RunTask(std::function<ThreadRet(State_&, Args...)> thread_func, State_&, Args ...args) noexcept
    {
        // m_Threads.emplace_back(thread_func, std::forward<Args>(args));
        m_Tasks.emplace_back(
            std::async(std::forward<ThreadRet(State_&, Args...)>(thread_func), 
            std::forward<Args>(args)...)
        );
    }

    void ShutDown() noexcept
    {
        // state propagates to all threads
        m_State = State_::SHUTDOWN;

        for (auto&& task : m_Tasks)
        {
            ThreadRet result = task.get();
            // do something with result
            // ?? maybe by passing user hook ?? 
        }
    }

    static ThreadPool& GetInstance() 
    {
        static bool isInit = false;
        if (!isInit){
            s_Instance = std::unique_ptr<ThreadPool>(new ThreadPool());
            isInit = true;
        }
        return *s_Instance;
    }

private:
    ThreadPool() {}
    
    ~ThreadPool() {}

    State_ m_State;
    // std::vector<std::thread> m_Threads;

    std::vector<std::future<ThreadRet>> m_Tasks;
    

    static std::unique_ptr<ThreadPool> s_Instance; // c++ 11 provides memory barriers for 
};




