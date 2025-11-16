#pragma once
#include "ScoringLayerImporter.h"
#include "FilterLayerDataImporter.h"
#include "StatisticsLayerDataImporter.h"
#include "TradeLayerDataImporter.h"

#include "..\MTSimulator_DataExporter\LayersEnums.h"

class InterfaceLayers_BKTSTImporter
{

public:
    // Deleted copy constructor and assignment operator to prevent copies
    InterfaceLayers_BKTSTImporter(const InterfaceLayers_BKTSTImporter&) = delete;
    InterfaceLayers_BKTSTImporter& operator=(const InterfaceLayers_BKTSTImporter&) = delete;

    // Static method to get the instance of the Singleton
    static InterfaceLayers_BKTSTImporter& getInstance() {
        static InterfaceLayers_BKTSTImporter instance; // Guaranteed to be destroyed and instantiated on first use
        return instance;
    }


    template<typename T>
	T* GetLayerImporter(LayerType layerT); // Get the specified layer importer
	LayerType Get_InitializedLayerType() const { return m_initializedLayerType; }
	template<typename QueueItemType>
	QueueItemType Get_NextQueueItem(); // Get next queue item from the specified layer importer
    
	template<typename QueueItemType>
	long long Get_MyQueuedItem_TimePoint(const QueueItemType& item) const; // Get next queue item time point from the specified layer importer

    std::string Get_BKTSTName() const { return m_bktstName; }

    void Set_HisyoryTL(std::vector<ChangeNode*>* historyTl, DBMap* histMap);
    void InitImporters();
	bool IsImportedDataAvailable() const { return m_isInitialized; }

private:
    InterfaceLayers_BKTSTImporter();
    bool ActPreCondition(LayerType layerType) const; // Check if the importers are initialized
	void ResetImportersToNull(); // Reset all importers -- Need to call InitImporters after this function to use again
    ScoringLayerImporter* m_scoringLayerImporter = nullptr;
    FilterLayerDataImporter* m_filterLayerImporter = nullptr;
    StatisticsLayerDataImporter* m_statisticsLayerImporter = nullptr;
    TradeLayerDataImporter* m_tradeLayerImporter = nullptr;
    std::string m_bktstName;

	std::vector<ChangeNode*>* m_historyTL = nullptr;
	DBMap* m_historyDBMap = nullptr;
	LayerType m_initializedLayerType = LayerType::NONE;
	bool m_isInitialized = false;
};



template<typename QueueItemType>
QueueItemType InterfaceLayers_BKTSTImporter::Get_NextQueueItem()
{
	LayerType layerT = m_initializedLayerType;
	if (!ActPreCondition(layerT))
		return QueueItemType();

	return QueueItemType();
}


template<typename QueueItemType>
long long InterfaceLayers_BKTSTImporter::Get_MyQueuedItem_TimePoint(const QueueItemType& item) const // Get next queue item time point from the specified layer importer
{
	LayerType layerT = m_initializedLayerType;
	if (!ActPreCondition(layerT))
		return -1;
	return -1;
}


template<typename T>
T* InterfaceLayers_BKTSTImporter::GetLayerImporter(LayerType layerT)
{
	if (!ActPreCondition(layerT))
		return nullptr;
	switch (layerT)
	{
	case LayerType::ScoringLayer:
		return static_cast<T*>(m_scoringLayerImporter);
	case LayerType::FilterLayer:
		return static_cast<T*>(m_filterLayerImporter);
	case LayerType::StatisticsLayer:
		return static_cast<T*>(m_statisticsLayerImporter);
	case LayerType::TradeManagementLayer:
		return static_cast<T*>(m_tradeLayerImporter);
	default:
		return nullptr;
	}
}


// ------------------ Scoring & Filter Layer Specialization ------------------
template<>
inline ScoringLayer_QueueItemType // This explicit specialization is for scoring and filter layer -- both have the same QueueItemType
InterfaceLayers_BKTSTImporter::Get_NextQueueItem<ScoringLayer_QueueItemType>() {
	LayerType layerT = m_initializedLayerType;
    if (!ActPreCondition(layerT))
        return {};
	
	switch (layerT)
	{
	case LayerType::ScoringLayer:
		return m_scoringLayerImporter->Get_NextItem().item;
	case LayerType::FilterLayer:
		return m_filterLayerImporter->Get_NextItem().item;
	default:
		return {};
	}
}

template<>
inline long long InterfaceLayers_BKTSTImporter::Get_MyQueuedItem_TimePoint(const ScoringLayer_QueueItemType& item) const // Get next queue item time point from the specified layer importer
{
	LayerType layerT = m_initializedLayerType;
	if (!ActPreCondition(layerT))
		return -1;
	if (layerT == LayerType::FilterLayer)
		return m_filterLayerImporter->Get_DataTime(item);
	else if (layerT == LayerType::ScoringLayer)
		return m_scoringLayerImporter->Get_DataTime(item);
	return -1;
}

// ------------------ Statistics Layer Specialization ------------------

template<>
inline StatisticsLayer_QueueItemType // This explicit specialization is for statistics layer
InterfaceLayers_BKTSTImporter::Get_NextQueueItem<StatisticsLayer_QueueItemType>() {
	LayerType layerT = m_initializedLayerType;
	if (!ActPreCondition(layerT))
		return {};
	return std::move(m_statisticsLayerImporter->Get_NextItem().item);
}

template<>
inline long long InterfaceLayers_BKTSTImporter::Get_MyQueuedItem_TimePoint(const StatisticsLayer_QueueItemType& item) const // Get next queue item time point from the specified layer importer
{
	LayerType layerT = m_initializedLayerType;
	if (!ActPreCondition(layerT))
		return -1;
	return m_statisticsLayerImporter->Get_DataTime(item);
}

// ------------------ Trade Layer Specialization ------------------
template<>
TradeLayer_QueueItemType // This explicit specialization is for trade layer
inline InterfaceLayers_BKTSTImporter::Get_NextQueueItem<TradeLayer_QueueItemType>() {
	LayerType layerT = m_initializedLayerType;
	if (!ActPreCondition(layerT))
		return {};
	return std::move(m_tradeLayerImporter->Get_NextItem().item);
}


template<>
inline long long InterfaceLayers_BKTSTImporter::Get_MyQueuedItem_TimePoint(const TradeLayer_QueueItemType& item) const // Get next queue item time point from the specified layer importer
{
	LayerType layerT = m_initializedLayerType;
	if (!ActPreCondition(layerT))
		return -1;
	return m_tradeLayerImporter->Get_DataTime(item);
}