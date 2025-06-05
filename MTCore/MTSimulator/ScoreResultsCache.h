#include "IndicatorStructres.h"
#include <mutex>
#include "Converts.h"
#include "Static_CurrentTimeLineRep.h"
#include "ResourceManager.h"

#pragma once
class ScoreResultsCache // Singleton class to cache score results
{
private:
	std::vector<std::unordered_map<std::string, ScoresResult>> m_scoreResultsCache; // Cache to store scores by time point
public:
	void StoreNewScoreResult(const std::string& timePoint1,const std::string& timePoint2,const ScoresResult& score, int threadIdx)
	{
		// Implementation to store the new score result
		std::string timePoint = timePoint1 + "_" + timePoint2;
		auto it = m_scoreResultsCache.at(threadIdx).find(timePoint);
		if (it != m_scoreResultsCache.at(threadIdx).end()) {
			return;
		}
		m_scoreResultsCache.at(threadIdx).emplace(std::make_pair(timePoint, score));
	}

	void StoreNewScoreResult(const std::string& timePointCombo,const ScoresResult& score,int threadIdx)
	{
		// Implementation to store the new score result
		
		auto it = m_scoreResultsCache.at(threadIdx).find(timePointCombo);
		if (it != m_scoreResultsCache.at(threadIdx).end()) {
			return;
		}
		m_scoreResultsCache.at(threadIdx).emplace(std::make_pair(timePointCombo, score));
	}


	ScoresResult* GetScoreResult(const std::string& timePoint1, const std::string& timePoint2,int threadIdx)
	{
		std::string timePoint = timePoint1 + "_" + timePoint2;
		auto it = m_scoreResultsCache.at(threadIdx).find(timePoint);
		if (it != m_scoreResultsCache.at(threadIdx).end()) {
			return &it->second; // Return the cached score result
		}
		return nullptr; // Return an empty ScoresResult if not found
	}

	void CleanOldResults()
	{
		long long oldestTP = Converts::ToLongLong(Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLine()->at(0)->point_DateTime);
		for (int i = 0; i < m_scoreResultsCache.size(); i++)
		{
			CleanOldResultsForThread(i, oldestTP);
		}
	}
	void CleanOldResultsForThread(int threadIdx, long long oldestTP)
	{
		for (auto it = m_scoreResultsCache.at(threadIdx).begin(); it != m_scoreResultsCache.at(threadIdx).end();) {
			std::string key = it->first;
			std::string timePoint2 = key.substr(key.find('_') + 1); // Extract the second part of the key (timePoint2) - The current timeline TimePoint
			long long timePoint2Long = Converts::ToLongLong(timePoint2); // Convert to long for comparison
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
		CleanOldResults(); // Clean old results at initialization
	}
};

class ScoreResultsCacheUtils
{
public:

	static void CacheScoreResult_InTempVector(std::vector<std::pair<std::string,ScoresResult>>& vec,const std::string& timePoint1, const std::string& timePoint2, ScoresResult& score)
	{
		vec.emplace_back(timePoint1 + "_" + timePoint2, score); // Store the score result in the vector with combined time points as key
	}
	static void CacheScoreResult_ToCache(std::vector<std::pair<std::string,ScoresResult>>& vec,int threadIdx)
	{
		ScoreResultsCache* scoreCache = ScoreResultsCache::getInstance();
		for (const auto& scoreResult : vec)
		{
			scoreCache->StoreNewScoreResult(scoreResult.first, scoreResult.second,threadIdx);
		}
	}
};
