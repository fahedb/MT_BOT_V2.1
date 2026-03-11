#pragma once
#include "ScoringLayerImporter.h"
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
	void Insert_BackToFront(QueueItemType item);
	
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
inline void InterfaceLayers_BKTSTImporter::Insert_BackToFront(QueueItemType item)
{
	// Important Node: This generic implementation may not be used if specializations exist for specific QueueItemTypes.
	// This function is just to provide a wide picture of how the logic should work for any QueueItemType :)
	// In such cases, the specialized versions will be called instead.
	// Please don't remove the specializations below, as the will cause a compile-time error if not present (Convertion errors, becuase multiple types in same function).
	QueueStruct<QueueItemType> queueSt;
	queueSt.item = std::move(item);
	queueSt.lineIndex = 0;
	// push into the specified importer
	LayerType layerT = m_initializedLayerType;
	if (!ActPreCondition(layerT))
		return;
	switch (layerT)
	{
	case LayerType::ScoringLayer:
		m_scoringLayerImporter->Insert_LoadedData_ToQueue_Front(std::move(queueSt));
		break;
	case LayerType::StatisticsLayer:
		m_statisticsLayerImporter->Insert_LoadedData_ToQueue_Front(std::move(queueSt));
		break;
	case LayerType::TradeManagementLayer:
		m_tradeLayerImporter->Insert_LoadedData_ToQueue_Front(std::move(queueSt));
		break;
	case LayerType::NONE:
		Logger::getInstance().WriteLog("Insert_BackToFront:: Error in LayerType: NONE", false, true, LogType::ERR);
	}
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
	case LayerType::StatisticsLayer:
		return static_cast<T*>(m_statisticsLayerImporter);
	case LayerType::TradeManagementLayer:
		return static_cast<T*>(m_tradeLayerImporter);
	default:
		return nullptr;
	}
}


// ------------------ Scoring Layer Specialization ------------------
template<>
inline ScoringLayer_QueueItemType // This explicit specialization is for scoring and filter layer -- both have the same QueueItemType
InterfaceLayers_BKTSTImporter::Get_NextQueueItem<ScoringLayer_QueueItemType>() {
	LayerType layerT = m_initializedLayerType;
    if (!ActPreCondition(layerT))
        return {};
	return m_scoringLayerImporter->Get_NextItem().item;
}

template<>
inline long long InterfaceLayers_BKTSTImporter::Get_MyQueuedItem_TimePoint(const ScoringLayer_QueueItemType& item) const // Get next queue item time point from the specified layer importer
{
	LayerType layerT = m_initializedLayerType;
	if (!ActPreCondition(layerT))
		return -1;
	return m_scoringLayerImporter->Get_DataTime(item);
}


template<>
inline void InterfaceLayers_BKTSTImporter::Insert_BackToFront(ScoringLayer_QueueItemType item)
{
	LayerType layerT = m_initializedLayerType;
	QueueStruct<ScoringLayer_QueueItemType> queueStruct;
	queueStruct.item = std::move(item);
	queueStruct.lineIndex = 0;

	if (layerT == LayerType::ScoringLayer)
		m_scoringLayerImporter->Insert_LoadedData_ToQueue_Front(std::move(queueStruct));
	else {
		Logger::getInstance().WriteLog("Insert_BackToFront:: Error in LayerType: " + std::to_string(static_cast<int>(layerT)), false, true, LogType::ERR);
	}
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


template<>
inline void InterfaceLayers_BKTSTImporter::Insert_BackToFront<StatisticsLayer_QueueItemType>(StatisticsLayer_QueueItemType item)
{
	QueueStruct<StatisticsLayer_QueueItemType> queueSt;
	queueSt.item = std::move(item);
	queueSt.lineIndex = 0;

	// push into statistics importer
	if (m_initializedLayerType == LayerType::StatisticsLayer)
		m_statisticsLayerImporter->Insert_LoadedData_ToQueue_Front(std::move(queueSt));
	else {
		Logger::getInstance().WriteLog("Insert_BackToFront:: Error in LayerType for statistics: " + std::to_string(static_cast<int>(m_initializedLayerType)), false, true, LogType::ERR);
	}
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

template<>
inline void InterfaceLayers_BKTSTImporter::Insert_BackToFront<TradeLayer_QueueItemType>(TradeLayer_QueueItemType item)
{
	QueueStruct<TradeLayer_QueueItemType> queueSt;
	queueSt.item = std::move(item);
	queueSt.lineIndex = 0;
	// push into trade importer
	if (m_initializedLayerType == LayerType::TradeManagementLayer)
		m_tradeLayerImporter->Insert_LoadedData_ToQueue_Front(std::move(queueSt));
	else {
		Logger::getInstance().WriteLog("Insert_BackToFront:: Error in LayerType for trade: " + std::to_string(static_cast<int>(m_initializedLayerType)), false, true, LogType::ERR);
	}
}