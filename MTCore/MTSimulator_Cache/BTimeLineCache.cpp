#include "BTimeLineCache.h"
#include "Static_CurrentTimeLineRep.h"
#include "LOGGER.H"
#include "Converts.h"
#include "ThreadPool.h"

// Pure virtual clear old method
template<typename ObjectKey, typename ObjectType>
void BaseTimeLineCache<ObjectKey, ObjectType>::SynchronouslyClearOldResults(long long oldestTP)
{
    for (auto& threadCache : m_hist) {
        for (auto& monthCache : threadCache) {
            for (auto& dayCache : monthCache) {
                CleanTask(oldestTP, &dayCache); // Thread index is not relevant in synchronous mode
            }
        }
    }
}

template<typename ObjectKey, typename ObjectType>
void BaseTimeLineCache<ObjectKey, ObjectType>::AsynchronouslyClearOldResults(long long oldestTP)
{
    // Limit the number of concurrent tasks to prevent deadlock
    // Use fewer tasks than available threads to avoid overwhelming the pool
	int availableThreads = static_cast<int>(MTThreadPool::getInstance()->Get_ThreadsNum());
    const int maxConcurrentTasks = std::min(static_cast<int>(availableThreads - 1), static_cast<int>(m_hist.size() * m_hist[0].size() * m_hist[0][0].size()));

    std::vector<std::future<void>> futures;
    futures.reserve(maxConcurrentTasks); // Pre-allocate to avoid reallocation

    int taskCount = 0;
    for (auto& threadCache : m_hist) {
        for (auto& monthCache : threadCache) {
            for (auto& dayCache : monthCache) {

                if (taskCount >= maxConcurrentTasks) {
                    // Wait for some tasks to complete before adding more
                    for (auto& f : futures) {
                        f.wait();
                    }
                    futures.clear();
                    taskCount = 0;
                }

                auto prom = std::make_shared<std::promise<void>>();
                futures.push_back(prom->get_future());
                auto* relevantMap = &dayCache;
                MTThreadPool::getInstance()->enqueueTask(
                    [this, oldestTP, prom, relevantMap]() {
                        CleanTask(oldestTP, relevantMap); // Thread index is not relevant in asynchronous mode
                        prom->set_value();
                    },
                    TaskPriority::High
                );
                taskCount++;

            }
        }
    }
    Logger::getInstance().WriteLog("Cleaning Results Cache Started", false, true, LogType::LOG);

    // Wait for all tasks with timeout to prevent infinite waiting
    const int timeoutSeconds = 180; // 30 seconds timeout
    auto startTime = std::chrono::steady_clock::now();

    for (auto& f : futures) {
        auto status = f.wait_for(std::chrono::seconds(timeoutSeconds));
        if (status == std::future_status::timeout) {
            Logger::getInstance().WriteLog("Cleaning Results Cache - Timeout occurred, some tasks may not have completed", false, true, LogType::ERR);
            break;
        }
    }

}

template<typename ObjectKey, typename ObjectType>
void BaseTimeLineCache<ObjectKey, ObjectType>::ClearOldResults(bool forceSynchronous)
{
    if (!CleanPreCondition())
        return;

    //Getting the older TimePoint
    long long oldestTP = Converts::ToLongLong(Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLine()->at(0)->point_DateTime);

    // If forced synchronous, we're inside a worker thread, or if we want to avoid potential deadlocks
    if (forceSynchronous || MTThreadPool::getInstance()->IsCurrentThreadWorker()) {
        Logger::getInstance().WriteLog("Cleaning Results Cache - Running synchronously", false, true, LogType::LOG);
        SynchronouslyClearOldResults(oldestTP);
        Logger::getInstance().WriteLog("Cleaning Results Cache Finished (synchronous)", false, true, LogType::LOG);
        m_cleaningInProgress.store(false);
        return;
    }

    // Check if we have enough threads available
    size_t availableThreads = MTThreadPool::getInstance()->Get_ThreadsNum();
    if (availableThreads <= 1) {
        // If only one thread is available, run synchronously to avoid deadlock
        Logger::getInstance().WriteLog("Cleaning Results Cache - Running synchronously (only one thread available)", false, true, LogType::LOG);
        SynchronouslyClearOldResults(oldestTP);
        Logger::getInstance().WriteLog("Cleaning Results Cache Finished (synchronous)", false, true, LogType::LOG);
        m_cleaningInProgress.store(false);
        return;
    }

    MTThreadPool::getInstance()->Set_PauseLowPriorityTasks(true);
    AsynchronouslyClearOldResults(oldestTP);
    MTThreadPool::getInstance()->Set_PauseLowPriorityTasks(false);

    Logger::getInstance().WriteLog("Cleaning Results Cache Finished", false, true, LogType::LOG);
    m_cleaningInProgress.store(false); // Reset the flag 
}

