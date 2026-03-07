#include "HistoryTimeLineLoader.h"
#include "Logger.h"
#include "ThreadPool.h"
#include "PolynomTimeLineCache.h"

std::vector<ChangeNode*> HistoryTimeLineLoader::LoadChangeNodesForThread(int threadIdx, int chunkSize, BasePricesTable* tb)
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

void HistoryTimeLineLoader::InitHistoryToVectorLoader(BasePricesTable* tb)
{
	Logger& logger = Logger::getInstance();
	logger.WriteLog("Pulling data from DB...", false, true);
	if (m_dbFileName.empty())
		m_dbFileName = Configrations::getInstance()->Get_DB_FileDir() + tb->GetDB_PhysicalFileName_Json();
	tb->Set_DATA_DB_FileName(m_dbFileName);
	tb->Set_MultiThreadedRead();
	m_chunkSize = tb->InitFileMapForRead();

}

void HistoryTimeLineLoader::ChangeNodeLoader(std::vector<std::vector<ChangeNode*>>* reads2D, BasePricesTable* tb)
{
	Logger& logger = Logger::getInstance();
	int threadCount = static_cast<int>(MTThreadPool::getInstance()->Get_ThreadsNum());

	std::vector<std::future<void>> futures;

	for (int t = 0; t < threadCount; ++t) {
		auto prom = std::make_shared<std::promise<void>>();
		futures.push_back(prom->get_future());
		int chunkSize = m_chunkSize;
		MTThreadPool::getInstance()->enqueueTask(
			[t, chunkSize, tb, reads2D, prom]() {
				try {

				processChunk(t, chunkSize, tb, reads2D);
				prom->set_value();
				}
				catch (const std::exception& ex) {
					std::cerr << "Exception in thread: " << ex.what() << std::endl;
					prom->set_exception(std::current_exception());
				}
			},
			TaskPriority::High
		);
	}

	for (auto& f : futures)
		f.get(); // Wait till the end
}

void HistoryTimeLineLoader::processChunk(int threadIndex, int chunkSize, BasePricesTable* tableObj, std::vector<std::vector<ChangeNode*>>* reads2D) {

	Logger& logger = Logger::getInstance();

	logger.WriteLog("Thread [" + std::to_string(threadIndex) + "] Started Building ChangeNodes...");

	(*reads2D)[threadIndex] = std::move(HistoryTimeLineLoader::LoadChangeNodesForThread(threadIndex, chunkSize, tableObj)); // Read changeNodes for this thread

	logger.WriteLog("Thread [" + std::to_string(threadIndex) + "] Done Building " + std::to_string(reads2D->at(threadIndex).size()) + " ChangeNodes.");

}

void HistoryTimeLineLoader::ChangeNodeThreadsLoadsMerger(std::vector<std::vector<ChangeNode*>>& reads2D, std::vector<ChangeNode*>& hitoryTimeLine)
{
	int threadCount = static_cast<int>(MTThreadPool::getInstance()->Get_ThreadsNum());
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

void HistoryTimeLineLoader::LoadHistoryToVector_MultiThreaded(BasePricesTable* tb)
{
	InitHistoryToVectorLoader(tb); // Initialize the loader with the table

	std::vector< std::vector<ChangeNode*>> reads2D(static_cast<size_t>(MTThreadPool::getInstance()->Get_ThreadsNum()));

	ChangeNodeLoader(&reads2D, tb); // Load ChangeNodes into a 2D vector, each thread gets its own vector
	ChangeNodeThreadsLoadsMerger(reads2D, m_hitoryTimeLine); // Merge the results from all threads into a single vector
	
	ExchangeData().DoneLoadingData(); // Notify that data loading is done
}


void HistoryTimeLineLoader::Run(const std::string& dbFileName)
{
	if (!m_table) // If no table is provided, create a default one
		m_table = new XAUUSD_OneMinTable(Configrations::getInstance()->Get_DB_FileDir());
	m_dbFileName = dbFileName;
	LoadHistoryToVector_MultiThreaded(m_table);
	m_table->RemoveSelect();
	int polysBuilt = PolynomTimeLineCacheManager::Build_PoylnomTimeLines_MultiThreaded(m_hitoryTimeLine, CacheType::HISTORICAL); // Build polynomial timelines from the loaded ChangeNodes and cache them
	Logger::getInstance().WriteLog("Built " + std::to_string(polysBuilt) + " polynomial timelines from the history loaded ChangeNodes.", true, true, LogType::SPECIAL_LOG);
}
