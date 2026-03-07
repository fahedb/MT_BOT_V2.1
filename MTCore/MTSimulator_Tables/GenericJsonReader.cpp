#include "GenericJsonReader.h"
#include "ThreadPool.h"

int GenericJsonReader::InitFileMapForRead() { // Init indexes and maps, and return chunk size
	InitFileMap();
	long linesCount = GetFileLinesCount();
	int threadsNum = MTThreadPool::getInstance()->Get_ThreadsNum();
	int chunkSize = static_cast<long>(linesCount) / threadsNum;

	for (int t = 0; t < threadsNum; t++)
	{
		int startIdx = t * chunkSize; // Calculate start index with gap
		int endIdx = (t == threadsNum - 1) ? linesCount : (t + 1) * chunkSize;
		InitLineIdx(startIdx, endIdx, t);
	}
	return chunkSize;
}

void GenericJsonReader::processChunk(int threadIndex, int chunkSize, GenericJsonReader* tableObj, std::vector<std::vector<nlohmann::json>>* reads2D) {
	nlohmann::json jsonEntry = *tableObj->GetNextSelection_newLogic(threadIndex)->Get_Json();

	while (!jsonEntry.is_null()) {
		reads2D->at(threadIndex).push_back(jsonEntry);
		jsonEntry = *tableObj->GetNextSelection_newLogic(threadIndex)->Get_Json();
	}
}

void GenericJsonReader::DataMerger(std::vector<std::vector<nlohmann::json>>* reads2D, std::vector<nlohmann::json>* allReads)
{
	for (const auto& threadReads : *reads2D) {
		for (const auto& jsonEntry : threadReads) {
			allReads->push_back(jsonEntry);
		}
	}
}

void GenericJsonReader::Load()
{
	Set_DATA_DB_FileName(m_filePath);
	Set_MultiThreadedRead();
	m_chunkSize = InitFileMapForRead();

	int threadCount = static_cast<int>(MTThreadPool::getInstance()->Get_ThreadsNum());

	std::vector<std::vector<nlohmann::json>> reads2D;
	reads2D.resize(threadCount);

	std::vector<std::future<void>> futures;

	for (int t = 0; t < threadCount; ++t) {
		auto prom = std::make_shared<std::promise<void>>();
		futures.push_back(prom->get_future());
		int chunkSize = m_chunkSize;
		auto tb = this;
		MTThreadPool::getInstance()->enqueueTask(
			[t, chunkSize, tb, &reads2D, prom]() {
				try {

					processChunk(t, chunkSize, tb, &reads2D);
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

	DataMerger(&reads2D, &m_allJsons);
}
