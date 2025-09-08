#include "TimeLine.h"
#include "CONFIGRATIONS.H"
#include "PolynomTimeLine.h"
#include "HashingStructs.h"
#include <memory>
#include <future>
#include "CacheEnums.h"

#pragma once

// <summary>
/// This class is a Singleton that manages a cache of polynomial timelines.
/// It allows inserting and retrieving polynomial timelines based on a key.
/// </summary>
/// Author: Fahed Elbatash
/// Date: 2025-07-22
/// 

typedef robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>> PolynomTimeLineCacheMap; //
class PolynomTimeLineCache
{

public:

    PolynomTimeLineCache() {
		m_hist_polynomTimeLineCache.resize(12); // Initialize historical cache for 12 months
    }
    PolynomTimeLineCache(const PolynomTimeLineCache&) = delete;
    PolynomTimeLineCache& operator=(const PolynomTimeLineCache&) = delete;

    static PolynomTimeLineCache* getInstance() {
        static PolynomTimeLineCache instance;
        return &instance;
    }

	
    void InsertPolynomTimeLine(const PolynomTimeLineKey& key, std::unique_ptr<PolynomTimeLine> polynomTl, PolynomTimeLineCacheType cachT);// Insert with a key and a unique_ptr to PolynomTimeLine
    void InsertPolynomTimeLine(std::unique_ptr<PolynomTimeLine> polynomTl, PolynomTimeLineCacheType cachT);// Insert using the segment as the key
    PolynomTimeLine* GetPolynomTimeLine(const PolynomTimeLineKey& key, PolynomTimeLineCacheType cachT);// Retrieve a PolynomTimeLine by its key

	void MergeIntoCache(robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>>& tempCache, PolynomTimeLineCacheType cachT); // Merge a temporary cache into the main cache
    void CleanOldResults();
private:
    void ClearMap(PolynomTimeLineCacheMap& map, long long oldestTP);
    
    robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>>* GetPolynomTimeLineCache(PolynomTimeLineCacheType type, int monthIdx);

    std::vector<PolynomTimeLineCacheMap> m_hist_polynomTimeLineCache;
    PolynomTimeLineCacheMap m_curr_polynomTimeLineCache;
	std::mutex m_cacheMutex; // Mutex for thread safety
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
	
    static void MergeTempCaches_Into_SignletonCache(std::vector<robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>>>& tempCaches, PolynomTimeLineCacheType cachT); // Merges temporary caches into the main cache.
public:
	static int Build_PoylnomTimeLines_MultiThreaded(const std::vector<ChangeNode*>& nodes, PolynomTimeLineCacheType cachT); // Builds polynomial timelines from a vector of ChangeNode pointers in a multi-threaded manner.
	static PolynomTimeLine* Get_PolynomTimeLine(const robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>>& cache, const PolynomTimeLineKey& key, PolynomTimeLineCacheType cachT); // Retrieves a polynomial timeline from the cache based on the provided key and cache type.
	static void Build_PolynomTimeLines_InTempMap(const std::vector<ChangeNode*>& nodes, PolynomTimeLineCacheType cachT, robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>>& tempCache); // Builds polynomial timelines from a vector of ChangeNode pointers and stores them in a temporary cache.
	static int Build_PoylnomTimeLines(const std::vector<ChangeNode*>& nodes, PolynomTimeLineCacheType cachT);// Builds polynomial timelines from a vector of ChangeNode pointers and inserts them into the main cache.
};