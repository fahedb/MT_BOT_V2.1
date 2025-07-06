#include "GraphContainer.h"
#include "robin_hood.h"
#include "Converts.h"
#include <mutex>

#pragma once

typedef robin_hood::unordered_map<long long, GraphContainer> GraphContainerMap; // Map to store GraphContainers with long long Ids (son Ids)
typedef std::pair<GraphContainer, GraphContainerMap> GraphPack; // Every GraphPack contains a parent GraphContainer(Main Id) and a map of son GraphContainers (History Ids)

/// <summary>
/// Singleton Class to manage GraphContainers
/// </summary>
class GraphContainerCache
{
public:
    // Deleted copy constructor and assignment operator to prevent copies
    GraphContainerCache(const GraphContainerCache&) = delete;
    GraphContainerCache& operator=(const GraphContainerCache&) = delete;
    // Static method to get the instance of the Singleton
    static GraphContainerCache* getInstance() {
        static GraphContainerCache instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }

    void Set_NewContainer(GraphContainer& container, const std::string& parentId);
    void Set_PngPath(const std::string& paterntId, const std::string& sonId, const std::string& path, PatternsCompare_FilterType fType, PatternCompare_GraphType gType);
    void Set_GraphStruct(const std::string& paterntId, const std::string& sonId, GraphStructure& gSt, PatternsCompare_FilterType fType, PatternCompare_GraphType gType);
    
    std::vector<GraphContainer> Get_AllContainers_ForCurrentPoint(const std::string& id ,PatternsCompare_FilterType fType);
    GraphContainer * Get_Container(std::string parentId,std::string sonId, PatternsCompare_TimeLineType tlT);
	GraphContainer* Get_SonContainer(const std::string& parentId, const std::string& sonId); // Get the son container for a specific parentId (Main Id) and sonId (History Id)
	GraphContainer* Get_ParentContainer(const std::string& parentId, PatternsCompare_FilterType fType); // Get the parent container for a specific parentId (Main Id)
	std::vector<GraphPack>* Get_GraphPacks(PatternsCompare_FilterType fType); // Get the GraphPacks for a specific filter type (ALL or TradesOnly)
	GraphPack* Get_GraphPack(PatternsCompare_FilterType fType, const std::string& parentId); // Get the GraphPack for a specific parentId (Main Id)
private:
    std::vector<GraphPack> m_scansGraphPacks;
    std::vector<GraphPack> m_tradesGraphPacks;
	std::mutex m_mutex; // Mutex for thread safety

    // Private constructor to prevent instantiation
    GraphContainerCache() {

    }
    ~GraphContainerCache()
    {
    }
};

class GraphContainerCacheUtils
{
public:
    static void CompleteMissingData(GraphContainerMap* myMap, GraphContainer& container)
    {
        auto val = myMap->find(Converts::ToLongLong(container.Get_Id()));
        if (val != myMap->end()) {
            GraphContainerCacheUtils::CompleteMissingPaths(val->second, container);
            GraphContainerCacheUtils::CompleteMissingStructures(val->second, container);
        }
    }


private:
    static void CompleteMissingPaths(GraphContainer& dest, GraphContainer& src)
    {
        if (!dest.Get_GraphPngPath(PatternCompare_GraphType::Bars).length())
        {
            dest.Set_GraphPngPath(src.Get_GraphPngPath(PatternCompare_GraphType::Bars), PatternCompare_GraphType::Bars);
        }
        if (!dest.Get_GraphPngPath(PatternCompare_GraphType::Linear).length())
        {
            dest.Set_GraphPngPath(src.Get_GraphPngPath(PatternCompare_GraphType::Linear), PatternCompare_GraphType::Linear);
        }
    }
    static void CompleteMissingStructures(GraphContainer& dest, GraphContainer& src)
    {
        if (!dest.Get_GraphStructure(PatternCompare_GraphType::Bars).points.size())
        {
            dest.Set_GraphStructure(src.Get_GraphStructure(PatternCompare_GraphType::Bars), PatternCompare_GraphType::Bars);
        }
        if (!dest.Get_GraphStructure(PatternCompare_GraphType::Linear).points.size())
        {
            dest.Set_GraphStructure(src.Get_GraphStructure(PatternCompare_GraphType::Linear), PatternCompare_GraphType::Linear);
        }
    }
};

