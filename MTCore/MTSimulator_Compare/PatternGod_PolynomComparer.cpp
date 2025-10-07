#include "PatternGod_PolynomComparer.h"
#include "CONFIGRATIONS.H"
#include "ThreadPool.h"
#include "PolynomScoreCalcs.h"
#include "Logger.h"

void PatternGod_PolynomComparer::ResetMyCacheCounters(int threadIdx)
{
	PatternGod_BaseComparer::ResetMyCacheCounters(threadIdx); // Call base class method to reset base cache counters
	if (threadIdx >= 0 && threadIdx < m_polyCacheHits.size())
	{
		m_polyCacheHits[threadIdx] = 0; // Reset polynomial cache hits for this thread
		m_polyCacheMisses[threadIdx] = 0; // Reset polynomial cache misses for this thread
	}
}

void PatternGod_PolynomComparer::InitComaprer()
{
	PolynomComparerPolicy* polyCompPolicy = Configrations::getInstance()->Get_Policy<PolynomComparerPolicy>(PolicyType::PolynomComparerPolicy);
	m_isEnabled = polyCompPolicy->EnablePolynomScan; // Get the enabled status from the policy
	m_NumOfSkips_ForLowScoreScan = NUM_OF_SKIPS_FOR_LOW_SCORE_SCAN;
	Set_ComparerType(); // Initialize the comparer type
	if (m_MaxScoreFound > 0)
		m_ScoreToAct = static_cast<double>(m_MaxScoreFound - 250) / static_cast<double>(m_currentTL->size());
	else
		m_ScoreToAct = polyCompPolicy->ScoreToAct; // Get the score to act threshold from the policy

	m_polyCacheHits.resize(MTThreadPool::getInstance()->Get_ThreadsNum(), 0); // Initialize polynomial cache hits for each thread
	m_polyCacheMisses.resize(MTThreadPool::getInstance()->Get_ThreadsNum(), 0); // Initialize polynomial cache misses for each thread
	m_polyLen = polyCompPolicy->PolynomLen_ForScan; // Get the polynomial length from the policy
}

void PatternGod_PolynomComparer::InitMaxScorePossibleForTimeLine(int polyLen)
{
	if (m_MaxScorePossibleForPolynom == 0)
	{
		std::vector<ChangeNode*> vec(
			m_currentTL->begin(),
			m_currentTL->begin() + polyLen // Use up to the first 10 nodes for estimation
		);
		PolynomChangeNodePackage vecPackage(vec);

		TimeLinePolynomScore maxScorePossible = PolynomScoreCalcs::CalcScore(&vecPackage, &vecPackage);
		ScoresResult scoreRes;
		scoreRes += *maxScorePossible.GetScoreContainer(IndicatorValueType::Value);
		scoreRes += *maxScorePossible.GetScoreContainer(IndicatorValueType::Slope);
		m_MaxScorePossibleForPolynom = scoreRes.totalScore.total;
	}
}

ScoresResult PatternGod_PolynomComparer::CompareOneMinNodesPeriods(int startIdx, int threadIdx, int totalScoreToAct)
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

	periodScore = PolynomScoreCalcs::CompareTimeLines(&histVec, m_currentTL, threadIdx, m_cacheHits[threadIdx], m_cacheMisses[threadIdx], m_polyCacheHits[threadIdx], m_polyCacheMisses[threadIdx], m_MaxScorePossibleForPolynom, totalScoreToAct); // Calculate the score by comparing the historical vector with the current time line

	return periodScore;
}


void PatternGod_PolynomComparer::ProccessScanChunk(int startIdx, int endIdx, int threadIndex, int totalScoreToAct, std::mutex* maxResultLock, ScoresResult* maxScore, std::vector<std::pair<ChangeNode*, ScoresResult>>* resultNodesPerThread)
{
	Logger::getInstance().WriteLog("Thread [" + std::to_string(threadIndex) + "] started processing indices [" + std::to_string(startIdx) + " - " + std::to_string(endIdx - 1) + "]", false, false, LogType::LOG);
	ResetMyCacheCounters(threadIndex); // Reset cache counters for this thread
	InitMaxScorePossibleForTimeLine(m_polyLen); // Initialize the maximum possible score for the time line based on the polynomial length

	int estimatedItems = 20;
	Logger& logger = Logger::getInstance();
	resultNodesPerThread->reserve(estimatedItems);
	int localScoreToAct = totalScoreToAct;
	ScoresResult maxLocalScore;
	const int currTlSize = m_currentTL->size(), histTlSize = m_historyTL->size();
	for (int i = startIdx; i < endIdx; i++) {
		ScoresResult resScore = PatternGod_PolynomComparer::CompareOneMinNodesPeriods(i, threadIndex, totalScoreToAct);

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
