#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <vector>
#include <map>
#include <queue>
#include <list>


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

class TaskInfo{
    public:
        std::vector<TaskID> stimulate;
        IRunnable * runnable;
        int num_total_tasks;
        int dep_cnt;
        int comp_cnt;
        TaskID id;
        TaskInfo(IRunnable* runnable, int num_total_tasks, TaskID task_id){
            this->runnable = runnable;
            this->num_total_tasks = num_total_tasks;
            this->dep_cnt = 0;
            this->comp_cnt = 0;
            this->id = task_id;
        }
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
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn: public ITaskSystem {
    public:
        TaskSystemParallelSpawn(int num_threads);
        ~TaskSystemParallelSpawn();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSpinning(int num_threads);
        ~TaskSystemParallelThreadPoolSpinning();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
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
        std::mutex queue_mutex;
        bool * worker_state;
        std::condition_variable wait_job;
        std::condition_variable wait_done;
        int num_threads;
        std::queue<TaskInfo*> *ready_que_thrd;
        std::list<TaskInfo> ready_que;
        std::map<TaskID,TaskInfo> wait_que; 
        TaskID  task_id;
        bool running;
        std::thread * workers;
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
