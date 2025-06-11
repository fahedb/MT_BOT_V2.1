#include "IndicatorStructres.h"
#include <mutex>
#include "Converts.h"
#include "Static_CurrentTimeLineRep.h"
#include "ResourceManager.h"
#include <cstdint>
#include "robin_hood.h"

#pragma once
namespace std {
	template<>
	struct hash<std::pair<ssize_t, ssize_t>> {
		size_t operator()(const std::pair<ssize_t, ssize_t>& p) const noexcept {
			uint64_t h1 = static_cast<uint64_t>(p.first);
			uint64_t h2 = static_cast<uint64_t>(p.second);

			h1 += 0x9e3779b97f4a7c15;
			h1 = (h1 ^ (h1 >> 30)) * 0xbf58476d1ce4e5b9;
			h1 = (h1 ^ (h1 >> 27)) * 0x94d049bb133111eb;
			h1 = h1 ^ (h1 >> 31);

			h2 += 0x9e3779b97f4a7c15;
			h2 = (h2 ^ (h2 >> 30)) * 0xbf58476d1ce4e5b9;
			h2 = (h2 ^ (h2 >> 27)) * 0x94d049bb133111eb;
			h2 = h2 ^ (h2 >> 31);

			return h1 ^ (h2 << 1);
		}
	};
}


class ScoreResultsCache // Singleton class to cache score results
{
private:
	long long m_lastCleanedTime = 0; // Timestamp of the last cleaning operation
	std::atomic<bool> m_cleaningInProgress = false; // Flag to indicate if cleaning is in progress
	std::vector<robin_hood::unordered_map<std::pair<long long, long long>, ScoresResult>> m_scoreResultsCache; // Cache to store scores by time point
	std::vector<std::unique_ptr<std::mutex>> m_threadsLocks; // Mutex for thread-safe access to the cache
public:
	void StoreNewScoreResult(const std::string& timePoint1,const std::string& timePoint2,const ScoresResult& score, int threadIdx)
	{
		// Implementation to store the new score result
		long long timePoint1Long = Converts::ToLongLong(timePoint1);
		long long timePoint2Long = Converts::ToLongLong(timePoint2);
		std::pair<long long, long long> timePointkey = std::make_pair(timePoint1Long, timePoint2Long); // Create a pair of time points for caching
		std::lock_guard<std::mutex> lock(*m_threadsLocks[threadIdx]); // Lock the mutex for thread-safe access
		m_scoreResultsCache[threadIdx].try_emplace(timePointkey, score);

	}

	void StoreNewScoreResult(const std::pair<long long, long long>& timePointCombo,const ScoresResult& score,int threadIdx)
	{
		std::lock_guard<std::mutex> lock(*m_threadsLocks[threadIdx]); // Lock the mutex for thread-safe access
		// Implementation to store the new score result
		m_scoreResultsCache[threadIdx].try_emplace(timePointCombo, score);
	}


	ScoresResult* GetScoreResult(const std::string& timePoint1, const std::string& timePoint2,int threadIdx)
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

	ScoresResult* GetScoreResult(long long timePoint1, long long timePoint2, int threadIdx)
	{
		std::pair<long long, long long> timePointKey = std::make_pair(timePoint1, timePoint2); // Create a pair of time points for caching

		auto it = m_scoreResultsCache.at(threadIdx).find(timePointKey);
		if (it != m_scoreResultsCache.at(threadIdx).end()) {
			return &it->second; // Return the cached score result
		}
		return nullptr; // Return an empty ScoresResult if not found
	}



	void CleanOldResults()
	{
		if (m_cleaningInProgress.exchange(true)) {
			Logger::getInstance().WriteLog("Cleaning is already in progress, skipping this call.", false, false, LogType::ERR);
			return; // Prevent multiple cleaning operations at the same time
		}

		ChangeNode* oldestChN = Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLine()->at(0); // Get the oldest ChangeNode from the current timeline
		
		if (!oldestChN) {
			m_cleaningInProgress.store(false); // Reset the flag if no cleaning is needed
			Logger::getInstance().WriteLog("Oldest ChangeNode is null, cannot clean old results.", false, false, LogType::ERR);
			return;
		}

		long long oldestTP = Converts::ToLongLong(oldestChN->point_DateTime);
		
		if (m_lastCleanedTime >= oldestTP) {
			m_cleaningInProgress.store(false); // Reset the flag if no cleaning is needed
			return; // No need to clean if the last cleaned time is already newer than the oldest time point
		}
		m_lastCleanedTime = oldestTP; // Update the last cleaned time to the oldest time point

		for (int i = 0; i < m_scoreResultsCache.size(); i++)
		{
			CleanOldResultsForThread(i, oldestTP);
		}
		m_cleaningInProgress.store(false); // Reset the flag if no cleaning is needed
	}

