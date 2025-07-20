#include "IndicatorStructres.h"
#include <mutex>
#include "Converts.h"
#include "Static_CurrentTimeLineRep.h"
#include "ResourceManager.h"
#include <cstdint>
#include "robin_hood.h"
#include <future>      // std::async, std::future

#pragma once

/// <summary>
/// Static function for ScoreResulsCache,
/// Need to add here all the statics
/// </summary>

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

// Singleton class to cache score results
// This Class cache is a vector of maps, every map is a cell in the vector
// And every cell is accessable from a single thread (for writing and reading)
class ScoreResultsCache 
{
private:
	long long m_lastCleanedTime = 0; // Timestamp of the last cleaning operation
	std::atomic<bool> m_cleaningInProgress = false; // Flag to indicate if cleaning is in progress
	std::vector<robin_hood::unordered_map<std::pair<long long, long long>, ScoresResult>> m_scoreResultsCache; // Cache to store scores by time point
public:
	void StoreNewScoreResult(const std::pair<long long, long long>& timePointCombo, const ScoresResult& score, int threadIdx) { m_scoreResultsCache[threadIdx].try_emplace(timePointCombo, score); }

	ScoresResult* GetScoreResult(const std::string& timePoint1, const std::string& timePoint2, int threadIdx);
	ScoresResult* GetScoreResult(long long timePoint1, long long timePoint2, int threadIdx);
	
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
			scoreCache->StoreNewScoreResult(scoreResult.first, scoreResult.second, threadIdx);
	}

	static std::pair<long long, long long> GeneratePairKey(const std::string& timePoint1, const std::string& timePoint2)
	{
		return std::make_pair(Converts::ToLongLong(timePoint1), Converts::ToLongLong(timePoint2)); // Create a pair of time points for caching
	}
};


