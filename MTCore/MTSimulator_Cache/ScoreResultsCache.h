#include "IndicatorStructres.h"
#include <mutex>
#include "BTimeLineCache.h"

#pragma once

/// <summary>
/// Static function for ScoreResulsCache,
/// Need to add here all the statics
/// </summary>


// Singleton class to cache score results
// This Class cache is a vector of maps, every map is a cell in the vector
// And every cell is accessable from a single thread (for writing and reading)
class ScoreResultsCache : public BaseTimeLineCache<std::pair<long long, long long>, ScoresResult>
{
private:
	long long m_lastCleanedTime = 0; // Timestamp of the last cleaning operation
	std::atomic<bool> m_cleaningInProgress = false; // Flag to indicate if cleaning is in progress
public:
	////////////////////////////////////////////////////////////////
	//////////// Getters & Setters /////////////////////////////////
	////////////////////////////////////////////////////////////////

	void StoreNewScoreResult(const std::pair<long long, long long>& timePointCombo, const ScoresResult& score, int threadIdx) { Insert(timePointCombo, score, CacheType::HISTORICAL, threadIdx); }
	ScoresResult* GetScoreResult(const std::string& timePoint1, const std::string& timePoint2, int threadIdx);
	ScoresResult* GetScoreResult(long long timePoint1, long long timePoint2, int threadIdx) { return Get(std::make_pair(timePoint1, timePoint2), CacheType::HISTORICAL, threadIdx); }
	std::vector<size_t> GetMyCacheSizes() { return GetCacheSizes(CacheType::HISTORICAL); }
	////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	////////////////////// Pure Virtual Methods ////////////////////
	////////////////////////////////////////////////////////////////

	virtual long long GetStartTP(const std::pair<long long, long long>& key) override { return key.first; } // Use the start time point of the first key as the reference	
	virtual void CleanTask(long long oldestTP, BaseCacheMap* cacheMap) override;
	////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

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

	static void CacheScoreResult_InTempVector(std::vector<std::pair<std::pair<long long, long long>, ScoresResult>>& vec, const long long& timePoint1, const long long& timePoint2, ScoresResult& score);

	static void CacheScoreResult_InTempVector(std::vector<std::pair<std::pair<long long, long long>, ScoresResult>>& vec, const std::string& timePoint1, const std::string& timePoint2, ScoresResult& score);

	static void CacheScoreResult_ToCache(std::vector<std::pair<std::pair<long long, long long>, ScoresResult>>& vec, int threadIdx);

	static std::pair<long long, long long> GeneratePairKey(const std::string& timePoint1, const std::string& timePoint2);
	
};


