#pragma once
#include "IndicatorStructres.h"
#include <mutex>
#include "Static_CurrentTimeLineRep.h"
#include <cstdint>

#include "..\MyRobinHoodMap\robin_hood.h"
#include "..\MyRobinHoodMap\HashingStructs.h"

#include <future>      // std::async, std::future
#include "BTimeLineCache.h"

/// <summary>
/// Static function for ScoreResultsCachePolynom,
/// </summary>


// Singleton class to cache score results
// This Class cache is a vector of maps, every map is a cell in the vector
// And every cell is accessable from a single thread (for writing and reading)
class ScoreResultsCachePolynom : public BaseTimeLineCache<std::pair<PolynomTimeLineKey, PolynomTimeLineKey>, ScoresResult>
{
private:

public:
	bool StoreNewScoreResult(const std::pair<PolynomTimeLineKey, PolynomTimeLineKey>& timePointCombo, const ScoresResult& score, int threadIdx) 
	{ return Insert(timePointCombo, score, CacheType::HISTORICAL, threadIdx); }

	ScoresResult* GetScoreResult(const PolynomTimeLineKey& key1, const PolynomTimeLineKey& key2, int threadIdx) 
	{ return Get(std::make_pair(key1, key2), CacheType::HISTORICAL, threadIdx); }

	virtual long long GetStartTP(const std::pair<PolynomTimeLineKey, PolynomTimeLineKey>& key) override { return key.first.startTimePoint; } // Use the start time point of the first key as the reference
	virtual void CleanTask(long long oldestTP, BaseCacheMap* relevantMap) override; // Implementation of the pure virtual method to clean old results
public:

	ScoreResultsCachePolynom(const ScoreResultsCachePolynom&) = delete;
	ScoreResultsCachePolynom& operator=(const ScoreResultsCachePolynom&) = delete;
	ScoreResultsCachePolynom(ScoreResultsCachePolynom&&) = delete;
	ScoreResultsCachePolynom& operator=(ScoreResultsCachePolynom&&) = delete;

	// Static method to get the instance of the Singleton
	static ScoreResultsCachePolynom* getInstance() {
		static ScoreResultsCachePolynom instance; // Guaranteed to be destroyed and instantiated on first use
		return &instance;
	}
private:
	ScoreResultsCachePolynom();

};