	void CleanOldResultsForThread(int threadIdx, long long oldestTP)
	{
		std::lock_guard<std::mutex> lock(*m_threadsLocks[threadIdx]); // Lock the mutex for thread-safe access
		for (auto it = m_scoreResultsCache.at(threadIdx).begin(); it != m_scoreResultsCache.at(threadIdx).end();) {
			std::pair<long long, long long> key = it->first;
			long long timePoint2Long = key.second; // // Extract the second part of the key (timePoint2) - The current timeline TimePoint

			if (timePoint2Long < oldestTP) {
				it = m_scoreResultsCache.at(threadIdx).erase(it); // Remove the element and get the next iterator
			}
			else
				++it; // Move to the next element
		}
	}
public:
	ScoreResultsCache(const ScoreResultsCache&) = delete;
	ScoreResultsCache& operator=(const ScoreResultsCache&) = delete;
	ScoreResultsCache(ScoreResultsCache&&) = delete;
	ScoreResultsCache& operator=(ScoreResultsCache&&) = delete;

	// Static method to get the instance of the Singleton
	static ScoreResultsCache* getInstance() {
		static ScoreResultsCache instance; // Guaranteed to be destroyed and instantiated on first use
		return &instance;
	}
private:
	ScoreResultsCache()
	{
		int threadsCount = Converts::ToInt(ResourceManager::getInstance()->Get_Data(THREADS_COUNT)); // Get the number of threads from the resource manager
		if (threadsCount <= 0) {
			threadsCount = std::thread::hardware_concurrency(); // Default to 1 thread if the count is invalid
		}
		m_scoreResultsCache.resize(threadsCount); // Resize the cache to hold scores for each thread

		long chunkSize = Configrations::getInstance()->Get_Policy()->ScoreResultsCacheSizePerThread; // Get the cache size per thread from the policy
		for (int i = 0; i < threadsCount; i++)
		{
			m_scoreResultsCache.at(i).reserve(chunkSize); // Reserve space for the expected number of results to avoid multiple reallocations
			m_threadsLocks.emplace_back(new std::mutex); // Create a mutex for each thread to ensure thread-safe access to the cache
		}
		CleanOldResults(); // Clean old results at initialization
	}

};

class ScoreResultsCacheUtils
{
public:

	static void CacheScoreResult_InTempVector(std::vector<std::pair<std::pair<long long, long long>, ScoresResult>>& vec, const long long& timePoint1, const long long& timePoint2, ScoresResult& score)
	{
		std::pair<long long, long long> timePointKey = std::make_pair(timePoint1, timePoint2); // Create a pair of time points for caching
		vec.emplace_back(timePointKey, score); // Store the score result in the vector with combined time points as key
	}

	static void CacheScoreResult_InTempVector(std::vector<std::pair<std::pair<long long, long long>,ScoresResult>>& vec,const std::string& timePoint1, const std::string& timePoint2, ScoresResult& score)
	{
		std::pair<long long, long long> timePointKey = std::make_pair(Converts::ToLongLong(timePoint1), Converts::ToLongLong(timePoint2)); // Create a pair of time points for caching
		vec.emplace_back(timePointKey, score); // Store the score result in the vector with combined time points as key
	}
	static void CacheScoreResult_ToCache(std::vector<std::pair<std::pair<long long, long long>,ScoresResult>>& vec,int threadIdx)
	{
		ScoreResultsCache* scoreCache = ScoreResultsCache::getInstance();
		for (const auto& scoreResult : vec)
		{
			scoreCache->StoreNewScoreResult(scoreResult.first, scoreResult.second,threadIdx);
		}
	}
};
