#include "PolynomTimeLineCache.h"
#include "Static_CurrentTimeLineRep.h"
#include "Logger.h"
#include "ThreadPool.h"
#include "Converts.h"

void PolynomTimeLineCache::CleanTask(long long oldestTP, BaseCacheMap* cacheMap) // Cleaning task for a specific cache map
{
    for (auto it = cacheMap->begin(); it != cacheMap->end(); ) {
        long long timePoint2Long = it->first.endTimePoint; // Extract the end time point of the timeline
        if (timePoint2Long < oldestTP) {
            it = cacheMap->erase(it); // Remove the element and get the next iterator
        }
        else
            ++it; // Move to the next element
    }
}


void PolynomTimeLineCache::ClearOldResults(bool forceSynchronous)
{
    if (!CleanPreCondition())
        return;
    long long oldestTP = Converts::ToLongLong(Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLine()->at(0)->point_DateTime);
    CleanTask(oldestTP, &m_curr);
    m_cleaningInProgress.store(false); // Reset the flag 
}

bool PolynomTimeLineCache::ActPreCondition()
{
    return m_enablePolynomTimeLineCaching;
}

void PolynomTimeLineCache::MergeIntoCache(robin_hood::unordered_map<PolynomTimeLineKey, PolynomTimeLine>& tempCache, CacheType cachT) {
    
    if(!ActPreCondition()) {
        return; // Cache is not active, do not merge
	}

    std::lock_guard<std::mutex> lock(m_cacheMutex); // Ensure thread safety
    for (auto& p : tempCache) {
        // Insert directly into the cache map
		Insert(p.first, std::move(p.second), cachT, 0);

    }
}

// -------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------

// PolynomTimeLineCacheManager
// PolynomTimeLineCacheManager Functions

bool PolynomTimeLineCacheManager::PreConditionBuild()
{
    // Check if the cache is enabled and if the polynom length is set
	PolynomTimeLineCache* cache = PolynomTimeLineCache::getInstance();
    if(cache == nullptr) {
        Logger::getInstance().WriteLog("PolynomTimeLineCache instance is not initialized.", true, true, LogType::ERR);
        return false; // Cache instance not initialized
	}

    if (cache->m_enablePolynomTimeLineCaching && (!cache->m_enablePolynomScan || cache->m_polynomLen_ForScan <= 0)) {
        Logger::getInstance().WriteLog("Polynom scan is not enabled or polynom length is not set.", true, true, LogType::ERR);
        return false; // Precondition failed
    }
    return true; // Precondition passed
}


PolynomTimeLine* PolynomTimeLineCacheManager::Get_PolynomTimeLine(robin_hood::unordered_map<PolynomTimeLineKey, PolynomTimeLine>& cache, const PolynomTimeLineKey& key, CacheType cachT)
{
    auto it = cache.find(key);
    if (it != cache.end()) {
        return & it->second;
    }
    return nullptr;
}

void PolynomTimeLineCacheManager::Build_PolynomTimeLines_InTempMap(const std::vector<ChangeNode*>& nodes, CacheType cachT, robin_hood::unordered_map<PolynomTimeLineKey, PolynomTimeLine>& tempCache)
{
    if (!PreConditionBuild()) {
        return; // Precondition failed, no polynom timelines built
    }

    // Create a PolynomTimeLine from the nodes
    PolynomTimeLineCache* cache = PolynomTimeLineCache::getInstance();
    int polyLen = cache->m_polynomLen_ForScan;

    int buildCount = 0;
    int nodesSize = nodes.size();

    if (nodesSize < polyLen) {
        Logger::getInstance().WriteLog("Not enough nodes to build a polynom timeline.", true, true, LogType::ERR);
        return; // Not enough nodes to build a polynom timeline
    }

    // Build each polyLen changeNodes into a PolynomTimeLine
    for (size_t i = 0; i <= nodesSize - polyLen - 1; i++) {
        std::vector<ChangeNode*> segmentNodes(nodes.begin() + i, nodes.begin() + i + polyLen);
        if (segmentNodes.size() < polyLen) continue; // Skip if not enough nodes

        // Check if the PolynomTimeLine already exists in the cache
        PolynomTimeLineKey key(segmentNodes.front()->point_DateTimeLong, segmentNodes.back()->point_DateTimeLong);
        if (Get_PolynomTimeLine(tempCache, key, cachT) != nullptr) {
            continue; // Skip if already exists
        }

        PolynomTimeLine polynomTimeLine(segmentNodes);
        tempCache.try_emplace(key, std::move(polynomTimeLine));
        buildCount++;
    }

}

