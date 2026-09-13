#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool {
    public:
    //constructor
        ThreadPool(size_t numThreads) : m_stop(false){
            for(size_t i = 0; i< numThreads; ++i){
                m_workers.emplace_back([this](){
                    while(true){
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(m_queueMutex);
                            // puts worker thread to sleep
                            m_cv.wait(lock,[this](){
                                return m_stop || !m_tasks.empty();
                            });

                            if(m_stop && m_tasks.empty()){
                                return;
                            }

                            //extracting the job
                            task= std::move(m_tasks.front());
                            m_tasks.pop();
                        } //mutex lock released

                        task(); //worker finishes running client request 
                    }
                });
            }
        }

        void enqueue(std::function<void()> task) {
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_tasks.push(task);
            }
            m_cv.notify_one();
        }
        
        //destructor 
        ~ThreadPool(){
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_stop = true;
            }
            m_cv.notify_all();

            for(std::thread &worker : m_workers){
                if(worker.joinable()){
                    worker.join(); //waits for threads to finish before freeing memory
                }
            }
        }
        
        private:
            std::vector<std::thread> m_workers;
            std::queue<std::function<void()>> m_tasks; 

            std::mutex m_queueMutex;
            std::condition_variable m_cv;
            bool m_stop;

};

#endif 