#include "ScoreResultsCache.h"
#include "Converts.h"
#include "ThreadPool.h"
ScoresResult* ScoreResultsCache::GetScoreResult(const std::string& timePoint1, const std::string& timePoint2, int threadIdx) { return GetScoreResult(Converts::ToLongLong(timePoint1), Converts::ToLongLong(timePoint2), threadIdx); }

void ScoreResultsCache::CleanTask(long long oldestTP, BaseCacheMap* cacheMap) // Implementation of the pure virtual method to clean old results
{
	for (auto it = cacheMap->begin(); it != cacheMap->end(); ) {
		long long currTimeLineTP = it->first.second; // Extract the end time point of the timeline
		if (currTimeLineTP < oldestTP) {
			it = cacheMap->erase(it); // Remove the element and get the next iterator
		}
		else
			++it; // Move to the next element
	}
}

ScoreResultsCache::ScoreResultsCache() : BaseTimeLineCache<std::pair<long long, long long>, ScoresResult>(MTThreadPool::getInstance()->Get_ThreadsNum(), 100000) {}

void ScoreResultsCacheUtils::CacheScoreResult_InTempVector(std::vector<std::pair<std::pair<long long, long long>, ScoresResult>>& vec, const long long& timePoint1, const long long& timePoint2, ScoresResult& score)
{
	vec.emplace_back(std::make_pair(timePoint1, timePoint2), score); // Store the score result in the vector with combined time points as key
}

void ScoreResultsCacheUtils::CacheScoreResult_InTempVector(std::vector<std::pair<std::pair<long long, long long>, ScoresResult>>& vec, const std::string& timePoint1, const std::string& timePoint2, ScoresResult& score)
{
	vec.emplace_back(std::make_pair(Converts::ToLongLong(timePoint1), Converts::ToLongLong(timePoint2)), score); // Store the score result in the vector with combined time points as key
}

void ScoreResultsCacheUtils::CacheScoreResult_ToCache(std::vector<std::pair<std::pair<long long, long long>, ScoresResult>>& vec, int threadIdx)
{
	ScoreResultsCache* scoreCache = ScoreResultsCache::getInstance();
	for (const auto& scoreResult : vec)
		scoreCache->StoreNewScoreResult(std::make_pair(scoreResult.first.first, scoreResult.first.second), scoreResult.second, threadIdx);
}

std::pair<long long, long long> ScoreResultsCacheUtils::GeneratePairKey(const std::string& timePoint1, const std::string& timePoint2)
{
	return std::make_pair(Converts::ToLongLong(timePoint1), Converts::ToLongLong(timePoint2)); // Create a pair of time points for caching
}
