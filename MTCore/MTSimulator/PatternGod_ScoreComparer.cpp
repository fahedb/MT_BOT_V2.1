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

ScoresResult PatternGod_ScoreComparer::CompareOneMinNodesPeriods(int startIdx, int threadIdx) // compare between periods of time in the history time line and the whole current time line
{
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
	ScoresResult periodScore;

	int currentSize = static_cast<int>(m_currentTL->size());
	int historyRemaining = static_cast<int>(m_historyTL->size()) - startIdx;
	int limit = (currentSize < historyRemaining) ? currentSize : historyRemaining;
	int currTotalScore = 0;
	int maxScorePossible = policy->maxScorePossibleForChangeNodes;
	int totalMinScoretoAct = limit * policy->Score_Min_TotalScore_ToAct_PerCandle;
	ScoreResultsCache* scoreResultsCache = ScoreResultsCache::getInstance();
	std::vector<std::pair<std::pair<long long, long long>, ScoresResult>> scoreResults;

	scoreResults.reserve(limit); // Reserve space for the expected number of results to avoid multiple reallocations
	const bool enableCaching = policy->EnableScoreCaching;


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