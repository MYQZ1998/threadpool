#include "threadpool.h"
using namespace tp;

threadpool::threadpool(int max_thread_num, float init_wakeup_ration):max_thread_num(max_thread_num), init_wakeup_ration(init_wakeup_ration), idle_thread_num(0), current_thread_num(0)
    , task_num(0), if_stop(false), err_num(0), milliseconds(50){    
    this->main_thread = std::thread([this]()
    {
        int wakeup_thread_num = this->max_thread_num * this->init_wakeup_ration;
        for(int i = 0; i < wakeup_thread_num; i++)
        {
            this->current_thread_num++;
            std::thread work_thread([this]()
                {
                    while(true)
                    {
                        this->threadpool_mutex.lock();

                        std::shared_ptr<task> ptask;
                        if(task_num > 0)
                        {
                            ptask = this->task_list.front();
                            this->task_list.pop_front();
                            this->task_num--;
                        }
                        this->threadpool_mutex.unlock();

                        if(ptask != nullptr && !this->if_stop)
                        {
                            ptask->excute_task();
                        }

                        std::cv_status ret;
                        if(!this->if_stop && this->task_num == 0)
                        {
                            std::unique_lock<std::mutex> ul(this->threadpool_mutex);
                            this->idle_thread_num++;
                            ret = this->threadpool_variable.wait_for(ul, std::chrono::seconds(60));
                            this->idle_thread_num--;
                        }
                        if(ret == std::cv_status::timeout || this->if_stop)
                        {
                            this->threadpool_mutex.lock();
                            this->current_thread_num--;
                            this->threadpool_mutex.unlock();
                            return;
                        }
                    }
                });
            work_thread.detach();
        }

        while(true)
        {
            std::unique_lock<std::mutex> ul(this->threadpool_mutex);
            this->threadpool_variable.wait_for(ul, std::chrono::milliseconds(this->milliseconds));

            if(this->if_stop)
            {
                return;
            }

            if(this->task_num > 0)
            {
                if(this->idle_thread_num > 0)
                {
                    this->threadpool_variable.notify_one();
                }
                else if(this->current_thread_num < this->max_thread_num)
                {   
                    this->current_thread_num++;
                    std::thread work_thread([this]()
                        {
                            while(true)
                            {
                                this->threadpool_mutex.lock();

                                std::shared_ptr<task> ptask;
                                if(task_num > 0)
                                {
                                    ptask = this->task_list.front();
                                    this->task_list.pop_front();
                                    this->task_num--;
                                }
                                this->threadpool_mutex.unlock();

                                if(ptask != nullptr)
                                {
                                    ptask->excute_task();
                                }

                                std::cv_status ret;
                                if(!this->if_stop)
                                {
                                    std::unique_lock<std::mutex> ul(this->threadpool_mutex);
                                    this->idle_thread_num++;
                                    ret = this->threadpool_variable.wait_for(ul, std::chrono::seconds(60));
                                    this->idle_thread_num--;
                                }

                                if(ret == std::cv_status::timeout || this->if_stop)
                                {
                                    this->threadpool_mutex.lock();
                                    this->current_thread_num--;
                                    this->threadpool_mutex.unlock();
                                    return;
                                }
                            }
                        });
                    work_thread.detach();
                }
            }
        }
    });
}

threadpool::~threadpool()
{
    this->finish();
    this->main_thread.join();
}

int threadpool::get_max_thread_num()
{
    return this->max_thread_num;
}

int threadpool::get_task_num()
{
    return this->task_num;
}

int threadpool::get_idle_thread_num()
{
    return this->idle_thread_num;
}

int threadpool::get_current_thread_num()
{
    return this->current_thread_num;
}

void threadpool::change_max_thread_num(const int new_num)
{
    this->max_thread_num = new_num;
    return;
}

void threadpool::stop_threadpool()
{
    this->if_stop = true;
}

void threadpool::finish()
{
    this->if_stop = true;

    while(this->current_thread_num != 0)
    {   
        std::this_thread::sleep_for(std::chrono::seconds(1));
        this->threadpool_variable.notify_all();
    }
}

int threadpool::get_control_thread_sleeptime()
{
    return this->milliseconds;
}

void threadpool::set_control_thread_sleeptime(int new_sleeptime)
{
    this->milliseconds = new_sleeptime;
}

void threadpool::print_err()
{
    switch(this->err_num)
    {
        case 0:
            std::cout << "normal" << std::endl;
            break;
        case 1:
            std::cout << "threadpool is stopped" << std::endl;
            break;
    }
}