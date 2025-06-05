#include <map>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>

#define SCORE_RESULTS_CACHE_SIZE_PER_THREAD "SCORE_RESULTS_CACHE_SIZE_PER_THREAD"
#define THREADS_COUNT "THREADS_COUNT"
#pragma once
class ResourceManager // Singleton class for cache data -- No need to use currently -- for future usage
{
	std::unordered_map<std::string, std::string> m_cacheData; // Cache data storage, can be used for storing any kind of data
	std::mutex m_cacheMutex; // Mutex for thread-safe access to the cache

public:
	std::string Get_Data(const std::string& key) {
		auto it = m_cacheData.find(key);
		if (it != m_cacheData.end()) {
			return it->second; // Return the cached data if found
		}
		return ""; // Return empty string if not found
	}
	void Set_Data(const std::string& key, const std::string& value) {
		std::lock_guard<std::mutex> lock(m_cacheMutex); // Lock the mutex for thread-safe access
		m_cacheData[key] = value; // Set the data in the cache
	}

private:

    ResourceManager()
    {

    }
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

public:
    // Static method to get the instance of the Singleton
    static ResourceManager* getInstance() {
        static ResourceManager instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }
    
};

