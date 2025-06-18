#include "ThreadPool.h"

// Constructor
ThreadPool::ThreadPool() : stop(false) {
    size_t numThreads = std::thread::hardware_concurrency() - 1; // Get the number of hardware threads
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] { workerThread(); });
    }
}

// Close all threads
ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (auto& worker : workers) {
        worker.join();
    }
}

// Add a task to the queue
void ThreadPool::enqueueTask(std::function<void()> task,TaskPriority priority) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);

        if (priority == TaskPriority::High) {
            highPriorityTasks.push(std::move(task));
		}
        else {
            lowPriorityTasks.push(std::move(task));
        }
    }
    condition.notify_one();
}

// Logic for threads - Running till there is no tasks
void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] {
                return stop || !highPriorityTasks.empty() || !lowPriorityTasks.empty();
                });

            if (!highPriorityTasks.empty()) {
                task = std::move(highPriorityTasks.front());
                highPriorityTasks.pop();
            }
            else if (!lowPriorityTasks.empty()) {
                task = std::move(lowPriorityTasks.front());
                lowPriorityTasks.pop();
            }
			else if (stop) {
				return; // Exit if stop is true and no tasks are available
			}
        }
        try {
            task();
        }
        catch (const std::exception& e) {
            std::cerr << "Task threw exception: " << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "Task threw unknown exception." << std::endl;
        }
    }
}