int PolynomTimeLineCacheManager::Build_PoylnomTimeLines_MultiThreaded(const std::vector<ChangeNode*>& nodes, CacheType cachT)
{
    if (!PreConditionBuild()) {
        return 0; // Precondition failed, no polynom timelines built
    }
    // Split the nodes to chunks for multi-threading

    int threadCount = MTThreadPool::getInstance()->Get_ThreadsNum();
    int chunkSize = nodes.size() / threadCount;

    std::vector< std::vector<ChangeNode*>> chunks;
    for (size_t i = 0; i < nodes.size(); i += chunkSize) {
        std::vector<ChangeNode*> chunk(nodes.begin() + i, nodes.begin() + std::min(i + chunkSize, nodes.size()));
        chunks.push_back(chunk);
    }

    std::vector<std::future<void>> futures;

    std::vector<robin_hood::unordered_map<PolynomTimeLineKey, PolynomTimeLine>> tempCaches(threadCount);

    for (int t = 0; t < threadCount; ++t) {
        auto prom = std::make_shared<std::promise<void>>();
        futures.push_back(prom->get_future());
        auto& chunk = chunks[t % chunks.size()]; // Use modulo to handle cases where threadCount > chunks.size()
        auto& tempCache = tempCaches[t];
        MTThreadPool::getInstance()->enqueueTask(
            [chunk, &tempCache, cachT, prom]() {
                Build_PolynomTimeLines_InTempMap(chunk, cachT, tempCache);
                prom->set_value();
            },
            TaskPriority::High
        );
    }

    for (auto& f : futures) {
        f.wait();
    }
	MergeTempCaches_Into_SignletonCache(tempCaches, cachT); // Merge all temporary caches into the singleton cache
    int totalBuilt = 0;
    for (const auto& tempCache : tempCaches)
    {
		totalBuilt += tempCache.size(); // Count the total number of built polynomial timelines
    }
    return totalBuilt;

}

int PolynomTimeLineCacheManager::Build_PoylnomTimeLines(const std::vector<ChangeNode*>& nodes, CacheType cachT) {

    if (!PreConditionBuild()) {
        return 0; // Precondition failed, no polynom timelines built
    }

    // Create a PolynomTimeLine from the nodes
    PolynomTimeLineCache* cache = PolynomTimeLineCache::getInstance();
    int polyLen = cache->m_polynomLen_ForScan;

    int buildCount = 0;
    int nodesSize = nodes.size();

    if (nodesSize < polyLen) {
        Logger::getInstance().WriteLog("Not enough nodes to build a polynom timeline.", true, true, LogType::ERR);
        return 0; // Not enough nodes to build a polynom timeline
    }

    // Build each polyLen changeNodes into a PolynomTimeLine
    robin_hood::unordered_map<PolynomTimeLineKey, PolynomTimeLine> tempCache;

    for (size_t i = 0; i <= nodesSize - polyLen - 1; i++) {
        std::vector<ChangeNode*> segmentNodes(nodes.begin() + i, nodes.begin() + i + polyLen);

        
        if (segmentNodes.size() < polyLen) continue; // Skip if not enough nodes

        // Check if the PolynomTimeLine already exists in the cache
        PolynomTimeLineKey key(segmentNodes.front()->point_DateTimeLong, segmentNodes.back()->point_DateTimeLong);
        if (cache->GetPolynomTimeLine(key, cachT) != nullptr) {
            continue; // Skip if already exists
        }

        PolynomTimeLine polynomTimeLine(segmentNodes);
        tempCache.try_emplace(key, std::move(polynomTimeLine));
        buildCount++;
    }
    cache->MergeIntoCache(tempCache, cachT);
    return buildCount;

}

void PolynomTimeLineCacheManager::MergeTempCaches_Into_SignletonCache(std::vector<robin_hood::unordered_map<PolynomTimeLineKey, PolynomTimeLine>>& tempCaches, CacheType cachT) // Merges temporary caches into the main cache.
{
    PolynomTimeLineCache* cache = PolynomTimeLineCache::getInstance();
    for (auto& tempCache : tempCaches) {
        cache->MergeIntoCache(tempCache, cachT); // Merge each temporary cache into the main cache
	}
}

