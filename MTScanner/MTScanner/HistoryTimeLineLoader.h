#pragma once
#include "ChangeData.h"
#include "XAUUSD_CurrentTable.h"
#include "XAUUSD_OneMinTable.h"
#include "ExchangeData.h"
#include "ThreadPool.h"
#include <future>
#include "PolynomTimeLine.h"
#include "PolynomTimeLineCache.h"

// <summary>
/// HistoryTimeLineLoader class for loading
/// historical data into a vector of ChangeNode objects.
/// It uses multi-threading to efficiently load data from a database table.
/// </summary>
/// Author: Fahed Elbatash
/// Date: 2025-07-12

class HistoryTimeLineLoader
{
public:
	HistoryTimeLineLoader(std::vector<ChangeNode*>& hitoryTimeLine,BasePricesTable* table) : 
		m_hitoryTimeLine(hitoryTimeLine),
		m_table(table)
	{
		m_chunkSize = 0;
	}

	void Run()
	{
		if (!m_table)
			m_table = new XAUUSD_OneMinTable(Configrations::getInstance()->Get_DB_FileDir());

		LoadHistoryToVector_MultiThreaded(m_table);
		m_table->RemoveSelect();
		int polysBuilt = PolynomTimeLineCacheManager::Build_PoylnomTimeLines_MultiThreaded(m_hitoryTimeLine,CacheType::HISTORICAL); // Build polynomial timelines from the loaded ChangeNodes and cache them
		Logger::getInstance().WriteLog("Built " + std::to_string(polysBuilt) + " polynomial timelines from the history loaded ChangeNodes.", true, true, LogType::SPECIAL_LOG);
	}

private:
	std::vector<ChangeNode*>& m_hitoryTimeLine;
	BasePricesTable* m_table = nullptr; // Pointer to the table, can be used for other purposes if needed
	int m_chunkSize;

	static std::vector<ChangeNode*> LoadChangeNodesForThread(int threadIdx, int chunkSize, BasePricesTable* tb); // Load ChangeNode objects for a specific thread
	void InitHistoryToVectorLoader(BasePricesTable* tb); // Initialize the loader by setting the database file name and preparing for multi-threaded reading
	void ChangeNodeLoader(std::vector< std::vector<ChangeNode*>>* reads2D, BasePricesTable* tb); // Load ChangeNode objects into a 2D vector, where each thread gets its own vector
	static void processChunk(int threadIndex, int chunkSize, BasePricesTable* tableObj, std::vector< std::vector<ChangeNode*>>* reads2D); // Process a chunk of data in a specific thread
	void ChangeNodeThreadsLoadsMerger(std::vector< std::vector<ChangeNode*>>& reads2D, std::vector<ChangeNode*>& hitoryTimeLine); // Merge results from all threads into a single vector of ChangeNode objects
	void LoadHistoryToVector_MultiThreaded(BasePricesTable* tb); // Load historical data into a vector of ChangeNode objects using multi-threading
};

