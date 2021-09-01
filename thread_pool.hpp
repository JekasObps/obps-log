#pragma once

#include <thread>
#include <future>
#include <functional>
#include <vector>

#include <mutex>
#include "spinlock.hpp"

namespace obps
{

template <
    typename ThreadRet, // user defined status enum that pool will use to indicate a state of thread execution  
    typename E,         // Exception Base that will be catched
    ThreadRet completed,// !value template parameter. Status that stops completed task thread.
    ThreadRet critical  // status that indicates criticalness of the error in user task (can the task continue after catching this exception?)
> class ThreadPool final 
{
public:
    enum class State_ {RUNNING, SHUTDOWN};

    using OnExcept = std::function<ThreadRet( const E& )>; 

    template <typename ...Args> 
    void RunTask(
        OnExcept on_except,                             // checks if exception is critical
        std::function<ThreadRet(Args...)> thread_func,  // user task
        Args ...args                                    // user arguments to pass into the task
    ) noexcept;

    // stop all tasks and wait for them
    void ShutDown() noexcept;


    static std::shared_ptr<ThreadPool> GetInstance() 
    {
        static bool isInit = false;
        if (!isInit){
            s_Instance = std::unique_ptr<ThreadPool>(new ThreadPool());
            isInit = true;
        }
        return s_Instance;
    }

    ~ThreadPool() {
        ShutDown();
    }
private:
    ThreadPool() : m_State{State_::RUNNING} 
    {}
    
    static std::shared_ptr<ThreadPool> s_Instance;
    
    State_ m_State; 
    std::vector<std::future<ThreadRet>> m_Tasks;
    spinlock m_PoolLock;    // prevents racecondition when running tasks and on shutdown
};

template <typename ThreadRet, typename E, ThreadRet completed, ThreadRet critical>
template <typename ...Args> 
void ThreadPool<ThreadRet, E, completed, critical>::RunTask( 
    OnExcept on_except, std::function<ThreadRet(Args...)> thread_func, Args ...args) noexcept 
{
    std::lock_guard<spinlock> _lock(m_PoolLock);
    m_Tasks.emplace_back(
        std::async([this, on_except, thread_func, args...] {
            while(m_State == State_::RUNNING)
            {
            try
            {
                ThreadRet status = thread_func(args...);
                if (status == completed || status == critical)
                {
                    return status;
                }
            }
            catch(const E& e)
            {
                if (on_except(e) == critical)
                {
                    return critical;
                }
            }
            }
            return critical;
        })
    );
}


template <typename ThreadRet, typename E, ThreadRet completed, ThreadRet critical>
void ThreadPool<ThreadRet, E, completed, critical>::ShutDown() noexcept
{
    std::lock_guard<spinlock> _lock(m_PoolLock);
    // state propagates to all threads
    m_State = State_::SHUTDOWN;

    while((! m_Tasks.empty()))
    {
        auto && task = m_Tasks.back();
        ThreadRet result = task.get();
        m_Tasks.pop_back();
        // do something with result
        // ?? maybe by passing to a user hook ?? 
    }
}


// ! instantiation of a ThreadPool singleton !
template <typename ThreadRet, typename E, ThreadRet completed, ThreadRet critical>
std::shared_ptr<ThreadPool<ThreadRet, E, completed, critical>> ThreadPool<ThreadRet, E, completed, critical>::s_Instance;

} // namespace obps