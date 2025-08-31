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

	long long m_lastCleanedTime = 0; // Timestamp of the last cleaning operation
	std::atomic<bool> m_cleaningInProgress = false; // Flag to indicate if cleaning is in progress
	std::vector<std::vector<robin_hood::unordered_map<std::pair<PolynomTimeLineKey, PolynomTimeLineKey>, ScoresResult>>> m_scoreResultsCache; // Cache to store scores - one map per thread and month idx
public:
	void StoreNewScoreResult(const std::pair<PolynomTimeLineKey, PolynomTimeLineKey>& timePointCombo, const ScoresResult& score, int threadIdx) { 
		int monthIdx = ExtractMonth( timePointCombo.first.startTimePoint);
		m_scoreResultsCache[threadIdx].at(monthIdx).try_emplace(timePointCombo, score);
	}

	ScoresResult* GetScoreResult(const PolynomTimeLineKey& key1, const PolynomTimeLineKey& key2, int threadIdx);

	bool CleanPreCondition(); // Function to check if cleaning is allowed
	void CleanOldResults(bool forceSynchronous = false); // Do not use This function in a seperated thread - Use it only in the main thread of scanning, need to freez the main thread to prevent scanning while cleaning
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
