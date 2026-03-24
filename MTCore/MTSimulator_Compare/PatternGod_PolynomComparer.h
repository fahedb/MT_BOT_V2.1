#pragma once
#include "PatternGod_BaseComparer.h"

#define NUM_OF_SKIPS_FOR_LOW_SCORE_SCAN 3 // Number of skips for low score scan -- TODO: Make this configurable

class PatternGod_PolynomComparer : public PatternGod_BaseComparer
{
public:
	PatternGod_PolynomComparer(std::vector<ChangeNode*>* history, ) : PatternGod_BaseComparer(history) { InitComaprer(); }
	PatternGod_PolynomComparer(std::vector<ChangeNode*>* history, std::vector<ChangeNode*>* curr) : PatternGod_BaseComparer(history, curr) { InitComaprer(); }

	~PatternGod_PolynomComparer() {}
	virtual void RefreshComparer() override
	{
		InitComaprer(); // Reinitialize the comparer
	}
	virtual void ResetMyCacheCounters(int threadIdx) override;
private:
	std::vector<int> m_polyCacheHits; // Counter for polynomial cache hits per thread
	std::vector<int> m_polyCacheMisses; // Counter for polynomial cache misses per thread
	int m_polyLen = 5; // Length of the polynomial for comparison
	void InitComaprer();
	void InitMaxScorePossibleForTimeLine(int polyLen);
	virtual ScoresResult CompareOneMinNodesPeriods(int startIdx, int threadIdx, int totalScoreToAct); // Compare one minute nodes periods in the history time line
	
	virtual void ProccessScanChunk(int startIdx, int endIdx, int threadIndex, int totalScoreToAct
		, std::mutex* maxResultLock, ScoresResult* maxScore
		, std::vector<std::pair<ChangeNode*, ScoresResult>>* resultNodesPerThread) override;
};