template<typename ObjectKey, typename ObjectType>
inline bool BaseTimeLineCache<ObjectKey, ObjectType>::Insert(const ObjectKey& key, const ObjectType& obj, CacheType cachT, int threadIdx) // virtual insert method
{
    if (!m_initialized)
        m_initialized = true;
    if (m_cleaningInProgress.load())
        return false; // If cleaning is in progress, do not allow insertion

    if (cachT == CacheType::HISTORICAL) {
        long long startTP = GetStartTP(key);
        int monthIdx = GetMonthIdx(startTP);
        int dayIdx = GetDayIdx(startTP);
        auto& cache = m_hist[threadIdx][monthIdx][dayIdx];
        auto result = cache.try_emplace(key, std::move(obj));
        return result.second; // Return true if insertion took place, false if key already existed
    }
    else {
        auto result = m_curr.try_emplace(key, std::move(obj));
        return result.second; // Return true if insertion took place, false if key already existed
    }
}

template<typename ObjectKey, typename ObjectType>
inline bool BaseTimeLineCache<ObjectKey, ObjectType>::Insert(const ObjectKey& key, ObjectType&& obj, CacheType cachT, int threadIdx) // Overload for move semantics (for unique_ptr support)
{
    if (!m_initialized)
        m_initialized = true;
    if (m_cleaningInProgress.load())
        return false; // If cleaning is in progress, do not allow insertion
    if (cachT == CacheType::HISTORICAL) {
        long long startTP = GetStartTP(key);
        int monthIdx = GetMonthIdx(startTP);
        int dayIdx = GetDayIdx(startTP);
        auto& cache = m_hist[threadIdx][monthIdx][dayIdx];
        auto result = cache.try_emplace(key, std::move(obj));
        return result.second; // Return true if insertion took place, false if key already existed
    }
    else {
        auto result = m_curr.try_emplace(key, std::move(obj));
        return result.second; // Return true if insertion took place, false if key already existed
    }
}

template<typename ObjectKey, typename ObjectType>
ObjectType* BaseTimeLineCache<ObjectKey, ObjectType>::Get(const ObjectKey& key, CacheType cachT, int threadIdx) // virtual get method
{
    if (cachT == CacheType::HISTORICAL) {
        long long startTP = GetStartTP(key);
        int monthIdx = GetMonthIdx(startTP);
        int dayIdx = GetDayIdx(startTP);
        auto& cache = m_hist[threadIdx][monthIdx][dayIdx];
        auto it = cache.find(key);
        if (it != cache.end()) {
            return &it->second;
        }
        return nullptr;
    }
    else {
        auto it = m_curr.find(key);
        if (it != m_curr.end()) {
            return &it->second;
        }
        return nullptr;
    }
}

