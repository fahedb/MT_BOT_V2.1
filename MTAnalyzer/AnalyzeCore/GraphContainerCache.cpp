#include "pch.h"
#include "GraphContainerCache.h"

void GraphContainerCache::Set_NewContainer(GraphContainer& container,const std::string& parentId)
{
	std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety

    auto* packs = Get_GraphPacks(container.Get_FilterType());
    for (auto& pack : *packs)
    {
        if (pack.first.Get_Id() == parentId)
        {
            auto& myMap = pack.second;
            auto sonIdLong = Converts::ToLong(container.Get_Id());
            auto sonContainer = myMap.find(sonIdLong);
            if (sonContainer != myMap.end())
            {
                // Update existing container
                GraphContainerCacheUtils::CompleteMissingData(&myMap, container);
                sonContainer->second = container; // Replace with the new container
            }
            else
            {
                // Add new container
                myMap[sonIdLong] = container;
            }
        }
	}
}

void GraphContainerCache::Set_PngPath(const std::string& paterntId, const std::string& sonId, const std::string& path, PatternsCompare_FilterType fType, PatternCompare_GraphType gType)
{
    std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety
    if (path.empty())
        return; // No path provided, nothing to set
    // Validate the filter type
    if (fType != PatternsCompare_FilterType::ALL && fType != PatternsCompare_FilterType::TradesOnly)
		return; // Invalid filter type, do nothing

	// Get the appropriate graph packs based on the filter type
	auto* packs = Get_GraphPacks(fType);
    for (auto& pack : *packs)
    {
        if (pack.first.Get_Id() == paterntId)
        {
            auto& myMap = pack.second;
            auto sonIdLong = Converts::ToLong(sonId);
            auto sonContainer = myMap.find(sonIdLong);
            if (sonContainer != myMap.end())
            {
                if (gType == PatternCompare_GraphType::Bars)
                    sonContainer->second.Set_GraphPngPath(path, PatternCompare_GraphType::Bars);
                else if (gType == PatternCompare_GraphType::Linear)
                    sonContainer->second.Set_GraphPngPath(path, PatternCompare_GraphType::Linear);
            }
        }
    }
}

void GraphContainerCache::Set_GraphStruct(const std::string& paterntId, const std::string& sonId, GraphStructure& gSt, PatternsCompare_FilterType fType, PatternCompare_GraphType gType)
{
	std::lock_guard<std::mutex> lock(m_mutex); // Ensure thread safety

    auto* packs = Get_GraphPacks(fType);
    for (auto& pack : *packs)
    {
        if (pack.first.Get_Id() == paterntId)
        {
            auto& myMap = pack.second;
            auto sonIdLong = Converts::ToLong(sonId);
            auto sonContainer = myMap.find(sonIdLong);
            if (sonContainer != myMap.end())
            {
                if (gType == PatternCompare_GraphType::Bars)
                    sonContainer->second.Set_GraphStructure(gSt, PatternCompare_GraphType::Bars);
                else if (gType == PatternCompare_GraphType::Linear)
                    sonContainer->second.Set_GraphStructure(gSt, PatternCompare_GraphType::Linear);
            }
        }
	}
}

std::vector<GraphContainer> GraphContainerCache::Get_AllContainers_ForCurrentPoint(const std::string& id,PatternsCompare_FilterType fType)
{
	auto* myMap = Get_GraphPacks(fType);
    std::vector<GraphContainer> resVec;
    for (auto& pack : *myMap)
    {
        if (pack.first.Get_Id() == id)
        {
			auto& myVec = pack.second;
            for (auto& container : myVec)
            {
                resVec.push_back(container.second);
            }
        }
    }
	return resVec;
}

GraphContainer* GraphContainerCache::Get_Container(std::string parentId, std::string sonId, PatternsCompare_TimeLineType tlT)
{
    if (tlT == PatternsCompare_TimeLineType::History)
    {
        return Get_SonContainer(parentId, sonId);
    }
    else if (tlT == PatternsCompare_TimeLineType::Current)
    {
        return Get_ParentContainer(parentId, PatternsCompare_FilterType::ALL);
    }
    return nullptr;
}

GraphContainer* GraphContainerCache::Get_SonContainer(const std::string& parentId, const std::string& sonId)
{
    auto* packs = Get_GraphPacks(PatternsCompare_FilterType::ALL);
    for (auto& pack : *packs)
    {
        if (pack.first.Get_Id() == parentId)
        {
            auto& myMap = pack.second;
            auto sonIdLong = Converts::ToLongLong(sonId);
            auto sonContainer = myMap.find(sonIdLong);
            if (sonContainer != myMap.end())
            {
                return &sonContainer->second;
            }
        }
    }
    return nullptr;
}

GraphContainer* GraphContainerCache::Get_ParentContainer(const std::string& parentId, PatternsCompare_FilterType fType)
{
    auto* packs = Get_GraphPacks(fType);
    for (auto& pack : *packs)
    {
        if (pack.first.Get_Id() == parentId)
        {
            return &pack.first;
        }
    }
    return nullptr;
}

std::vector<GraphPack>* GraphContainerCache::Get_GraphPacks(PatternsCompare_FilterType fType)
{
    if (fType == PatternsCompare_FilterType::ALL)
        return &m_scansGraphPacks;
    else if (fType == PatternsCompare_FilterType::TradesOnly)
        return &m_tradesGraphPacks;
    else
        return nullptr; // Invalid filter type
}

GraphPack* GraphContainerCache::Get_GraphPack(PatternsCompare_FilterType fType, const std::string& parentId)
{
    auto* packs = Get_GraphPacks(fType);
    for (auto& pack : *packs)
    {
        if (pack.first.Get_Id() == parentId)
        {
            return &pack;
        }
    }
    return nullptr;
}
