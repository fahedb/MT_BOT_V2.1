#include "TimeLine.h"
#include "CONFIGRATIONS.H"
#include "PolynomTimeLine.h"
#include <memory>
#include <future>

#pragma once

/// <summary>
/// Class to save current timeline 
/// And long current timeLine (last X bars (by a config) )
/// </summary>
struct PolynomTimeLineKey
{
	// Default constructor
	PolynomTimeLineKey() : startTimePoint(0), endTimePoint(0) {}
	// Constructor with parameters
	PolynomTimeLineKey(long long start,long long end) : startTimePoint(start), endTimePoint(end) {}

	long long startTimePoint; // Start time point of the timeline
	long long endTimePoint; // End time point of the timeline

    bool operator==(const PolynomTimeLineKey& other) const {
        return startTimePoint == other.startTimePoint &&
            endTimePoint == other.endTimePoint;
    }
};

/// <summary>
/// Specializes the robin_hood::hash template to provide a custom hash function for PolynomTimeLineKey objects.
/// This allows PolynomTimeLineKey to be used as a key in unordered maps.
/// </summary>
/// Author: Fahed Elbatash
/// Date: 2025-07-22
namespace robin_hood {
    template<>
	struct hash<PolynomTimeLineKey> { // Custom hash function for PolynomTimeLineKey
        size_t operator()(const PolynomTimeLineKey& key) const {
            size_t h1 = robin_hood::hash<long long>()(key.startTimePoint);
            size_t h2 = robin_hood::hash<long long>()(key.endTimePoint);
            return h1 ^ (h2 << 1);
        }
    };

    // Hash function for pair of PolynomTimeLineKeys
    template <>
    struct hash<std::pair<PolynomTimeLineKey, PolynomTimeLineKey>> {
        size_t operator()(const std::pair<PolynomTimeLineKey, PolynomTimeLineKey>& p) const {
            size_t h1 = robin_hood::hash<PolynomTimeLineKey>{}(p.first);
            size_t h2 = robin_hood::hash<PolynomTimeLineKey>{}(p.second);
            // Combine hashes
            return h1 ^ (h2 << 1);
        }
    };
}
enum class PolynomTimeLineCacheType
{
    HISTORICAL, // Historical polynomial timelines
    CURRENT // Current polynomial timelines
};

// <summary>
/// This class is a Singleton that manages a cache of polynomial timelines.
/// It allows inserting and retrieving polynomial timelines based on a key.
/// </summary>
/// Author: Fahed Elbatash
/// Date: 2025-07-22
class PolynomTimeLineCache
{

public:

    PolynomTimeLineCache() {}
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
private:
    
    robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>>* GetPolynomTimeLineCache(PolynomTimeLineCacheType type);

    robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>> m_hist_polynomTimeLineCache;
    robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>> m_curr_polynomTimeLineCache;
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