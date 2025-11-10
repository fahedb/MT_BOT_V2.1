#include "HTTPServicesQueues.h"

std::string HTTPServicesQueues::Get_FromQueue(HTTPQueueType queueType)
{
    std::string result;
    int idx = static_cast<int>(queueType);
    if (idx < 0 || idx >= m_queues.size())
        return result; // Invalid queue type

    std::lock_guard<std::mutex> lock(m_queueMutexes[idx]); // Lock the mutex for thread-safe access
    if (!m_queues[idx].empty()) {
        result = m_queues[idx].front();
        m_queues[idx].pop();
    }
    return result;
}

void HTTPServicesQueues::Push_ToStack(HTTPQueueType stackType, const std::string& data)
{
    if (static_cast<size_t>(stackType) >= m_stackMutexes.size())
        return; // Invalid queue type
    std::lock_guard<std::mutex> lock(m_stackMutexes[static_cast<int>(stackType)]);
    m_stacks[static_cast<int>(stackType)].push(data);
}

std::string HTTPServicesQueues::Pop_FromStack(HTTPQueueType stackType)
{
    int idx = static_cast<int>(stackType);
    if (idx < 0 || idx >= m_stacks.size())
        return ""; // Invalid stack type
    std::lock_guard<std::mutex> lock(m_stackMutexes[idx]);
    if (m_stacks[idx].empty())
        return ""; // Stack is empty
    std::string data = m_stacks[idx].top();
    m_stacks[idx].pop();
    return data;
}

void HTTPServicesQueues::Push_ToQueue(HTTPQueueType queueType, const std::string& data)
{
    if (static_cast<size_t>(queueType) >= m_queues.size())
        return; // Invalid queue type
    std::lock_guard<std::mutex> lock(m_queueMutexes[static_cast<size_t>(queueType)]);
    m_queues[static_cast<size_t>(queueType)].push(data);
}

HTTPServicesQueues::HTTPServicesQueues() {
    // Private constructor to prevent instantiation
    HTTPQueueTypeList queueTypes;
    for (auto type : queueTypes.types)
    {
        m_queues.emplace_back(std::queue<std::string>());
        m_queueMutexes.emplace_back(std::mutex());
    }
    HTTPStackTypeList stackTypes;
    for (auto type : stackTypes.types)
    {
        m_stacks.emplace_back(std::stack<std::string>());
        m_stackMutexes.emplace_back(std::mutex());
    }
}
