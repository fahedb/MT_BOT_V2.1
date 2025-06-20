#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>

#pragma once

enum class TaskPriority {
	High,
	Low
};

// **מחלקת Thread Pool מותאמת ל-C++11**
class ThreadPool {

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

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadPool();
    ~ThreadPool();
public:
	void enqueueTask(std::function<void()> task, TaskPriority priority = TaskPriority::Low); // Add a task to the queue with specified priority
    size_t Get_ThreadsNum() { return m_workersNum; }
    void Set_PauseLowPriorityTasks(bool pause) { m_pauseLowTasks = pause; }
    bool IsLowPriorityPaused() { return m_pauseLowTasks; }
    // Static method to get the instance of the Singleton
    static ThreadPool* getInstance() {
        static ThreadPool instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }
};
