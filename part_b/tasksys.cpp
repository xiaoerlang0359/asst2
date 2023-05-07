#include "tasksys.h"


IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemSerial::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */
const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    this->num_threads = num_threads -1;
    running = true;
    task_id = 0;
    curr_task = -1;
    workers = new std::thread[num_threads-1];
    ready_que_thrd = new std::queue<TaskInfo*>[num_threads-1];
    worker_state = new bool[num_threads-1];
    for (int i=0;i<num_threads-1;i++){
        worker_state[i] = false;
    }
    for (int i=0;i<num_threads-1;i++){
        workers[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::runThread, this, i);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    queue_mutex.lock();
    running = false;
    bool all_worker_run = false;;
    while (!all_worker_run){
        all_worker_run = true;
        for (int i=0; i<num_threads; i++)
            all_worker_run &= worker_state[i];
        queue_mutex.unlock();
        wait_job.notify_all();
        queue_mutex.lock();    
    }
    queue_mutex.unlock();
    for (int i=0;i<num_threads;i++){
        workers[i].join();
    }
    delete[] workers;
    delete[] ready_que_thrd;
    delete[] worker_state;
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    std::vector<TaskID> empty_vec;
    runAsyncWithDeps(runnable,num_total_tasks,empty_vec);
    sync();
}

void TaskSystemParallelThreadPoolSleeping::runThread(int thread_id){
    while(true){
        TaskInfo * task_ptr;
        std::unique_lock<std::mutex> lk(queue_mutex);
        while(ready_que_thrd[thread_id].empty()){
            worker_state[thread_id] = false;
            if (!running){
                worker_state[thread_id] = true;
                lk.unlock();
                return;
            }
            wait_job.wait(lk);
        }
        while(!ready_que_thrd[thread_id].empty()){
            worker_state[thread_id] = true;
            if (!running){
                lk.unlock();
                return;
            }
            task_ptr = ready_que_thrd[thread_id].front();
            lk.unlock();
            int num_tasks = task_ptr->num_total_tasks;
            IRunnable * local_runnable = task_ptr->runnable;
            for (int i=thread_id; i<num_tasks;i+=num_threads){
                local_runnable->runTask(i,num_tasks);
            }
            ready_que_thrd[thread_id].pop();
            lk.lock();
            task_ptr->comp_cnt++;
            //printf("thread %d finish task %d, with comp_cnt %d\n",thread_id,ready_que.front().id,task_ptr->comp_cnt);
            if (task_ptr->comp_cnt == num_threads){
                //printf("finish task: %d, with ready_que size: %ld\n", ready_que.front().id,ready_que.size());
                for (auto & elem: task_ptr->stimulate){
                    auto wait_task = wait_que.find(elem);
                    if (wait_task != wait_que.end()){
                        wait_task->second.dep_cnt--;
                        if (wait_task->second.dep_cnt==0){
                            ready_que.push_back(wait_task->second);
                            for (int i=0;i<num_threads;i++) ready_que_thrd[i].push(&ready_que.back());
                            //printf("wait_que -> ready_que with task id: %d\n",wait_task->first);
                            wait_que.erase(wait_task);
                        }
                    }
                }
                ready_que.pop_front();
            }
        }
        lk.unlock();
        wait_done.notify_all();
    }
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //
    task_id++;
    TaskInfo new_task = TaskInfo(runnable,num_total_tasks,task_id);
    queue_mutex.lock();
    for (auto& elem: deps){
        for (auto& ready_task: ready_que){
            if (ready_task.id==elem){
                new_task.dep_cnt++;
                ready_task.stimulate.push_back(task_id);
            }
        }
        auto wait_task = wait_que.find(elem);
        if (wait_task != wait_que.end()){
            new_task.dep_cnt++;
            wait_task->second.stimulate.push_back(task_id);
        }
    }
    //printf("insert new task(%d), with dep_cnt(%d), with ready_que size(%ld)\n", task_id,new_task.dep_cnt,ready_que.size());
    if (new_task.dep_cnt>0){
        wait_que.insert({task_id,new_task});
    }else{
        ready_que.push_back(new_task);   
        for (int i=0;i<num_threads;i++) ready_que_thrd[i].push(&ready_que.back());
    }
    //volatile bool all_worker_run = false;;
    //while (!all_worker_run){
    //    all_worker_run = true;
    //    for (int i=0; i<num_threads; i++)
    //        all_worker_run &= worker_state[i];
    queue_mutex.unlock();
    wait_job.notify_all();
    //queue_mutex.lock();    
    //} 
    //queue_mutex.unlock();
    return task_id;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //
    wait_job.notify_all();
    std::unique_lock<std::mutex> lk(queue_mutex);
    //printf("sync threads with ready_que size: %ld\n", ready_que.size());
    while (!ready_que.empty()){
        lk.unlock();
        wait_job.notify_all();
        lk.lock();
        //wait_done.wait(lk);
    }
    lk.unlock();
    return;
}
