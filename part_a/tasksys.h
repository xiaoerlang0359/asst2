#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>



class DefaultIRunnable: public IRunnable{
    public:
        void runTask(int task_id, int num_total_tasks){
            return;
        }
};
/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial: public ITaskSystem {
    public:
        TaskSystemSerial(int num_threads);
        ~TaskSystemSerial();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn: public ITaskSystem {
    private:
        int num_threads;
        std::thread * workers;
        std::atomic<TaskID> curr_task;
    public:
        TaskSystemParallelSpawn(int num_threads);
        ~TaskSystemParallelSpawn();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        void runThread(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

class ThreadState{
    public:
        std::condition_variable* cond_ptr;
        std::mutex* mutex_ptr;
        int counter;
        bool * thread_first;
    ThreadState(int num_threads){
        cond_ptr = new std::condition_variable;
        mutex_ptr = new std::mutex();
        counter = 0;
        thread_first = new bool[num_threads];
    }
    ~ThreadState(){
        delete cond_ptr;
        delete mutex_ptr;
        delete thread_first;
    }
};
/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning: public ITaskSystem {
    private:
        ThreadState * start_state;
        ThreadState * finish_state; 
        int num_threads;
        bool running;
        std::thread * workers;
        IRunnable * runnable;
        int num_total_tasks;
        std::atomic<int> finish_counter;
        std::atomic<int> curr_task;
    public:
        TaskSystemParallelThreadPoolSpinning(int num_threads);
        ~TaskSystemParallelThreadPoolSpinning();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        void runThread(int thread_id);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSleeping: public ITaskSystem {
    private:
        ThreadState * start_state;
        ThreadState * finish_state; 
        int num_threads;
        bool running;
        std::thread * workers;
        IRunnable * runnable;
        int num_total_tasks;
        std::atomic<int> curr_task;
    public:
        TaskSystemParallelThreadPoolSleeping(int num_threads);
        ~TaskSystemParallelThreadPoolSleeping();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        void runThread(int thread_id);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

#endif
