#include "PatternGod_PolynomComparer.h"

void PatternGod_PolynomComparer::InitComaprer()
{
	m_isEnabled = Configrations::getInstance()->Get_Policy<PolynomComparerPolicy>(PolicyType::PolynomComparerPolicy)->EnablePolynomScan; // Get the enabled status from the policy
	m_NumOfSkips_ForLowScoreScan = NUM_OF_SKIPS_FOR_LOW_SCORE_SCAN;
	Set_ComparerType(); // Initialize the comparer type
	m_ScoreToAct = Configrations::getInstance()->Get_Policy<PolynomComparerPolicy>(PolicyType::PolynomComparerPolicy)->ScoreToAct; // Get the score to act threshold from the policy
	m_polyCacheHits.resize(MTThreadPool::getInstance()->Get_ThreadsNum(), 0); // Initialize polynomial cache hits for each thread
	m_polyCacheMisses.resize(MTThreadPool::getInstance()->Get_ThreadsNum(), 0); // Initialize polynomial cache misses for each thread
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

	periodScore = PolynomScoreCalcs::CompareTimeLines(&histVec, m_currentTL, threadIdx, m_cacheHits[threadIdx], m_cacheMisses[threadIdx], m_polyCacheHits[threadIdx], m_polyCacheMisses[threadIdx]); // Calculate the score by comparing the historical vector with the current time line

	return periodScore;
}


void PatternGod_PolynomComparer::ProccessScanChunk(int startIdx, int endIdx, int threadIndex, int scoreToAct, std::mutex* maxResultLock, ScoresResult* maxScore, std::vector<std::pair<ChangeNode*, ScoresResult>>* resultNodesPerThread)
{
	Logger::getInstance().WriteLog("Thread [" + std::to_string(threadIndex) + "] started processing indices [" + std::to_string(startIdx) + " - " + std::to_string(endIdx - 1) + "]", false, false, LogType::LOG);
	m_cacheHits[threadIndex] = 0; // Reset cache hits for this thread
	m_cacheMisses[threadIndex] = 0; // Reset cache misses for this thread
	m_polyCacheHits[threadIndex] = 0; // Reset poly cache hits for this thread
	m_polyCacheMisses[threadIndex] = 0; // Reset poly cache misses for this thread
	int estimatedItems = 20;
	Logger& logger = Logger::getInstance();
	resultNodesPerThread->reserve(estimatedItems);
	int localScoreToAct = scoreToAct;
	ScoresResult maxLocalScore;
	const int currTlSize = m_currentTL->size(), histTlSize = m_historyTL->size();
	for (int i = startIdx; i < endIdx; i++) {
		ScoresResult resScore = PatternGod_PolynomComparer::CompareOneMinNodesPeriods(i, threadIndex);

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
		+ "\npoly cache hits : " + std::to_string(m_polyCacheHits[threadIndex])
		+ "\npoly cache misses : " + std::to_string(m_polyCacheMisses[threadIndex])
		, false, false, LogType::LOG);
}
