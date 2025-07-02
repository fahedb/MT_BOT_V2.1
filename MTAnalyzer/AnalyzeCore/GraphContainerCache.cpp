#include "pch.h"
#include "GraphContainerCache.h"

void GraphContainerCache::Set_NewContainer(GraphContainer& container)
{
    PatternsCompare_TimeLineType tlT = container.Get_TimeLineType();
    auto* myMap = Get_Map(tlT);

    auto pr = std::make_pair(container.Get_FilterType(), Converts::ToLong(container.Get_Id()));
    auto val = myMap->find(pr);

    if (val != myMap->end())
        GraphContainerCacheUtils::CompleteMissingData(myMap, container);
    else
        myMap->try_emplace(pr, container);
}

void GraphContainerCache::Set_PngPath(GraphContainerKey key, const std::string& path, PatternCompare_GraphType gType, PatternsCompare_TimeLineType tlT)
{
    auto* myMap = Get_Map(tlT);
    auto val = myMap->find(key);
    if (val != myMap->end())
    {
        val->second.Set_GraphPngPath(path, gType);
    }
}

void GraphContainerCache::Set_GraphStruct(GraphContainerKey key, GraphStructure& gSt, PatternCompare_GraphType gType, PatternsCompare_TimeLineType tlT)
{
    auto* myMap = Get_Map(tlT);
    auto val = myMap->find(key);
    if (val != myMap->end())
    {
        val->second.Set_GraphStructure(gSt, gType);
    }
}

std::pair<std::vector<GraphContainer>, std::vector<GraphContainer>> GraphContainerCache::Get_AllContainers(PatternsCompare_FilterType fType)
{
    std::pair<std::vector<GraphContainer>, std::vector<GraphContainer>> resPairVec;
    if (fType != PatternsCompare_FilterType::NONE)
        return resPairVec;

    auto* currMap = Get_Map(PatternsCompare_TimeLineType::Current);
    auto* histMap = Get_Map(PatternsCompare_TimeLineType::History);
    resPairVec.first = GraphContainerCacheUtils::Get_Containers(currMap, fType);
    resPairVec.second = GraphContainerCacheUtils::Get_Containers(histMap, fType);
    return resPairVec;
}
