#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
#include <thread>

#pragma once

enum class TaskPriority {
	High,
	Low
};

// **����� Thread Pool ������ �-C++11**
class MTThreadPool {

private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_highPriorityTasks;
    std::queue<std::function<void()>> m_lowPriorityTasks;
    std::mutex m_queueMutex;
    std::condition_variable condition;
    size_t m_workersNum;
    bool m_stop;
    bool m_pauseLowTasks;

    void workerThread();

    MTThreadPool(const MTThreadPool&) = delete;
    MTThreadPool& operator=(const MTThreadPool&) = delete;

    MTThreadPool();
    ~MTThreadPool();
public:
	void enqueueTask(std::function<void()> task, TaskPriority priority = TaskPriority::Low); // Add a task to the queue with specified priority
    size_t Get_ThreadsNum() { return m_workersNum; }
    void Set_PauseLowPriorityTasks(bool pause) { m_pauseLowTasks = pause; }
    bool IsLowPriorityPaused() { return m_pauseLowTasks; }
    bool IsCurrentThreadWorker() const; // Check if current thread is a worker thread
    // Static method to get the instance of the Singleton
    static MTThreadPool* getInstance() {
        static MTThreadPool instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }
};
