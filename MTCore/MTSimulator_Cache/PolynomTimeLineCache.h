#include "TimeLine.h"
#include "CONFIGRATIONS.H"
#include "PolynomTimeLine.h"
#include "..\MyRobinHoodMap\HashingStructs.h"
#include <memory>
#include <future>
#include "CacheEnums.h"
#include "BTimeLineCache.h"

#pragma once

// <summary>
/// This class is a Singleton that manages a cache of polynomial timelines.
/// It allows inserting and retrieving polynomial timelines based on a key.
/// </summary>
/// Author: Fahed Elbatash
/// Date: 2025-07-22
/// 

class PolynomTimeLineCache : public BaseTimeLineCache<PolynomTimeLineKey, PolynomTimeLine>
{
private:
    std::mutex m_cacheMutex; // Mutex for thread safety

public:
    PolynomTimeLineCache(const PolynomTimeLineCache&) = delete;
    PolynomTimeLineCache& operator=(const PolynomTimeLineCache&) = delete;
    PolynomTimeLineCache(PolynomTimeLineCache&&) = delete;
    PolynomTimeLineCache& operator=(PolynomTimeLineCache&&) = delete;

    PolynomTimeLineCache() : BaseTimeLineCache<PolynomTimeLineKey, PolynomTimeLine>(
        1,
		50000)
    {
    }
    
    static PolynomTimeLineCache* getInstance() {
        static PolynomTimeLineCache instance;
        return &instance;
    }
    virtual void CleanTask(long long oldestTP, BaseCacheMap* relevantMap) override;
	virtual long long GetStartTP(const PolynomTimeLineKey& key) override { return key.startTimePoint; } // Use the start time point of the key as the reference
    void InsertPolynomTimeLine(PolynomTimeLineKey& key, PolynomTimeLine&& polynomTl, CacheType cachT, bool externalLock = false) {  // Insert with a key and a PolynomTimeLine (move)
        if (!externalLock)
            std::lock_guard<std::mutex> lock(m_cacheMutex); // Ensure thread safety
        Insert(key, std::move(polynomTl), cachT, 0);
    }
    virtual void ClearOldResults(bool forceSynchronous) override;

    PolynomTimeLine* GetPolynomTimeLine(PolynomTimeLineKey& key, CacheType cachT)// Retrieve a PolynomTimeLine by its key
    {
        return Get(key, cachT, 0);
    }
	void MergeIntoCache(robin_hood::unordered_map<PolynomTimeLineKey, PolynomTimeLine>& tempCache, CacheType cachT); // Merge a temporary cache into the main cache
    
    // Helper functions
    int GetMonthIdx(long long tp);
    int GetDayIdx(long long tp);

private:
};


/// <summary>
/// This Class is responsible for managing the cache of polynomial timelines.
/// </summary>
/// Author: Fahed Elbatash
/// Date: 2025-07-22
class PolynomTimeLineCacheManager
{
private:
	static bool PreConditionBuild(); // Checks if the preconditions for building polynomial timelines are met, such as whether polynomial scanning is enabled and the polynomial length is set.
	
    static void MergeTempCaches_Into_SignletonCache(std::vector<robin_hood::unordered_map<PolynomTimeLineKey, PolynomTimeLine>>& tempCaches, CacheType cachT); // Merges temporary caches into the main cache.
public:
	static int Build_PoylnomTimeLines_MultiThreaded(const std::vector<ChangeNode*>& nodes, CacheType cachT); // Builds polynomial timelines from a vector of ChangeNode pointers in a multi-threaded manner.
	static PolynomTimeLine* Get_PolynomTimeLine(robin_hood::unordered_map<PolynomTimeLineKey, PolynomTimeLine>& cache, const PolynomTimeLineKey& key, CacheType cachT); // Retrieves a polynomial timeline from the cache based on the provided key and cache type.
	static void Build_PolynomTimeLines_InTempMap(const std::vector<ChangeNode*>& nodes, CacheType cachT, robin_hood::unordered_map<PolynomTimeLineKey, PolynomTimeLine>& tempCache); // Builds polynomial timelines from a vector of ChangeNode pointers and stores them in a temporary cache.
	static int Build_PoylnomTimeLines(const std::vector<ChangeNode*>& nodes, CacheType cachT);// Builds polynomial timelines from a vector of ChangeNode pointers and inserts them into the main cache.
};


