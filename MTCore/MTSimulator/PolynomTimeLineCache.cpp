#include "PolynomTimeLineCache.h"

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