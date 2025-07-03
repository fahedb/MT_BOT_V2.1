#include "GraphContainer.h"
#include "robin_hood.h"
#include "Converts.h"

#pragma once
struct PairHash {
    std::size_t operator()(const std::pair<PatternsCompare_FilterType, long>& p) const {
        std::size_t h1 = std::hash<PatternsCompare_FilterType>{}(p.first);
        std::size_t h2 = std::hash<long>{}(p.second);
        return h1 ^ (h2 << 1);
    }
    std::size_t operator()(const PatternsCompare_FilterType& val) const {
        return std::hash<std::underlying_type_t<PatternsCompare_FilterType>>{}(static_cast<std::underlying_type_t<PatternsCompare_FilterType>>(val));
    }
};


typedef robin_hood::unordered_map<std::pair<PatternsCompare_FilterType, long>, GraphContainer, PairHash> GraphContainerMap;
typedef std::pair<PatternsCompare_FilterType, long> GraphContainerKey;
typedef std::vector<GraphContainer> GraphContainerVec;

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

    void Set_NewContainer(GraphContainer& container);
    void Set_PngPath(GraphContainerKey key,const std::string& path,PatternCompare_GraphType gType,PatternsCompare_TimeLineType tlT);
    void Set_GraphStruct(GraphContainerKey key, GraphStructure& gSt, PatternCompare_GraphType gType, PatternsCompare_TimeLineType tlT);
    
    std::pair<GraphContainerVec, GraphContainerVec> Get_AllContainers(PatternsCompare_FilterType fType);

private:
    GraphContainerMap* Get_Map(PatternsCompare_TimeLineType tlT) { if (tlT == PatternsCompare_TimeLineType::History) return &m_historyGraphsContainers; else return &m_currentGraphsContainers; }
    GraphContainerMap m_currentGraphsContainers;
    GraphContainerMap m_historyGraphsContainers;


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
        auto pr = std::make_pair(container.Get_FilterType(), Converts::ToLong(container.Get_Id()));
        auto val = myMap->find(pr);
        if (val != myMap->end()) {
            GraphContainerCacheUtils::CompleteMissingPaths(val->second, container);
            GraphContainerCacheUtils::CompleteMissingStructures(val->second, container);
        }
    }

    static std::vector<GraphContainer> Get_Containers(GraphContainerMap* myMap,PatternsCompare_FilterType fType)
    {
        std::vector<GraphContainer> resVec;
        for (auto it = myMap->begin(); it != myMap->end(); ) {
            if (it->first.first == fType)
            {
                resVec.push_back(it->second);
            }
            ++it; // Move to the next element
        }
        return resVec;
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

