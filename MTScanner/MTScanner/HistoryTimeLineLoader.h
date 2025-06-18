#include "ChangeData.h"
#include "XAUUSD_CurrentTable.h"
#include "XAUUSD_OneMinTable.h"
#include "ExchangeData.h"
#include "ThreadPool.h"

#include <future>

#pragma once
#define LOAD_CHANGE_NODES_THREAD_NUM std::thread::hardware_concurrency() - 1
class HistoryTimeLineLoader
{
public:
	HistoryTimeLineLoader(std::vector<ChangeNode*>& hitoryTimeLine,BasePricesTable* table) : 
		m_hitoryTimeLine(hitoryTimeLine),
		m_table(table)
	{}

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
	static std::vector<ChangeNode*> LoadChangeNodesForThread(int threadIdx, BasePricesTable* tb)
	{
		std::vector<ChangeNode*> localHistoryTimeLine;
		localHistoryTimeLine.reserve(tb->GetSelected_Count_MultiThreaded(0));

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

	}
	void LoadDataIntoRAM(std::vector<ChangeNode*>& hitoryTimeLine, BasePricesTable* tb)
	{
		Logger& logger = Logger::getInstance();

		tb->SelectDataFromJsonFile();
		logger.WriteLog("Done pulling from DB.", false, true);

		logger.WriteLog("Start Loading data into RAM...", false, true);
		int threadCount = static_cast<int>(LOAD_CHANGE_NODES_THREAD_NUM);

		logger.WriteLog("Trying to reserve " + std::to_string(tb->GetSelected_Count_MultiThreaded(0) * threadCount) + " Data cells...");

		hitoryTimeLine.reserve(static_cast<size_t>(tb->GetSelected_Count_MultiThreaded(0)) * static_cast<size_t>(threadCount));
		logger.WriteLog("Done reserving.");

	}

	void ChangeNodeLoader(std::vector< std::vector<ChangeNode*>>* reads2D, BasePricesTable* tb)
	{
		Logger& logger = Logger::getInstance();
		int threadCount = static_cast<int>(LOAD_CHANGE_NODES_THREAD_NUM);

		logger.WriteLog("Threads Available for parallel Build: " + std::to_string(threadCount), false, true);
		int chunkSize = tb->GetSelected_Count_MultiThreaded(0);
		logger.WriteLog("Every thread should build " + std::to_string(chunkSize) + " CHange nodes.", false, true);


		std::vector<std::future<void>> futures;

		for (int t = 0; t < threadCount; ++t) {
			auto prom = std::make_shared<std::promise<void>>();
			futures.push_back(prom->get_future());

			ThreadPool::getInstance()->enqueueTask(
				[t, tb, reads2D, prom]() {
					processChunk(t, tb, reads2D);
					prom->set_value();
				},
				TaskPriority::High
			);
		}

		for (auto& f : futures)
			f.get(); // Wait till the end
	}
	static void processChunk(int threadIndex, BasePricesTable* tableObj, std::vector< std::vector<ChangeNode*>>* reads2D) {

		Logger& logger = Logger::getInstance();

		logger.WriteLog("Thread [" + std::to_string(threadIndex) + "] Started Building ChangeNodes...");

		(*reads2D)[threadIndex] = std::move(HistoryTimeLineLoader::LoadChangeNodesForThread(threadIndex, tableObj)); // קריאה מקובץ

		logger.WriteLog("Thread [" + std::to_string(threadIndex) + "] Done Building " + std::to_string(reads2D[threadIndex].size()) + " ChangeNodes.");

	}
	void ChangeNodeThreadsLoadsMerger(std::vector< std::vector<ChangeNode*>>& reads2D, std::vector<ChangeNode*>& hitoryTimeLine)
	{
		int threadCount = static_cast<int>(LOAD_CHANGE_NODES_THREAD_NUM);
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

		LoadDataIntoRAM(m_hitoryTimeLine, tb);

		std::vector< std::vector<ChangeNode*>> reads2D(static_cast<size_t>(LOAD_CHANGE_NODES_THREAD_NUM));

		ChangeNodeLoader(&reads2D, tb);
		ChangeNodeThreadsLoadsMerger(reads2D, m_hitoryTimeLine);

		ExchangeData exh;
		exh.DoneLoadingData();
	}
};

