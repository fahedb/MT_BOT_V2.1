#pragma once
#include "PatternGod_BaseComparer.h"

#define NUM_OF_SKIPS_FOR_LOW_SCORE_SCAN 10 // Number of skips for low score scan

class PatternGod_ScoreComparer : public PatternGod_BaseComparer
{
public:
	PatternGod_ScoreComparer(std::vector<ChangeNode*>* history) : PatternGod_BaseComparer(history) { InitComaprer(); }
	PatternGod_ScoreComparer(std::vector<ChangeNode*>* history, std::vector<ChangeNode*>* curr) : PatternGod_BaseComparer(history, curr) { InitComaprer(); }

	~PatternGod_ScoreComparer() {}
	virtual void RefreshComparer()
	{
		InitComaprer(); // Reinitialize the comparer
	}
private:
	void InitComaprer();

	virtual void Set_ComparerType() { m_comparerType = PatternGodComparerType::ScoreComparer; } // Set the comparer type
	virtual ScoresResult CompareOneMinNodesPeriods(int startIdx, int threadIdx, int totalScoreToAct); // Compare one minute nodes periods in the history time line

	virtual void ProccessScanChunk(int startIdx, int endIdx, int threadIndex, int scoreToAct
		, std::mutex* maxResultLock, ScoresResult* maxScore
		, std::vector<std::pair<ChangeNode*, ScoresResult>>* resultNodesPerThread) override;

};

