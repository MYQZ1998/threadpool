#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <deque>
#include <iostream>
#include <exception>

class task
{
private:
    // task function
    using void_func = std::function<void()>;
    void_func task_func;

public:
    // init task
    task(void_func func)
    {
        task_func = func;
    }
    void excute_task()
    {
        task_func();
    }
};

class threadpool
{
private:
    // thread num
    std::atomic<int> max_thread_num;
    const float init_wakeup_ration;
    std::atomic<int> idle_thread_num;
    std::atomic<int> current_thread_num;

    // main thread(control thread)
    std::thread main_thread;

    // task
    std::deque< std::shared_ptr<task> > task_list;
    std::atomic<int> task_num;

    // lock
    std::mutex threadpool_mutex;
    std::condition_variable threadpool_variable;

    // control flag
    std::atomic<bool> if_stop;
    std::atomic<bool> if_finish;

    // err num
    std::atomic<int> err_num;

public:
    threadpool(int max_thread_num, float init_wakeup_ration);
    ~threadpool();
    int get_max_thread_num();
    int get_task_num();
    int get_idle_thread_num();
    int get_current_thread_num();
    void change_max_thread_num(const int new_num);
    template<class F, class... Args>
    bool commit_task(F&& func, Args&&... args)
    {
        // judge if threadpool stop working
        if(if_stop)
        {
            err_num = 1;
            return false;
        }

        using ret_type = decltype(func(args...));
        this->threadpool_mutex.lock();
        std::shared_ptr< std::packaged_task<ret_type()> > pfunc = std::make_shared< std::packaged_task<ret_type()> >(std::bind(std::forward<F>(func), std::forward<Args>(args)...));
        
        std::function<void()> task_func = ([pfunc](){(*pfunc)();});
        std::shared_ptr<task> ptask = std::make_shared<task>(task(task_func));
        this->task_list.push_back(ptask);
        this->task_num++;
        this->threadpool_mutex.unlock();
        
        return true;
    }
    void stop_threadpool();
    void finish();

    void print_err();
};

#endif