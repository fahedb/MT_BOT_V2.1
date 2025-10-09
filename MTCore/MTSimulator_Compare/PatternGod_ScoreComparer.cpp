#include "PatternGod_ScoreComparer.h"
#include <iomanip>
#include <sstream>
#include "ScoreCalcs.h"
#include "Configrations.h"
#include "Logger.h"
#include <mutex>
#include <future>
#include "ScoreResultsCache.h"
#include "ThreadPool.h"
#include "MTSimulatorUtils.h"
#include "PatternGodUtils.h"

void PatternGod_ScoreComparer::InitComaprer()
{
	ScoreComparerPolicy* scoreCompPolicy = Configrations::getInstance()->Get_Policy<ScoreComparerPolicy>(PolicyType::ScoreComparerPolicy);
	m_isEnabled = scoreCompPolicy->EnableScoreScan; // Get the enabled status from the policy
	m_NumOfSkips_ForLowScoreScan = NUM_OF_SKIPS_FOR_LOW_SCORE_SCAN;
	if (m_MaxScoreFound > 0)
		m_ScoreToAct = static_cast<double>(m_MaxScoreFound - scoreCompPolicy->Diff_Between_Max_And_ScoreToAct) / static_cast<double>(m_currentTL->size()); // Set the score to act based on the max score found
	else
		m_ScoreToAct = scoreCompPolicy->ScoreToAct; // Get the score to act threshold from the policy
	Set_ComparerType(); // Initialize the comparer type
}

ScoresResult PatternGod_ScoreComparer::CompareOneMinNodesPeriods(int startIdx, int threadIdx, int totalScoreToAct) // compare between periods of time in the history time line and the whole current time line
{
	ScoreCachingPolicy* scoreCachePolicy = Configrations::getInstance()->Get_Policy<ScoreCachingPolicy>(PolicyType::ScoreCachingPolicy);
	ConfigrationsPolicy* confPolicy = Configrations::getInstance()->Get_Policy<ConfigrationsPolicy>(PolicyType::ConfigrationsPolicy);
	ScoresResult periodScore;

	int currentSize = static_cast<int>(m_currentTL->size());
	int historyRemaining = static_cast<int>(m_historyTL->size()) - startIdx;
	int limit = (currentSize < historyRemaining) ? currentSize : historyRemaining;
	int currTotalScore = 0;
	int maxScorePossible = scoreCachePolicy->maxScorePossibleForChangeNodes;
	int totalMinScoretoAct = limit * m_ScoreToAct;
	ScoreResultsCache* scoreResultsCache = ScoreResultsCache::getInstance();
	std::vector<std::pair<std::pair<long long, long long>, ScoresResult>> scoreResults;

	scoreResults.reserve(limit); // Reserve space for the expected number of results to avoid multiple reallocations
	const bool enableCaching = scoreCachePolicy->EnableScoreCaching;


	for (int i = 0; i < limit; ++i)
	{
		int remainingCandles = limit - i;
		int maxFutureScore = remainingCandles * maxScorePossible;
		if (currTotalScore + maxFutureScore < totalMinScoretoAct) // If the current total score plus the maximum possible future score is less than the minimum score to act, skip further calculations
			break;

		if (enableCaching)
		{
			ScoresResult* cachedScore = nullptr;
			try {
				cachedScore = scoreResultsCache->GetScoreResult(m_historyTL->at(static_cast<size_t>(startIdx) + static_cast<size_t>(i))->point_DateTimeLong, m_currentTL->at(i)->point_DateTimeLong, threadIdx);
			}
			catch (const std::exception& e) {
				Logger::getInstance().WriteLog("Error in getting cached result: " + std::string(e.what()), false, false, LogType::ERR);
				return ScoresResult(); // Return an empty ScoresResult on error
			}

			if (cachedScore) // If score already calculated, use it
			{
				periodScore += *cachedScore;
				currTotalScore += cachedScore->totalScore.total;
				m_cacheHits[threadIdx]++;
				continue;
			}

		}
		ScoresResult myScore;
		try {
			// Compare the change nodes and calculate the score
			// This function should be defined in ProxyScoreCalcs or similar utility class
			// It compares two ChangeNode objects and returns a ScoresResult object
			// If an error occurs, it will throw an exception which we catch below
			myScore = ProxyScoreCalcs::CompareChangeNodes(m_currentTL->at(i), m_historyTL->at(static_cast<size_t>(startIdx) + static_cast<size_t>(i)));
		}
		catch (const std::exception& e) {
			Logger::getInstance().WriteLog("Error in comparing change nodes: " + std::string(e.what()), false, false, LogType::ERR);
			return ScoresResult(); // Return an empty ScoresResult on error
		}

		periodScore += myScore;
		currTotalScore += myScore.totalScore.total;
		m_cacheMisses[threadIdx]++;
		if (enableCaching)
		{
			try {
				// Cache the score result for future insertion in the ScoreResultsCache singletone class
				ScoreResultsCacheUtils::CacheScoreResult_InTempVector(scoreResults, m_historyTL->at(static_cast<size_t>(startIdx) + static_cast<size_t>(i))->point_DateTimeLong, m_currentTL->at(i)->point_DateTimeLong, myScore);
			}
			catch (const std::exception& e) {
				Logger::getInstance().WriteLog("Error in caching score result: " + std::string(e.what()), false, false, LogType::ERR);
				return ScoresResult(); // Return an empty ScoresResult on error
			}
		}
	}

	if (enableCaching)
	{
		ScoreResultsCacheUtils::CacheScoreResult_ToCache(scoreResults, threadIdx);
	}

	return periodScore;
}

