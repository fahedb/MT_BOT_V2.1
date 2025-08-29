#pragma once
#include "IndicatorStructres.h"
#include <mutex>
#include "Converts.h"
#include "Static_CurrentTimeLineRep.h"
#include "ResourceManager.h"
#include <cstdint>
#include "robin_hood.h"
#include <future>      // std::async, std::future
#include "HashingStructs.h"

/// <summary>
/// Static function for ScoreResultsCachePolynom,
/// </summary>


// Singleton class to cache score results
// This Class cache is a vector of maps, every map is a cell in the vector
// And every cell is accessable from a single thread (for writing and reading)
class ScoreResultsCachePolynom
{
private:
	
	long long m_lastCleanedTime = 0; // Timestamp of the last cleaning operation
	std::atomic<bool> m_cleaningInProgress = false; // Flag to indicate if cleaning is in progress
	std::vector<robin_hood::unordered_map<std::pair<PolynomTimeLineKey, PolynomTimeLineKey>, ScoresResult>> m_scoreResultsCache; // Cache to store scores by time point
public:
	void StoreNewScoreResult(const std::pair<PolynomTimeLineKey, PolynomTimeLineKey>& timePointCombo, const ScoresResult& score, int threadIdx) { m_scoreResultsCache[threadIdx].try_emplace(timePointCombo, score); }

	ScoresResult* GetScoreResult(const PolynomTimeLineKey& key1, const PolynomTimeLineKey& key2, int threadIdx);

	bool CleanPreCondition(); // Function to check if cleaning is allowed
	void CleanOldResults(); // Do not use This function in a seperated thread - Use it only in the main thread of scanning, need to freez the main thread to prevent scanning while cleaning
	static void CleanOldResultsForThread(int threadIdx, long long oldestTP, robin_hood::unordered_map<std::pair<PolynomTimeLineKey, PolynomTimeLineKey>, ScoresResult>* relevantMap); // Function to clean cache per thread

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
