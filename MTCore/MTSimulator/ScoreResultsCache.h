#include "IndicatorStructres.h"
#include <mutex>
#include "Converts.h"
#include "Static_CurrentTimeLineRep.h"
#include "ResourceManager.h"
#include <cstdint>
#include "robin_hood.h"
#include <future>      // std::async, std::future
#include "HashingStructs.h"

#pragma once

/// <summary>
/// Static function for ScoreResulsCache,
/// Need to add here all the statics
/// </summary>


// Singleton class to cache score results
// This Class cache is a vector of maps, every map is a cell in the vector
// And every cell is accessable from a single thread (for writing and reading)
class ScoreResultsCache 
{
private:
	long long m_lastCleanedTime = 0; // Timestamp of the last cleaning operation
	std::atomic<bool> m_cleaningInProgress = false; // Flag to indicate if cleaning is in progress
	std::vector<std::vector<robin_hood::unordered_map<std::pair<long long, long long>, ScoresResult>>> m_scoreResultsCache; // Cache to store scores by time point
	int ExtractMonth(long long tp) {
		// Better month extraction for timestamp format YYYYMMDDHHMMSSSSS
		// Extract month from position 5-6 (0-based)
		int month = ((tp / 100000000000LL) % 100);
		// Ensure month is in valid range 1-12, then convert to 0-11
		if (month < 1 || month > 12) {
			Logger::getInstance().WriteLog("Invalid month extracted: " + std::to_string(month) + " from timestamp: " + std::to_string(tp), false, true, LogType::ERR);
			return 0; // Default to January if invalid
		}
		return month - 1; // Convert to 0-based index
	}
public:
	void StoreNewScoreResult(const std::pair<long long, long long>& timePointCombo, const ScoresResult& score, int threadIdx) { 
		if (timePointCombo.first == 20221201070500000 && timePointCombo.second == 1790)
			int k = 0;
		int monthIdx = ExtractMonth(timePointCombo.first);
		// Bounds checking
		if (threadIdx < 0 || threadIdx >= m_scoreResultsCache.size()) {
			Logger::getInstance().WriteLog("Invalid threadIdx: " + std::to_string(threadIdx), false, true, LogType::ERR);
			return;
		}
		if (monthIdx < 0 || monthIdx >= m_scoreResultsCache[threadIdx].size()) {
			Logger::getInstance().WriteLog("Invalid monthIdx: " + std::to_string(monthIdx) + " for thread: " + std::to_string(threadIdx), false, true, LogType::ERR);
			return;
		}
		m_scoreResultsCache[threadIdx].at(monthIdx).try_emplace(timePointCombo, score);
	}

	ScoresResult* GetScoreResult(const std::string& timePoint1, const std::string& timePoint2, int threadIdx);
	ScoresResult* GetScoreResult(long long timePoint1, long long timePoint2, int threadIdx);
	
	std::vector<size_t> GetCacheSizes();
	size_t GetCacheSize(int threadIdx);

	bool CleanPreCondition(); // Function to check if cleaning is allowed
	void CleanOldResults(); // Do not use This function in a seperated thread - Use it only in the main thread of scanning, need to freez the main thread to prevent scanning while cleaning
	static void CleanOldResultsForThread(int threadIdx, long long oldestTP, robin_hood::unordered_map<std::pair<long long, long long>, ScoresResult>* relevantMap); // Function to clean cache per thread

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
	ScoreResultsCache();

};

class ScoreResultsCacheUtils
{
public:

	static void CacheScoreResult_InTempVector(std::vector<std::pair<std::pair<long long, long long>, ScoresResult>>& vec, const long long& timePoint1, const long long& timePoint2, ScoresResult& score)
	{
		vec.emplace_back(std::make_pair(timePoint1, timePoint2), score); // Store the score result in the vector with combined time points as key
	}

	static void CacheScoreResult_InTempVector(std::vector<std::pair<std::pair<long long, long long>, ScoresResult>>& vec, const std::string& timePoint1, const std::string& timePoint2, ScoresResult& score)
	{
		vec.emplace_back(std::make_pair(Converts::ToLongLong(timePoint1), Converts::ToLongLong(timePoint2)), score); // Store the score result in the vector with combined time points as key
	}

	static void CacheScoreResult_ToCache(std::vector<std::pair<std::pair<long long, long long>, ScoresResult>>& vec, int threadIdx)
	{
		ScoreResultsCache* scoreCache = ScoreResultsCache::getInstance();
		for (const auto& scoreResult : vec)
			scoreCache->StoreNewScoreResult(std::make_pair(scoreResult.first.first,scoreResult.first.second), scoreResult.second, threadIdx);
	}

	static std::pair<long long, long long> GeneratePairKey(const std::string& timePoint1, const std::string& timePoint2)
	{
		return std::make_pair(Converts::ToLongLong(timePoint1), Converts::ToLongLong(timePoint2)); // Create a pair of time points for caching
	}
	
};


