#include "PatternGod_PolynomComparer.h"

void PatternGod_PolynomComparer::InitComaprer()
{
	m_NumOfSkips_ForLowScoreScan = NUM_OF_SKIPS_FOR_LOW_SCORE_SCAN;
	Set_ComparerType(); // Initialize the comparer type
	m_ScoreToAct = Configrations::getInstance()->Get_Policy<PolynomComparerPolicy>(PolicyType::PolynomComparerPolicy)->ScoreToAct; // Get the score to act threshold from the policy
}

ScoresResult PatternGod_PolynomComparer::CompareOneMinNodesPeriods(int startIdx, int threadIdx)
{
	int currentSize = static_cast<int>(m_currentTL->size());
	
	if (startIdx + currentSize > m_historyTL->size())  // Check if the start index plus the current size exceeds the history time line size
		return ScoresResult();

	std::vector<ChangeNode*> histVec(
		m_historyTL->begin() + startIdx,
		m_historyTL->begin() + startIdx + currentSize
	);

	ScoresResult periodScore;
	periodScore.Reset(); // Reset the score to ensure it's empty before calculation

	periodScore = PolynomScoreCalcs::CompareTimeLines(&histVec, m_currentTL, threadIdx); // Calculate the score by comparing the historical vector with the current time line

	return periodScore;
}
