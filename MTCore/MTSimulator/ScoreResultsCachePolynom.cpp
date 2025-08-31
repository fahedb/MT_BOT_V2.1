#include "ScoreResultsCachePolynom.h"
#include "ThreadPool.h"


// Hash function for PolynomTimeLineKey



ScoreResultsCachePolynom::ScoreResultsCachePolynom()
{
	int threadsCount = Converts::ToInt(ResourceManager::getInstance()->Get_Data(THREADS_COUNT)); // Get the number of threads from the resource manager
	if (threadsCount <= 0) {
		threadsCount = MTThreadPool::getInstance()->Get_ThreadsNum(); // Default to 1 thread if the count is invalid
	}
	m_scoreResultsCache.resize(threadsCount); // Resize the cache to hold scores for each thread

	long chunkSize = Configrations::getInstance()->Get_Policy<ScoreCachingPolicy>(PolicyType::ScoreCachingPolicy)->ScoreResultsCacheSizePerThread; // Get the cache size per thread from the policy
	for (int i = 0; i < threadsCount; i++)
	{
		m_scoreResultsCache.at(i).resize(12); // Resize the inner vector to hold 12 months (0-11)
		for (int m = 0; m < 12; m++)
			m_scoreResultsCache.at(i).at(m).reserve(chunkSize); // Reserve space for the expected number of results to avoid multiple reallocations
	}
	CleanOldResults(); // Clean old results at initialization
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------

void ScoreResultsCachePolynom::CleanOldResultsForThread(int threadIdx, long long oldestTP, robin_hood::unordered_map<std::pair<PolynomTimeLineKey, PolynomTimeLineKey>, ScoresResult>* relevantMap)
{
	Logger::getInstance().WriteLog("Thread [" + std::to_string(threadIdx) + "] Started cleaning cache...");
	for (auto it = relevantMap->begin(); it != relevantMap->end(); ) {
		long long timePoint2Long = it->first.second.endTimePoint; // Extract the second part of the key (timePoint2) - The current timeline TimePoint

		if (timePoint2Long < oldestTP) {
			it = relevantMap->erase(it); // Remove the element and get the next iterator
		}
		else
			++it; // Move to the next element
	}
	Logger::getInstance().WriteLog("Thread [" + std::to_string(threadIdx) + "] end cleaning cache");
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------

ScoresResult* ScoreResultsCachePolynom::GetScoreResult(const PolynomTimeLineKey& key1, const PolynomTimeLineKey& key2, int threadIdx)
{
	int monthIdx = ExtractMonth(key1.startTimePoint);
	auto it = m_scoreResultsCache.at(threadIdx).at(monthIdx).find(std::make_pair(key1, key2)); // Find the score result in the cache using the pair of PolynomTimeLineKeys
	if (it != m_scoreResultsCache.at(threadIdx).at(monthIdx).end()) {
		return &it->second; // Return the cached score result
	}
	if (false)
	{
		int size = static_cast<int>(m_scoreResultsCache.at(threadIdx).size());
		// get first elemnt in the map
		auto firstElem = m_scoreResultsCache.at(threadIdx).begin();
		int tt = 0;
	}
	return nullptr; // Return an empty ScoresResult if not found
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------------------------------

void ScoreResultsCachePolynom::CleanOldResults(bool forceSynchronous) // Do not use This function in a seperated thread - Use it only in the main thread of scanning
{
	if (!CleanPreCondition())
		return;

	//Getting the older TimePoint
	long long oldestTP = Converts::ToLongLong(Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLine()->at(0)->point_DateTime);

	// If forced synchronous, we're inside a worker thread, or if we want to avoid potential deadlocks
	if (forceSynchronous || MTThreadPool::getInstance()->IsCurrentThreadWorker()) {
		Logger::getInstance().WriteLog("Cleaning Results Cache - Running synchronously", false, true, LogType::LOG);
		for (int i = 0; i < m_scoreResultsCache.size(); i++) {
			for (int t = 0; t < m_scoreResultsCache.at(i).size(); t++) {
				auto* relevantMap = &m_scoreResultsCache.at(i).at(t);
				ScoreResultsCachePolynom::CleanOldResultsForThread(i, oldestTP, relevantMap);
			}
		}
		Logger::getInstance().WriteLog("Cleaning Results Cache Finished (synchronous)", false, true, LogType::LOG);
		m_cleaningInProgress.store(false);
		return;
	}

	// Check if we have enough threads available
	size_t availableThreads = MTThreadPool::getInstance()->Get_ThreadsNum();
	if (availableThreads <= 1) {
		// If only one thread is available, run synchronously to avoid deadlock
		Logger::getInstance().WriteLog("Cleaning Results Cache - Running synchronously (only one thread available)", false, true, LogType::LOG);
		for (int i = 0; i < m_scoreResultsCache.size(); i++) {
			for (int t = 0; t < m_scoreResultsCache.at(i).size(); t++) {
				auto* relevantMap = &m_scoreResultsCache.at(i).at(t);
				ScoreResultsCachePolynom::CleanOldResultsForThread(i, oldestTP, relevantMap);
			}
		}
		Logger::getInstance().WriteLog("Cleaning Results Cache Finished (synchronous)", false, true, LogType::LOG);
		m_cleaningInProgress.store(false);
		return;
	}

	// Limit the number of concurrent tasks to prevent deadlock
	// Use fewer tasks than available threads to avoid overwhelming the pool
	const int maxConcurrentTasks = std::min(static_cast<int>(availableThreads - 1), static_cast<int>(m_scoreResultsCache.size() * m_scoreResultsCache[0].size()));
	
	MTThreadPool::getInstance()->Set_PauseLowPriorityTasks(true);
	std::vector<std::future<void>> futures;
	futures.reserve(maxConcurrentTasks); // Pre-allocate to avoid reallocation
	
	int taskCount = 0;
	for (int i = 0; i < m_scoreResultsCache.size(); i++) {
		for (int t = 0; t < m_scoreResultsCache.at(i).size(); t++) {
			// Limit concurrent tasks
			if (taskCount >= maxConcurrentTasks) {
				// Wait for some tasks to complete before adding more
				for (auto& f : futures) {
					f.wait();
				}
				futures.clear();
				taskCount = 0;
			}
			
			auto prom = std::make_shared<std::promise<void>>();
			futures.push_back(prom->get_future());
			auto* relevantMap = &m_scoreResultsCache.at(i).at(t);
			MTThreadPool::getInstance()->enqueueTask(
				[this, i, oldestTP, prom, relevantMap]() {
					ScoreResultsCachePolynom::CleanOldResultsForThread(i, oldestTP, relevantMap);
					prom->set_value();
				},
				TaskPriority::High
			);
			taskCount++;
		}
	}
	Logger::getInstance().WriteLog("Cleaning Results Cache Started", false, true, LogType::LOG);
	
	// Wait for all tasks with timeout to prevent infinite waiting
	const int timeoutSeconds = 30; // 30 seconds timeout
	auto startTime = std::chrono::steady_clock::now();
	
	for (auto& f : futures) {
		auto status = f.wait_for(std::chrono::seconds(timeoutSeconds));
		if (status == std::future_status::timeout) {
			Logger::getInstance().WriteLog("Cleaning Results Cache - Timeout occurred, some tasks may not have completed", false, true, LogType::ERR);
			break;
		}
	}
	
	Logger::getInstance().WriteLog("Cleaning Results Cache Finished", false, true, LogType::LOG);
	MTThreadPool::getInstance()->Set_PauseLowPriorityTasks(false);
	m_cleaningInProgress.store(false); // Reset the flag 
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------

bool ScoreResultsCachePolynom::CleanPreCondition()
{
	if (m_cleaningInProgress.exchange(true)) { // Try to lock the atomic bool
		Logger::getInstance().WriteLog("Cleaning is already in progress, skipping this call.", false, false, LogType::ERR);
		return false; // Prevent multiple cleaning operations at the same time
	}
	if (!m_scoreResultsCache.at(0).at(0).size())
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

