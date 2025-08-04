#include "TimeLine.h"
#include "CONFIGRATIONS.H"
#include "PolynomTimeLine.h"
#include <memory>

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

    // Insert with a key and a unique_ptr to PolynomTimeLine
    void InsertPolynomTimeLine(const PolynomTimeLineKey& key, std::unique_ptr<PolynomTimeLine> polynomTl, PolynomTimeLineCacheType cachT) {
        auto cache = GetPolynomTimeLineCache(cachT);
        cache->try_emplace(key, std::move(polynomTl));
    }

    // Insert using the segment as the key
    void InsertPolynomTimeLine(std::unique_ptr<PolynomTimeLine> polynomTl, PolynomTimeLineCacheType cachT) {
        auto cache = GetPolynomTimeLineCache(cachT);
        
        PolynomTimeLineKey key;
        key.startTimePoint = polynomTl->Get_Segment().first;
        key.endTimePoint = polynomTl->Get_Segment().second;
        cache->try_emplace(key, std::move(polynomTl));
    }

    PolynomTimeLine* GetPolynomTimeLine(const PolynomTimeLineKey& key, PolynomTimeLineCacheType cachT)  {
		auto cache = GetPolynomTimeLineCache(cachT);

        auto it = cache->find(key);
        if (it != cache->end()) {
            return it->second.get();
        }
        return nullptr;
    }


private:
    robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>>* GetPolynomTimeLineCache(PolynomTimeLineCacheType type) {
        if (type == PolynomTimeLineCacheType::HISTORICAL) {
            return &m_hist_polynomTimeLineCache;
        } else {
            return &m_curr_polynomTimeLineCache;
        }
	}
    robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>> m_hist_polynomTimeLineCache;
    robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>> m_curr_polynomTimeLineCache;
};




/// <summary>
/// This Class is responsible for managing the cache of polynomial timelines.
/// </summary>
/// Author: Fahed Elbatash
/// Date: 2025-07-22
class PolynomTimeLineCacheManager
{
private:
    static bool PreConditionBuild()
    {
        // Check if the cache is enabled and if the polynom length is set
        Configrations* config = Configrations::getInstance();
        if (!config->Get_Policy()->EnablePolynomScan || config->Get_PolynomPolicy()->PolynomLen_ForScan <= 0) {
            Logger::getInstance().WriteLog("Polynom scan is not enabled or polynom length is not set.", true, true, LogType::ERR);
            return false; // Precondition failed
        }
		return true; // Precondition passed
    }
public:
    static int Build_PoylnomTimeLines(const std::vector<ChangeNode*>& nodes, PolynomTimeLineCacheType cachT) {
        
        if(!PreConditionBuild()) {
            return 0; // Precondition failed, no polynom timelines built
		}

        // Create a PolynomTimeLine from the nodes
		PolynomTimeLineCache* cache = PolynomTimeLineCache::getInstance();
		Configrations* config = Configrations::getInstance();
        int polyLen = config->Get_PolynomPolicy()->PolynomLen_ForScan;

		int buildCount = 0;
        int nodesSize = nodes.size();

        if (nodesSize < polyLen) {
            Logger::getInstance().WriteLog("Not enough nodes to build a polynom timeline.", true, true, LogType::ERR);
            return 0; // Not enough nodes to build a polynom timeline
		}

		// Build each polyLen changeNodes into a PolynomTimeLine
        for (size_t i = 0; i <= nodesSize - polyLen - 1; i++) {
            std::vector<ChangeNode*> segmentNodes(nodes.begin() + i, nodes.begin() + i + polyLen);
            if (segmentNodes.size() < polyLen) continue; // Skip if not enough nodes

			// Check if the PolynomTimeLine already exists in the cache
			PolynomTimeLineKey key(segmentNodes.front()->point_DateTimeLong, segmentNodes.back()->point_DateTimeLong);
            if (cache->GetPolynomTimeLine(key,cachT) != nullptr) {
                continue; // Skip if already exists
			}

            auto polynomTimeLine = std::make_unique<PolynomTimeLine>(segmentNodes);
            cache->InsertPolynomTimeLine(key, std::move(polynomTimeLine), cachT);
            buildCount++;
        }
		return buildCount;

	}
};