template<typename ObjectKey, typename ObjectType>
bool BaseTimeLineCache<ObjectKey, ObjectType>::CleanPreCondition()
{

    if (m_cleaningInProgress.exchange(true)) { // Try to lock the atomic bool
        Logger::getInstance().WriteLog("Cleaning is already in progress, skipping this call.", false, false, LogType::ERR);
        return false; // Prevent multiple cleaning operations at the same time
    }
    if (!m_initialized)
    {
        m_cleaningInProgress.store(false); // Reset the flag if no cleaning is needed
        Logger::getInstance().WriteLog("Nothing to clean.", false, false, LogType::ERR);
        return false;
    }
    ChangeNode* oldestChN = Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLine()->at(0); // Get the oldest ChangeNode from the current timeline

    if (!oldestChN) {
        m_cleaningInProgress.store(false); // Reset the flag if no cleaning is needed
        Logger::getInstance().WriteLog("Oldest ChangeNode is null, cannot clean old results.", false, false, LogType::ERR);
        return false;
    }

    long long oldestTP = Converts::ToLongLong(oldestChN->point_DateTime);

    if (m_lastCleanedTime >= oldestTP) {
        m_cleaningInProgress.store(false); // Reset the flag if no cleaning is needed
        return false; // No need to clean if the last cleaned time is already newer than the oldest time point
    }
    m_lastCleanedTime = oldestTP; // Update the last cleaned time to the oldest time point
    return true;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
template<typename ObjectKey, typename ObjectType>
std::vector<size_t> BaseTimeLineCache<ObjectKey, ObjectType>::GetCacheSizes(CacheType cT)
{
    std::vector<size_t> sizes;
    if (cT == CacheType::CURRENT) {
        sizes.push_back(m_curr.size());
        return sizes;
    }

    sizes.resize(m_hist.size()); // Resize to number of threads
    for (int i = 0; i < m_hist.size(); i++) {
        size_t totalSize = 0;
        for (const auto& monthCache : m_hist[i]) {
            for (const auto& dayCache : monthCache) {
                totalSize += dayCache.size();
            }
        }
        sizes[i] = totalSize;
    }

    return sizes;
}

template<typename ObjectKey, typename ObjectType>
size_t BaseTimeLineCache<ObjectKey, ObjectType>::GetCacheSize(int threadIdx)
{
    if (threadIdx < 0 || threadIdx >= m_hist.size())
        return 0;
    size_t totalSize = 0;
    for (const auto& monthCache : m_hist[threadIdx]) {
        for (const auto& dayCache : monthCache) {
            totalSize += dayCache.size();
        }
    }
    return totalSize;
}

template<typename ObjectKey, typename ObjectType>
int BaseTimeLineCache<ObjectKey, ObjectType>::GetMonthIdx(long long tp) {
    // Better month extraction for timestamp format YYYYMMDDHHMMSSSSS
    // Extract month from position 5-6 (0-based)
    int month = ((tp / 100000000000LL) % 100);
    // Ensure month is in valid range 1-12, then convert to 0-11
    if (month < 1 || month > 12) {
        return 0; // Default to January if invalid
    }
    return month - 1; // Convert to 0-based index
}

template<typename ObjectKey, typename ObjectType>
inline int BaseTimeLineCache<ObjectKey, ObjectType>::GetDayIdx(long long tp) {
    // Better day extraction for timestamp format YYYYMMDDHHMMSSSSS
    // Extract day from position 7-8 (0-based)
    int day = ((tp / 1000000000LL) % 100);
    // Ensure day is in valid range 1-31, then convert to 0-30
    if (day < 1 || day > 31) {
        return 0; // Default to 1st if invalid
    }
    return day - 1; // Convert to 0-based index
}

// Explicit instantiation for ScoreResultsCachePolynom
#include "ScoreResultsCachePolynom.h"
template class BaseTimeLineCache<std::pair<PolynomTimeLineKey, PolynomTimeLineKey>, ScoresResult>;

// Explicit instantiation for BaseTimeLineCache with long long pairs (used by some components)
template class BaseTimeLineCache<std::pair<long long, long long>, ScoresResult>;

// Explicit instantiation for PolynomTimeLineCache
#include "PolynomTimeLineCache.h"
template class BaseTimeLineCache<PolynomTimeLineKey, PolynomTimeLine>;