#include "ChangeData.h"
#include "XAUUSD_CurrentTable.h"
#include "XAUUSD_OneMinTable.h"
#include "ExchangeData.h"
#include "ThreadPool.h"

#include <future>

#pragma once
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
	}

private:
	std::vector<ChangeNode*>& m_hitoryTimeLine;
	BasePricesTable* m_table = nullptr; // Pointer to the table, can be used for other purposes if needed
	int m_chunkSize;
	static std::vector<ChangeNode*> LoadChangeNodesForThread(int threadIdx,int chunkSize, BasePricesTable* tb)
	{
		std::vector<ChangeNode*> localHistoryTimeLine;
		localHistoryTimeLine.reserve(chunkSize);

		ChangeNode* changeN;
		changeN = tb->GetNextChangeNode_MultiThreaded(threadIdx);
		while (changeN && changeN->activePoint)
		{
			localHistoryTimeLine.push_back(changeN);
			changeN = tb->GetNextChangeNode_MultiThreaded(threadIdx);
		}

		return localHistoryTimeLine;
	}


	void InitHistoryToVectorLoader(BasePricesTable* tb)
	{
		Logger& logger = Logger::getInstance();
		logger.WriteLog("Pulling data from DB...", false, true);
		tb->Set_DATA_DB_FileName(Configrations::getInstance()->Get_DB_FileDir() + tb->GetDB_PhysicalFileName_Json());
		tb->Set_MultiThreadedRead();
		m_chunkSize = tb->InitFileMapForRead();

	}

	void ChangeNodeLoader(std::vector< std::vector<ChangeNode*>>* reads2D, BasePricesTable* tb)
	{
		Logger& logger = Logger::getInstance();
		int threadCount = static_cast<int>(ThreadPool::getInstance()->Get_ThreadsNum());

		std::vector<std::future<void>> futures;

		for (int t = 0; t < threadCount; ++t) {
			auto prom = std::make_shared<std::promise<void>>();
			futures.push_back(prom->get_future());
			int chunkSize = m_chunkSize;
			ThreadPool::getInstance()->enqueueTask(
				[t, chunkSize,tb, reads2D, prom]() {
					processChunk(t,chunkSize, tb, reads2D);
					prom->set_value();
				},
				TaskPriority::High
			);
		}

		for (auto& f : futures)
			f.get(); // Wait till the end
	}
	static void processChunk(int threadIndex,int chunkSize, BasePricesTable* tableObj, std::vector< std::vector<ChangeNode*>>* reads2D) {

		Logger& logger = Logger::getInstance();

		logger.WriteLog("Thread [" + std::to_string(threadIndex) + "] Started Building ChangeNodes...");

		(*reads2D)[threadIndex] = std::move(HistoryTimeLineLoader::LoadChangeNodesForThread(threadIndex,chunkSize, tableObj)); // קריאה מקובץ

		logger.WriteLog("Thread [" + std::to_string(threadIndex) + "] Done Building " + std::to_string(reads2D->at(threadIndex).size()) + " ChangeNodes.");

	}
	void ChangeNodeThreadsLoadsMerger(std::vector< std::vector<ChangeNode*>>& reads2D, std::vector<ChangeNode*>& hitoryTimeLine)
	{
		int threadCount = static_cast<int>(ThreadPool::getInstance()->Get_ThreadsNum());
		// Merge results
		for (int i = 0; i < threadCount; i++)
		{
			std::vector<ChangeNode*> vec = reads2D.at(i);
			for (int t = 0; t < vec.size(); t++)
			{
				hitoryTimeLine.push_back(vec.at(t));
			}
			reads2D.at(i).clear();
		}
		reads2D.clear();
	}

	void LoadHistoryToVector_MultiThreaded(BasePricesTable* tb) 
	{
		InitHistoryToVectorLoader(tb);

		std::vector< std::vector<ChangeNode*>> reads2D(static_cast<size_t>(ThreadPool::getInstance()->Get_ThreadsNum()));

		ChangeNodeLoader(&reads2D, tb);
		ChangeNodeThreadsLoadsMerger(reads2D, m_hitoryTimeLine); 

		ExchangeData exh;
		exh.DoneLoadingData();
	}
};

