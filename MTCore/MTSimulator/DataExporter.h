#pragma once
#include "DirectionStats.h"
#include "ChangeData.h"
#include "LOGGER.H"
#include "JasonHandler.h"
#include "CSVExportData.h"
#include <mutex>

class DataExporter
{
public:
	static std::mutex m_jsonFileLocker;
	static std::mutex m_csvFileLocker;

	static void Export_All_Statistics_Data(DirectionStats& stats, std::vector<ChangeNode*>* potentialRes, bool tradeDecRes);
private: 
	static std::vector<std::string> PrepareSummeryLogs(DirectionStats& stats);
	static void Export_DataToJson(DirectionStats& stats, bool tradeDecRes);
	static void Export_DataToCSV(DirectionStats& stats);
};

