#pragma once
#include "TimeLine.h"
#include "XAUUSD_OneMinTable.h"
#include "PatternGod.h"
#include "StatisticsDecession.h"
#include "CONFIGRATIONS.H"
#include "HistoryTimeLineLoader.h"
#include "LOGGER.H"
#include "OrderStruct.h"
#include "LayersEnums.h"
#include "HttpServerSyncer.h"
class PatternGodScanner
{
public:
	PatternGodScanner() : m_scansCounter(0) {}

	void Run()
	{	
		HttpServerSyncer::LoadAllConfigsToHttpCore(); // Load all configs to the HTTP Core
		std::vector<ChangeNode*> history;
		XAUUSD_OneMinTable table(Configrations::getInstance()->Get_DB_FileDir());
		HistoryTimeLineLoader historyLoader(history, &table);
		historyLoader.Run();
		Logger::getInstance().WriteLog("About to run PatternGod !!", true, true, LogType::SPECIAL_LOG);
		TimeLine historyTimeLine(&history);
		Run_PatternGod(historyTimeLine);

	}
	void Set_ExternalCurrTimeLineQueue(bool val) { m_externalCurrTimeLineQueue = val; }
	OrderEntity* Get_LastPlacedOrder();
	DirectionStats* Get_LastStatisticsResult() { return &m_lastStatisticsResult; }

private:
	std::string m_lastTimePoint;
	int m_scansCounter;
	bool m_lastOrderGetPlaced = false;
	bool m_externalCurrTimeLineQueue = false;
	void Run_PatternGod(TimeLine& historyTimeLine);
	bool OneMinutePatternGodLogic(PatternGod* patternScan, StatisticsDecession* statisticsCalcucator, TimeLine* history);
	bool ScanPreCondition(); // Precondition function before start scanning
	void UpdateCurrTimeLine(BasePricesTable* table); // Update the current timeline 
private: 
	// Layers run functions /////////////////////////////////////////////////////////////////////////////
	std::vector<std::pair<ChangeNode*, ScoresResult>>	ScoringLayerRun(PatternGod* patternScan); // Layer 1, This layer should scan for patterns and score them
	std::vector<std::pair<ChangeNode*, ScoresResult>>	FilterLayerRun(PatternGod* patternScan, std::vector<std::pair<ChangeNode*, ScoresResult>>& results); // Layer 2, This layer should filter the scored patterns based on some criteria
	DirectionStats										StatisticsLayerRun(StatisticsDecession* statisticsCalcucator, std::vector<std::pair<ChangeNode*, ScoresResult>> results, TimeLine* historyTL, std::vector<ChangeNode*>* currTL); // Layer 3, This layer should calculate statistics based on the filtered patterns
	std::unique_ptr<OrderEntity>						TradeLayerRun(DirectionStats&); // Layer 4, This layer should decide on trades based on the statistics
	///////////////////////////////////////////////////////////////////////////////////////////////////////


	// Importer data retrieval functions //////////////////////////////////////////////////////////////////
	std::vector<std::pair<ChangeNode*, ScoresResult>>	Get_ScoringLayer_ImporterData_NextItem(); // Get the next queued item from the scoring layer importer
	std::vector<std::pair<ChangeNode*, ScoresResult>>	Get_FilterLayer_ImporterData_NextItem(); // Get the next queued item from the filter layer importer
	DirectionStats										Get_StatisticsLayer_ImporterData_NextItem(); // Get the next queued item from the statistics layer importer
	std::unique_ptr<OrderEntity>						Get_TradeLayer_ImporterData_NextItem(); // Get the next queued item from the trade layer importer
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<typename QueueItemType>
	QueueItemType Get_MyQueuedItem(); // Template function to get the next queued item from the importer (There is specific function for each layer to call this template)
	std::unique_ptr<OrderEntity> m_lastPlacedOrder; // Keep track of the last placed order to avoid duplicates
	DirectionStats m_lastStatisticsResult; // Keep track of the last statistics result to avoid duplicates
	
	LayerType GetInitializedImporterLayerType(); // Get the layer type that the importer is initialized to (If any)
};

