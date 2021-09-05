#pragma once

#include <vector>
#include <thread>
#include <future>
#include <functional>

#include <mutex>
#include "spinlock.hpp"

namespace obps
{

template <
    typename ThreadRet, // user defined status enum that pool will use to indicate a state of thread execution
    ThreadRet running,  // continue looping on thread_func
    ThreadRet finished, // stop looping, task completed
    ThreadRet aborted   // status that indicates errorneus execution stop
> class ThreadPool final 
{
public:
    enum class PoolState {RUNNING, SHUTDOWN};

    template <typename ...Args> 
    void RunTask(
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
    ThreadPool() : m_State{PoolState::RUNNING} 
    {}
    
    static std::shared_ptr<ThreadPool> s_Instance;
    
    PoolState m_State;

    std::vector<std::thread> m_Threads;
    std::vector<std::pair<std::promise<ThreadRet>, std::future<ThreadRet>>> m_Tasks;
    spinlock m_PoolLock;    // prevents race-condition when running tasks and on shutdown
};

template <typename ThreadRet, ThreadRet running, ThreadRet finished, ThreadRet aborted>
template <typename ...Args> 
void ThreadPool<ThreadRet, running, finished, aborted>::RunTask(
    std::function<ThreadRet(Args...)> thread_func, Args ...args) noexcept 
{
    std::lock_guard<spinlock> lock(m_PoolLock);
    
    auto& [promise, future] = m_Tasks.emplace_back();
    future = promise.get_future();

    m_Threads.emplace_back([this, thread_func, &promise, args...] {
        while(m_State == PoolState::RUNNING)
        {
            try 
            {
                ThreadRet status = thread_func(args...);
                switch (status)
                {
                    case running: 
                        continue; break;

                    case finished:
                    case aborted:
                    {
                        promise.set_value_at_thread_exit(status);
                        return;
                    }

                    default: 
                        continue;
                }
            }
            catch(const std::exception& e)
            {
                try 
                {
                    promise.set_exception(std::current_exception());
                } 
                catch(const std::exception& e) {
                    std::cerr << "Exception was thrown on promise::set_exception:\n"
                              << e.what() << std::endl;
                }

                return;
            }
        }
    });
}


template <typename ThreadRet, ThreadRet running, ThreadRet completed, ThreadRet aborted>
void ThreadPool<ThreadRet, running, completed, aborted>::ShutDown() noexcept
{
    std::lock_guard<spinlock> lock(m_PoolLock);
    // state propagates to all threads
    m_State = PoolState::SHUTDOWN;
    
    while((!m_Threads.empty()))
    {
        m_Threads.back().join();
        ThreadRet result = m_Tasks.back().second.get();
        m_Threads.pop_back();
        m_Tasks.pop_back();

        // TODO: process result
    }
}


// ! instantiation of a ThreadPool singleton !
template <typename ThreadRet, ThreadRet running, ThreadRet completed, ThreadRet aborted>
inline std::shared_ptr<ThreadPool<ThreadRet, running, completed, aborted>> ThreadPool<ThreadRet, running, completed, aborted>::s_Instance;

} // namespace obps