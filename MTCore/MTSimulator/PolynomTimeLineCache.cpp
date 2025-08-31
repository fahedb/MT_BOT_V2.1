#include "PolynomTimeLineCache.h"

//PolynomTimeLineCache
//PolynomTimeLineCache Functions

void PolynomTimeLineCache::InsertPolynomTimeLine(const PolynomTimeLineKey& key, std::unique_ptr<PolynomTimeLine> polynomTl, PolynomTimeLineCacheType cachT) {
    auto cache = GetPolynomTimeLineCache(cachT);
    std::lock_guard<std::mutex> lock(m_cacheMutex); // Ensure thread safety
    cache->try_emplace(key, std::move(polynomTl));
}

void PolynomTimeLineCache::InsertPolynomTimeLine(std::unique_ptr<PolynomTimeLine> polynomTl, PolynomTimeLineCacheType cachT) {
    auto cache = GetPolynomTimeLineCache(cachT);

    PolynomTimeLineKey key;
    key.startTimePoint = polynomTl->Get_Segment().first;
    key.endTimePoint = polynomTl->Get_Segment().second;

    std::lock_guard<std::mutex> lock(m_cacheMutex); // Ensure thread safety
    cache->try_emplace(key, std::move(polynomTl));
}

PolynomTimeLine* PolynomTimeLineCache::GetPolynomTimeLine(const PolynomTimeLineKey& key, PolynomTimeLineCacheType cachT) {
    auto cache = GetPolynomTimeLineCache(cachT);

    auto it = cache->find(key);
    if (it != cache->end()) {
        return it->second.get();
    }
    return nullptr;
}
robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>>* PolynomTimeLineCache::GetPolynomTimeLineCache(PolynomTimeLineCacheType type) {
    if (type == PolynomTimeLineCacheType::HISTORICAL) {
        return &m_hist_polynomTimeLineCache;
    }
    else {
        return &m_curr_polynomTimeLineCache;
    }
}

void PolynomTimeLineCache::MergeIntoCache(robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>>&tempCache, PolynomTimeLineCacheType cachT) {
    auto cache = GetPolynomTimeLineCache(cachT);
    std::lock_guard<std::mutex> lock(m_cacheMutex); // Ensure thread safety
    for (auto& p : tempCache) {
        cache->try_emplace(p.first, std::move(p.second));
    }
}

// -------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------

// PolynomTimeLineCacheManager
// PolynomTimeLineCacheManager Functions

bool PolynomTimeLineCacheManager::PreConditionBuild()
{
    // Check if the cache is enabled and if the polynom length is set
    PolynomComparerPolicy* polynomComparerPolicy = Configrations::getInstance()->Get_Policy<PolynomComparerPolicy>(PolicyType::PolynomComparerPolicy);
    if (!polynomComparerPolicy->EnablePolynomScan || polynomComparerPolicy->PolynomLen_ForScan <= 0) {
        Logger::getInstance().WriteLog("Polynom scan is not enabled or polynom length is not set.", true, true, LogType::ERR);
        return false; // Precondition failed
    }
    return true; // Precondition passed
}


PolynomTimeLine* PolynomTimeLineCacheManager::Get_PolynomTimeLine(const robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>>& cache, const PolynomTimeLineKey& key, PolynomTimeLineCacheType cachT)
{
    auto it = cache.find(key);
    if (it != cache.end()) {
        return it->second.get();
    }
    return nullptr;
}

void PolynomTimeLineCacheManager::Build_PolynomTimeLines_InTempMap(const std::vector<ChangeNode*>& nodes, PolynomTimeLineCacheType cachT, robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>>& tempCache)
{
    if (!PreConditionBuild()) {
        return; // Precondition failed, no polynom timelines built
    }

    // Create a PolynomTimeLine from the nodes
    Configrations* config = Configrations::getInstance();
    int polyLen = config->Get_Policy<PolynomComparerPolicy>(PolicyType::PolynomComparerPolicy)->PolynomLen_ForScan;

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

        auto polynomTimeLine = std::make_unique<PolynomTimeLine>(segmentNodes);
        tempCache.try_emplace(key, std::move(polynomTimeLine));
        buildCount++;
    }

}

int PolynomTimeLineCacheManager::Build_PoylnomTimeLines_MultiThreaded(const std::vector<ChangeNode*>& nodes, PolynomTimeLineCacheType cachT)
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

    std::vector<robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>>> tempCaches(threadCount);

    for (int t = 0; t < threadCount; ++t) {
        auto prom = std::make_shared<std::promise<void>>();
        futures.push_back(prom->get_future());
        auto& chunk = chunks[t % chunks.size()]; // Use modulo to handle cases where threadCount > chunks.size()
        auto& tempCache = tempCaches[t];
        MTThreadPool::getInstance()->enqueueTask(
            [chunk, &tempCache, prom]() {
                Build_PolynomTimeLines_InTempMap(chunk, PolynomTimeLineCacheType::HISTORICAL, tempCache);
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

int PolynomTimeLineCacheManager::Build_PoylnomTimeLines(const std::vector<ChangeNode*>& nodes, PolynomTimeLineCacheType cachT) {

    if (!PreConditionBuild()) {
        return 0; // Precondition failed, no polynom timelines built
    }

    // Create a PolynomTimeLine from the nodes
    PolynomTimeLineCache* cache = PolynomTimeLineCache::getInstance();
    Configrations* config = Configrations::getInstance();
    int polyLen = config->Get_Policy<PolynomComparerPolicy>(PolicyType::PolynomComparerPolicy)->PolynomLen_ForScan;

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
        if (cache->GetPolynomTimeLine(key, cachT) != nullptr) {
            continue; // Skip if already exists
        }

        auto polynomTimeLine = std::make_unique<PolynomTimeLine>(segmentNodes);
        cache->InsertPolynomTimeLine(key, std::move(polynomTimeLine), cachT);
        buildCount++;
    }
    return buildCount;

}

void PolynomTimeLineCacheManager::MergeTempCaches_Into_SignletonCache(std::vector<robin_hood::unordered_map<PolynomTimeLineKey, std::unique_ptr<PolynomTimeLine>>>& tempCaches, PolynomTimeLineCacheType cachT) // Merges temporary caches into the main cache.
{
    PolynomTimeLineCache* cache = PolynomTimeLineCache::getInstance();
    for (auto& tempCache : tempCaches) {
        cache->MergeIntoCache(tempCache, cachT); // Merge each temporary cache into the main cache
	}
}