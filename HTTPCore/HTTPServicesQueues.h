#pragma once
#include <vector>
#include <mutex>
#include <queue>
#include <stack>
#include "HTTPQueuesEnums.h"

/// <summary>
/// HTTP Services Queues
/// Should contain all queues used by the HTTP services
/// It's a singleton class
/// </summary>
class HTTPServicesQueues
{

public:
    // Deleted copy constructor and assignment operator to prevent copies
    HTTPServicesQueues(const HTTPServicesQueues&) = delete;
    HTTPServicesQueues& operator=(const HTTPServicesQueues&) = delete;
    // Static method to get the instance of the Singleton
    static HTTPServicesQueues* getInstance() {
        static HTTPServicesQueues instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }

    void Push_ToQueue(HTTPQueueType queueType, const std::string& data);
    std::string Get_FromQueue(HTTPQueueType queueType);

    void Push_ToStack(HTTPQueueType stackType, const std::string& data);
    std::string Pop_FromStack(HTTPQueueType stackType);

private:
    HTTPServicesQueues();

    std::vector< std::queue<std::string>> m_queues;
	std::vector<std::mutex> m_queueMutexes;

	std::vector<std::stack<std::string>> m_stacks;
    std::vector<std::mutex> m_stackMutexes;

};

