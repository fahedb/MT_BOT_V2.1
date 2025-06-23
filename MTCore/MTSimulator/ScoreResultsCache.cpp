#include "ScoreResultsCache.h"
#include "ThreadPool.h"

ScoreResultsCache::ScoreResultsCache()
{
	int threadsCount = Converts::ToInt(ResourceManager::getInstance()->Get_Data(THREADS_COUNT)); // Get the number of threads from the resource manager
	if (threadsCount <= 0) {
		threadsCount = ThreadPool::getInstance()->Get_ThreadsNum(); // Default to 1 thread if the count is invalid
	}
	m_scoreResultsCache.resize(threadsCount); // Resize the cache to hold scores for each thread

	long chunkSize = Configrations::getInstance()->Get_Policy()->ScoreResultsCacheSizePerThread; // Get the cache size per thread from the policy
	for (int i = 0; i < threadsCount; i++)
	{
		m_scoreResultsCache.at(i).reserve(chunkSize); // Reserve space for the expected number of results to avoid multiple reallocations
	}
	CleanOldResults(); // Clean old results at initialization
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------

void ScoreResultsCache::CleanOldResultsForThread(int threadIdx, long long oldestTP, robin_hood::unordered_map<std::pair<long long, long long>, ScoresResult>* relevantMap)
{
	Logger::getInstance().WriteLog("Thread [" + std::to_string(threadIdx) + "] Started cleaning cache...");
	for (auto it = relevantMap->begin(); it != relevantMap->end(); ) {
		long long timePoint2Long = it->first.second; // Extract the second part of the key (timePoint2) - The current timeline TimePoint

		if (timePoint2Long < oldestTP) {
			it = relevantMap->erase(it); // Remove the element and get the next iterator
		}
		else
			++it; // Move to the next element
	}
	Logger::getInstance().WriteLog("Thread [" + std::to_string(threadIdx) + "] end cleaning cache");
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------

ScoresResult* ScoreResultsCache::GetScoreResult(const std::string& timePoint1, const std::string& timePoint2, int threadIdx)
{
	long long timePoint1Long = Converts::ToLongLong(timePoint1);
	long long timePoint2Long = Converts::ToLongLong(timePoint2);
	std::pair<long long, long long> timePointKey = std::make_pair(timePoint1Long, timePoint2Long); // Create a pair of time points for caching

	auto it = m_scoreResultsCache.at(threadIdx).find(timePointKey);
	if (it != m_scoreResultsCache.at(threadIdx).end()) {
		return &it->second; // Return the cached score result
	}
	return nullptr; // Return an empty ScoresResult if not found
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------

ScoresResult* ScoreResultsCache::GetScoreResult(long long timePoint1, long long timePoint2, int threadIdx)
{
	auto it = m_scoreResultsCache.at(threadIdx).find(std::make_pair(timePoint1, timePoint2));
	if (it != m_scoreResultsCache.at(threadIdx).end()) {
		return &it->second; // Return the cached score result
	}
	return nullptr; // Return an empty ScoresResult if not found
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------

void ScoreResultsCache::CleanOldResults() // Do not use This function in a seperated thread - Use it only in the main thread of scanning
{
	if (!CleanPreCondition())
		return;

	//Getting the older TimePoint
	long long oldestTP = Converts::ToLongLong(Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLine()->at(0)->point_DateTime);

	ThreadPool::getInstance()->Set_PauseLowPriorityTasks(true);
	std::vector<std::future<void>> futures;
	for (int i = 0; i < m_scoreResultsCache.size(); i++) {
		auto prom = std::make_shared<std::promise<void>>();
		futures.push_back(prom->get_future());
		auto* relevantMap = &m_scoreResultsCache.at(i);
		ThreadPool::getInstance()->enqueueTask(
			[this, i, oldestTP, prom, relevantMap]() {
				ScoreResultsCache::CleanOldResultsForThread(i, oldestTP,relevantMap);
				prom->set_value();
			},
			TaskPriority::High
		);
	}
	Logger::getInstance().WriteLog("Cleaning Results Cache Started", false, true, LogType::LOG);
	for (auto& f : futures)
		f.get(); // Wait till the end
	Logger::getInstance().WriteLog("Cleaning Results Cache Finished", false, true, LogType::LOG);
	ThreadPool::getInstance()->Set_PauseLowPriorityTasks(false);
	m_cleaningInProgress.store(false); // Reset the flag 
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------

bool ScoreResultsCache::CleanPreCondition()
{
	if (m_cleaningInProgress.exchange(true)) { // Try to lock the atomic bool
		Logger::getInstance().WriteLog("Cleaning is already in progress, skipping this call.", false, false, LogType::ERR);
		return false; // Prevent multiple cleaning operations at the same time
	}
	if (!m_scoreResultsCache.at(0).size())
	{
		m_cleaningInProgress.store(false); // Reset the flag if no cleaning is needed
		Logger::getInstance().WriteLog("Nothing to clean.", false, false, LogType::ERR);
		return false;
	}
	ChangeNode* oldestChN = Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLine()->at(0); // Get the oldest ChangeNode from the current timeline

	if (!oldestChN) {
		m_cleaningInProgress.store(false); // Reset the flag if no cleaning is needed
		Logger::getInstance().WriteLog("Oldest ChangeNode is null, cannot clean old results.", false, false, LogType::ERR);
		return false;
	}

	long long oldestTP = Converts::ToLongLong(oldestChN->point_DateTime);

	if (m_lastCleanedTime >= oldestTP) {
		m_cleaningInProgress.store(false); // Reset the flag if no cleaning is needed
		return false; // No need to clean if the last cleaned time is already newer than the oldest time point
	}
	m_lastCleanedTime = oldestTP; // Update the last cleaned time to the oldest time point
	return true;
}

// ----------------------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------------------------------