void PatternGod_ScoreComparer::ProccessScanChunk(int startIdx, int endIdx, int threadIndex, int totalScoreToAct, std::mutex* maxResultLock, ScoresResult* maxScore, std::vector<std::pair<ChangeNode*, ScoresResult>>* resultNodesPerThread)
{
	m_cacheHits[threadIndex] = 0; // Reset cache hits for this thread
	m_cacheMisses[threadIndex] = 0; // Reset cache misses for this thread
	int estimatedItems = 100;
	Logger& logger = Logger::getInstance();
	resultNodesPerThread->reserve(estimatedItems);
	int localScoreToAct = totalScoreToAct;
	ScoresResult maxLocalScore;
	const int currTlSize = m_currentTL->size(), histTlSize = m_historyTL->size();
	for (int i = startIdx; i < endIdx; i++) {
		ScoresResult resScore = PatternGod_ScoreComparer::CompareOneMinNodesPeriods(i, threadIndex, totalScoreToAct);

		if (resScore.totalScore.total > localScoreToAct) {
			if (i + currTlSize < histTlSize) {
				resultNodesPerThread->emplace_back(m_historyTL->at(i + currTlSize - 1), resScore);
			}
		}
		else
		{
			if (m_scanCounter % 8 != 0) // Every 8 scans, we do a full scan without skips to load cache better
				i += m_NumOfSkips_ForLowScoreScan;
		}
		if (maxLocalScore.totalScore.total < resScore.totalScore.total)
			maxLocalScore = resScore;

	}

	{
		std::lock_guard<std::mutex> lock(*maxResultLock);
		if (maxScore->totalScore.total < maxLocalScore.totalScore.total)
			*maxScore = maxLocalScore;
	}
	// Write logs for this thread
	logger.WriteLog("Thread [" + std::to_string(threadIndex) + "] processed indices [" + std::to_string(startIdx) + " - " + std::to_string(endIdx - 1) + "]"
		+ "\nfound " + std::to_string(resultNodesPerThread->size()) + " results"
		+ "\nmax score: " + std::to_string(maxLocalScore.totalScore.total)
		+ "\ncache hits : " + std::to_string(m_cacheHits[threadIndex])
		+ "\ncache misses : " + std::to_string(m_cacheMisses[threadIndex])
		, false, true, LogType::LOG);
}
