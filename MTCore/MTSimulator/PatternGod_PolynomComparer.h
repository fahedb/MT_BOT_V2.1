#include "PatternGod_BaseComparer.h"

#pragma once
#define NUM_OF_SKIPS_FOR_LOW_SCORE_SCAN 3 // Number of skips for low score scan

class PatternGod_PolynomComparer : public PatternGod_BaseComparer
{
public:
	PatternGod_PolynomComparer(std::vector<ChangeNode*>* history) : PatternGod_BaseComparer(history) { InitComaprer(); }
	PatternGod_PolynomComparer(std::vector<ChangeNode*>* history, std::vector<ChangeNode*>* curr) : PatternGod_BaseComparer(history, curr) { InitComaprer(); }

	~PatternGod_PolynomComparer() {}

private:
	void InitComaprer()
	{
		m_NumOfSkips_ForLowScoreScan = NUM_OF_SKIPS_FOR_LOW_SCORE_SCAN;
		Set_ComparerType(); // Initialize the comparer type
	}

	virtual void Set_ComparerType() { m_comparerType = PatternGodComparerType::PolynomComparer; } // Set the comparer type
	virtual ScoresResult CompareOneMinNodesPeriods(int startIdx, int threadIdx); // Compare one minute nodes periods in the history time line
};

