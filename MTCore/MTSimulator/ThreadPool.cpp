#include "ThreadPool.h"

// Constructor
ThreadPool::ThreadPool() : m_stop(false), m_pauseLowTasks(false) {
    m_workersNum = std::thread::hardware_concurrency() - 1; // Get the number of hardware threads
    for (size_t i = 0; i < m_workersNum; ++i) {
        m_workers.emplace_back([this] { workerThread(); });
    }
}

// Close all threads
ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_stop = true;
    }
    condition.notify_all();
    for (auto& worker : m_workers) {
        worker.join();
    }
}

// Add a task to the queue
void ThreadPool::enqueueTask(std::function<void()> task,TaskPriority priority) {
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);

        if (priority == TaskPriority::High) {
            m_highPriorityTasks.push(std::move(task));
		}
        else {
            m_lowPriorityTasks.push(std::move(task));
        }
    }
    condition.notify_one();
}

// Logic for threads - Running till there is no tasks
void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            condition.wait(lock, [this] {
                // Wait till relevant task
                return m_stop ||
                    !m_highPriorityTasks.empty() ||
                    (!m_lowPriorityTasks.empty() && !m_pauseLowTasks);
                });

            if (m_stop) return;

            if (!m_highPriorityTasks.empty()) {
                task = std::move(m_highPriorityTasks.front());
                m_highPriorityTasks.pop();
            }
            else if (!m_lowPriorityTasks.empty() && !m_pauseLowTasks) {
                task = std::move(m_lowPriorityTasks.front());
                m_lowPriorityTasks.pop();
            }
            else {
                continue; // Nothing valid to do yet
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