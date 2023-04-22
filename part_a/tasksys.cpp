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
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    this->num_threads = num_threads;
    workers = new std::thread[num_threads];
    curr_task = 0;
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {
    delete[] workers;
}

void TaskSystemParallelSpawn::runThread(IRunnable* runnable, int num_total_tasks) {
   while(curr_task<num_total_tasks){
    int local_task_id = ++curr_task;
    if (local_task_id >= num_total_tasks) break;
    runnable->runTask(local_task_id,num_total_tasks);
   } 
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    curr_task = -1;
    for (int i = 0; i < num_threads-1; i++) {
        workers[i] = std::thread(&TaskSystemParallelSpawn::runThread, this, runnable, num_total_tasks); 
    }
    this->runThread(runnable, num_total_tasks);
    for (int i=0; i< num_threads-1; i++){
        workers[i].join();
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    return 0;
}

void TaskSystemParallelSpawn::sync() {
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
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    this->num_threads = num_threads;
    running = true;
    start_state = new ThreadState(num_threads);
    finish_state = new ThreadState(num_threads);
    runnable = new DefaultIRunnable();
    workers = new std::thread[num_threads];
    for (int i=0;i<num_threads;i++){
        workers[i] = std::thread(&TaskSystemParallelThreadPoolSpinning::runThread, this, i);
    }
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    running = false;
    start_state->mutex_ptr->lock();
    for (int i=0;i<num_threads;i++){
        start_state->thread_first[i] = true;
    }
    start_state->counter = 0;
    while(start_state->counter<num_threads){
        start_state->mutex_ptr->unlock();
        start_state->cond_ptr->notify_all();
        start_state->mutex_ptr->lock();
    }
    start_state->mutex_ptr->unlock();
    for (int i=0;i<num_threads;i++){
        workers[i].join();
    }
    delete start_state;
    delete finish_state;
    delete[] workers;
}

void TaskSystemParallelThreadPoolSpinning::runThread(int thread_id){
    while(true){ 
        std::unique_lock<std::mutex> lk(*start_state->mutex_ptr);
        start_state->cond_ptr->wait(lk);
        if (start_state->thread_first[thread_id]){
            start_state->thread_first[thread_id] = false;
            start_state->counter++;
            lk.unlock();
            //printf("thread %d start run\n",thread_id);
            if (!running) return;
            //for (int i=thread_id;i<num_total_tasks;i=i+num_threads){
            //    if (i>=num_total_tasks) break;
            //    runnable->runTask(i,num_total_tasks);
            //}
            while(curr_task<num_total_tasks){
                int local_task_id = ++curr_task;
                if (local_task_id >= num_total_tasks) break;
                runnable->runTask(local_task_id,num_total_tasks);
            } 
            //printf("thread %d finish run\n",thread_id);
            finish_counter++;
        }
        else {
            lk.unlock();
            //if (!running) return;
        }
    }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    this->runnable = runnable;
    this->num_total_tasks = num_total_tasks;
    start_state->mutex_ptr->lock();
    finish_counter = 0;
    curr_task = -1;
    for (int i=0;i<num_threads;i++){
        start_state->thread_first[i] = true;
    }
    start_state->counter = 0;
    //printf("start %d threads\n",num_threads);
    while(start_state->counter < num_threads){
        start_state->mutex_ptr->unlock();
        start_state->cond_ptr->notify_all();
        start_state->mutex_ptr->lock();
    }
    start_state->mutex_ptr->unlock();
    //std::unique_lock<std::mutex> lk(*finish_state->mutex_ptr);
    while(finish_counter < num_threads){
        //cond_boss->wait(lk);
    }
    
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
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
    this->num_threads = num_threads-1;
    running = true;
    start_state = new ThreadState(num_threads-1);
    finish_state = new ThreadState(num_threads-1);
    runnable = new DefaultIRunnable();
    workers = new std::thread[num_threads-1];
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
    running = false;
    start_state->mutex_ptr->lock();
    for (int i=0;i<num_threads;i++){
        start_state->thread_first[i] = true;
    }
    start_state->counter = 0;
    while(start_state->counter<num_threads){
        start_state->mutex_ptr->unlock();
        start_state->cond_ptr->notify_all();
        start_state->mutex_ptr->lock();
    }
    start_state->mutex_ptr->unlock();
    for (int i=0;i<num_threads;i++){
        workers[i].join();
    }
    delete start_state;
    delete finish_state;
    delete[] workers;
}

void TaskSystemParallelThreadPoolSleeping::runThread(int thread_id){
    while(true){ 
        std::unique_lock<std::mutex> lk(*start_state->mutex_ptr);
        start_state->cond_ptr->wait(lk);
        if (start_state->thread_first[thread_id]){
            start_state->thread_first[thread_id] = false;
            start_state->counter++;
            lk.unlock();
            //printf("thread %d start run\n",thread_id);
            if (!running) return;
            //for (int i=thread_id;i<num_total_tasks;i=i+num_threads){
            //    if (i>=num_total_tasks) break;
            //    runnable->runTask(i,num_total_tasks);
            //}
            while(curr_task<num_total_tasks){
                int local_task_id = ++curr_task;
                if (local_task_id >= num_total_tasks) break;
                runnable->runTask(local_task_id,num_total_tasks);
            } 
            //printf("thread %d finish run\n",thread_id);
            finish_state->mutex_ptr->lock();
            finish_state->counter++;
            while(finish_state->counter==num_threads){    
                finish_state->mutex_ptr->unlock();
                finish_state->cond_ptr->notify_all();
                finish_state->mutex_ptr->lock();
            }
            finish_state->mutex_ptr->unlock();
        }
        else {
            lk.unlock();
            //if (!running) return;
        }
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    this->runnable = runnable;
    this->num_total_tasks = num_total_tasks;
    start_state->mutex_ptr->lock();
    curr_task = -1;
    for (int i=0;i<num_threads;i++){
        start_state->thread_first[i] = true;
    }
    start_state->counter = 0;
    //printf("start %d threads\n",num_threads);
    while(start_state->counter < num_threads){
        start_state->mutex_ptr->unlock();
        start_state->cond_ptr->notify_all();
        start_state->mutex_ptr->lock();
    }
    start_state->mutex_ptr->unlock();
    if (finish_state->counter==num_threads){
        finish_state->mutex_ptr->lock();
        finish_state->counter = 0;
        finish_state->mutex_ptr->unlock();
    }else{
        std::unique_lock<std::mutex> lk(*finish_state->mutex_ptr);
        finish_state->cond_ptr->wait(lk);
        finish_state->counter = 0;  
        lk.unlock();     
    }
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
