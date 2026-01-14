#include "ScoreResultsCachePolynom.h"
#include "ThreadPool.h"
#include "Configrations.h"
#include "Converts.h"

// Constructor implementation
ScoreResultsCachePolynom::ScoreResultsCachePolynom() 
	: BaseTimeLineCache<std::pair<PolynomTimeLineKey, PolynomTimeLineKey>, ScoresResult>(
		MTThreadPool::getInstance()->Get_ThreadsNum(),
		Configrations::getInstance()->Get_Policy<ScoreCachingPolicy>(PolicyType::ScoreCachingPolicy)->EnableScoreCaching ? Configrations::getInstance()->Get_Policy<ScoreCachingPolicy>(PolicyType::ScoreCachingPolicy)->ScoreResultsCacheSizePerThread : 0)
{
	// Constructor body can be empty as base class handles initialization
}

// Implementation of pure virtual method from BaseTimeLineCache
void ScoreResultsCachePolynom::CleanTask(long long oldestTP, BaseCacheMap* relevantMap)
{
	for (auto it = relevantMap->begin(); it != relevantMap->end(); ) {
		long long timePoint2Long = it->first.second.endTimePoint; // Extract the second part of the key (timePoint2) - The current timeline TimePoint

		if (timePoint2Long < oldestTP) {
			it = relevantMap->erase(it); // Remove the element and get the next iterator
		}
		else
			++it; // Move to the next element
	}
}

