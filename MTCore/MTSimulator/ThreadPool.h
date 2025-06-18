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
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> highPriorityTasks;
    std::queue<std::function<void()>> lowPriorityTasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;

    void workerThread();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadPool();
    ~ThreadPool();
public:
	void enqueueTask(std::function<void()> task, TaskPriority priority = TaskPriority::Low); // Add a task to the queue with specified priority
    // Static method to get the instance of the Singleton
    static ThreadPool* getInstance() {
        static ThreadPool instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }
};
