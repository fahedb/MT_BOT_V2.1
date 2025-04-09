#pragma once
#include "DirectionStats.h"
#include "ChangeData.h"
#include "LOGGER.H"
#include "JasonHandler.h"
#include "CSVExportData.h"
#include "PatternExporter.h"
#include <mutex>

/// <summary>
/// Proxy Class to export many kinds of data like graphs and statistics...
/// </summary>
class DataExporter
{
public:
	static std::mutex m_jsonFileLocker;
	static std::mutex m_csvFileLocker;

	static void Export_All_Statistics_Data(DirectionStats& stats, bool tradeDecRes);
private: 
	static std::vector<std::string> PrepareSummeryLogs(DirectionStats& stats);
	static void Export_DataToJson(DirectionStats& stats, bool tradeDecRes);
	static void Export_DataToCSV(DirectionStats& stats);

	static void Send_For_PNGExport(DirectionStats& stats);

};

