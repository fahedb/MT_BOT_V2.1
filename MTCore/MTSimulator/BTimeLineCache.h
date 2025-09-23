#include "CacheEnums.h"
#include "robin_hood.h"
#include "ThreadPool.h"
#include <future>
#include "Converts.h"
#include "LOGGER.H"
#include "Static_CurrentTimeLineRep.h"

// Not used yet

#pragma once
// This class will be the base class of all cache classes
template <typename ObjectKey, typename ObjectType>
class BaseTimeLineCache
{
public:
    BaseTimeLineCache(int threads, long chunkSize)
    {
		m_hist.resize(threads); // Resize historical cache for each thread
        for (auto& monthCache : m_hist) {
            monthCache.resize(12); // Resize month cache for 12 months
            for (auto& dayCache : monthCache) {
                dayCache.resize(31); // Resize day cache for 31 days
                for (auto& cacheMap : dayCache) {
                    cacheMap.reserve(chunkSize); // Reserve space for chunkSize entries to minimize rehashing
				}
            }
        }
    }

    typedef robin_hood::unordered_map<ObjectKey, ObjectType> BaseCacheMap; // Base cache map type
    typedef std::vector<BaseCacheMap> DayInMonthCache; // Cache organized by day in a month
    typedef std::vector<DayInMonthCache> MonthInYearCache; // Cache organized by month in a year
    typedef std::vector<MonthInYearCache> ThreadCache; // Cache organized by thread

    virtual void ClearOldResults(bool forceSynchronous); // Do not use This function in a seperated thread - Use it only in the main thread of scanning, need to freez the main thread to prevent scanning while cleaning
protected:

    ThreadCache m_hist; // Historical cache organized by thread, month and day
    BaseCacheMap m_curr; // Current cache
    
    long long m_lastCleanedTime = 0; // Timestamp of the last cleaning operation
    std::atomic<bool> m_cleaningInProgress = false; // Flag to indicate if cleaning is in progress

	bool m_initialized = false; // Flag to indicate if the cache has been initialized
    
	///////////////////////////////////////////////////////////////////////
	///////////// Insertion and Retrieval methods  /////////////////////////
	///////////////////////////////////////////////////////////////////////

	virtual bool Insert(const ObjectKey& key,const ObjectType& obj, CacheType cachT, int threadIdx = 0);
    virtual bool Insert(const ObjectKey& key, ObjectType&& obj, CacheType cachT, int threadIdx = 0);
    // Overload for move semantics (for unique_ptr support)
    
	virtual ObjectType* Get(const ObjectKey& key, CacheType cachT, int threadIdx) // virtual get method
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
	virtual long long GetStartTP(const ObjectKey& key) = 0; // Pure virtual method to extract start time point from the key
	virtual bool IsCleaningInProgress() { return m_cleaningInProgress.load(); } // Check if cleaning is in progress
	/////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////
	///////////// Cleaning methods  ///////////////////////////////////////
	///////////////////////////////////////////////////////////////////////

	virtual bool CleanPreCondition(); // Precondition check for cleaning
	virtual void SynchronouslyClearOldResults(long long oldestTP); // Synchronous clear old method
	virtual void AsynchronouslyClearOldResults(long long oldestTP); // Asynchronous clear old method

    
    virtual void CleanTask(long long oldestTP, BaseCacheMap* relevantMap) = 0; // Pure virtual clear old method

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////
	///////////// Helper methods  ////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	int GetMonthIdx(long long tp); // Extract month index from timestamp
	int GetDayIdx(long long tp); // Extract day index from timestamp
	///////////////////////////////////////////////////////////////////////////
};
