# threadpool  
基于C++11标准实现的线程池，可以动态的改变最大线程数量。  
***  
## 类接口函数  
**threadpool(int max_thread_num, float init_wakeup_ration)**  
构造函数，第一个参数是最大线程数量，第二个参数是一个比例，决定类初始化时创建的线程数量。 
没有做范围判定，请保证：  
0 < max_thread_num  
0 <= init_wakeup_ration <= 1  
  
**int get_max_thread_num()**
获得当前设定的最大线程数量。  
  
**int get_task_num()**  
获得现在还未被执行的任务数量。  
  
**int get_idle_thread_num()** 
获得现在空闲（阻塞状态）的线程数量。  
  
**int get_current_thread_num()**  
获得现在已创建的线程数量。  
  
**void change_max_thread_num(const int new_num)**  
重新设定最大线程数量，请保证：
0 < new_num  
  
**template<class F, class... Args>**  
**bool commit_task(F&& func, Args&&... agrs)**  
提交任务，第一个参数为函数，其余参数为函数执行所需参数。  
成功返回true，失败返回false。  

**void stop_threadpool()**
线程池停止工作（剩余的任务不在会被执行，正在被执行的任务继续执行）。  

**void finish()**  
线程池停止工作并阻塞等待正在被执行的任务执行完毕。  

**void print_err()**  
打印最近一次的失败信息。  
目前并没有什么用处。  
***  
## 线程停留时间
被创建的线程最多阻塞60秒，60秒内没有需要执行的任务的话，该线程会被销毁。  
