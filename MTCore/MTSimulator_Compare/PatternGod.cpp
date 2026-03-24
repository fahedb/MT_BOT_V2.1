#include "PatternGod.h"
#include "LOGGER.H"
#include "CPP_To_Cuda_Tunnel.h"
#include "ScopedPerfTimer.h"
#include "TimeLineBuilder.h"
#include "PatternGodUtils.h"

PatternGod::PatternGod(std::vector<ChangeNode*>* historyTimeLine, std::vector<ChangeNode*>* currentTimeLine, const std::string& scanFilters, const int polyLen, bool EnableScoreCaching, double wEval, double wRMSE, double wCoeff)
{
	m_historyTL = historyTimeLine;
	m_currentTL = currentTimeLine;

	m_historyDB_Map = new DBMap(); // Initialize the history DB map
	TimeLineBuilder::Build_TimeLineDB(m_historyTL, m_historyDB_Map); // Build the DB map for the history time line
	m_historyDB_Owner = true;
	InitComparers(scanFilters, polyLen, EnableScoreCaching, wEval, wRMSE, wCoeff); // Initialize the comparers with the history time line

}

void PatternGod::InitComparers(const std::string& scanFilters, const int polyLen, bool EnableScoreCaching, double wEval, double wRMSE, double wCoeff) // Initialize the comparers with the history time line
{
	if (!m_polynomFilterEngine)
		m_polynomFilterEngine = new PatternGod_PolynomFilterEngine(m_historyTL, polyLen, EnableScoreCaching, wEval, wRMSE, wCoeff);
	if (!m_scoreComparer)
		m_scoreComparer = new PatternGod_ScoreComparer(m_historyTL, m_currentTL, scanFilters);
	// *** Add more comparers as needed ***
}

bool PatternGod::ScanPreCondition() const
{
	if (m_historyTL == nullptr || m_historyTL->empty())
	{
		Logger::getInstance().WriteLog("History time line is not set or is empty.", true, true, LogType::ERR);
		return false; // Return false if the history time line is not set or is empty
	}

	if (m_currentTL == nullptr || m_currentTL->empty())
	{
		Logger::getInstance().WriteLog("Current time line is not set or is empty.", true, true, LogType::ERR);
		return false; // Return false if the current time line is not set or is empty
	}
	if(m_scoreComparer == nullptr)
	{
		Logger::getInstance().WriteLog("Score comparer is not initialized.", true, true, LogType::ERR);
		return false; // Return false if the score comparer is not initialized
	}
	if(m_polynomFilterEngine == nullptr)
	{
		Logger::getInstance().WriteLog("Polynom filter engine is not initialized.", true, true, LogType::ERR);
		return false; // Return false if the polynom filter engine is not initialized
	}
	return true; // Return true if both time lines are valid
}

int PatternGod::ScanForOneMinPatterns(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D)
{
	if (!ScanPreCondition())
		return 0; // Return 0 if preconditions for scanning are not met

	int totalScans = 0;
	ScopedPerfTimer scoringPerf("PatternGod - ScanForOneMinPatterns_MultiThreaded"); // Performance timer for the entire scanning process
	std::vector<std::pair<ChangeNode*, ScoresResult>> scoringResults;
	
	ScoringEngineScan(scoringResults); // Run the scoring engine scan to get initial scoring results
	PolynomEngineFilter(scoringResults); // Run the polynom filter engine to pre-filter the scoring results based on patterns
	resultNodes2D = scoringResults;
	return totalScans; // Return the total number of scans performed
}

int PatternGod::ScoringEngineScan(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D)
{
	m_scoreComparer->Set_NewCurrentTimeLine(m_currentTL); // Update current time line in score comparer before scanning
	m_scoreComparer->RunScan(resultNodes2D); // Run the score comparer to get scoring results for the current time line
	return resultNodes2D.size(); // Return the number of scored patterns found
}

int PatternGod::PolynomEngineFilter(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D)
{
	if (!resultNodes2D.size())
	{
		Logger::getInstance().WriteLog("Results vector is empty, No filter is applied.", false, true, LogType::ERR);
		return 0;
	}

	std::vector<ChangeNode*>heads = TimeLineBuilder::SplitChangeNodeFromScores(&resultNodes2D);

	auto tempVec = m_polynomFilterEngine->ProcessPatterns(
		&heads,
		m_currentTL->size());
	PatternGodUtils::filterByThresholdScore(tempVec, 0.98, 20);
	PatternGodUtils::filterByHighScores(tempVec, 20);
	resultNodes2D = tempVec;
}