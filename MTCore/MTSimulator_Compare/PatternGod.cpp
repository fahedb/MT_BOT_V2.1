#include "PatternGod.h"
#include "TimeLineBuilder.h"
#include "LOGGER.H"

PatternGod::PatternGod(std::vector<ChangeNode*>* historyTimeLine, std::vector<ChangeNode*>* currentTimeLine)
{
	m_historyTL = historyTimeLine;
	m_currentTL = currentTimeLine;

	m_historyDB_Map = new DBMap(); // Initialize the history DB map
	TimeLineBuilder::Build_TimeLineDB(m_historyTL, m_historyDB_Map); // Build the DB map for the history time line
	m_historyDB_Owner = true;
	InitComparers(); // Initialize the comparers with the history time line

}

PatternGod::PatternGod(std::vector<ChangeNode*>* historyTimeLine)
{
	m_historyTL = historyTimeLine;
	m_currentTL = nullptr; // Initialize current time line as nullptr

	m_historyDB_Map = new DBMap(); // Initialize the history DB map
	TimeLineBuilder::Build_TimeLineDB(m_historyTL, m_historyDB_Map); // Build the DB map for the history time line
	m_historyDB_Owner = true; // Set ownership of the history DB map
	InitComparers(); // Initialize the comparers with the history time line
}

void PatternGod::InitComparers() // Initialize the comparers with the history time line
{
	m_comparers.push_back(std::make_unique<PatternGod_PolynomComparer>(m_historyTL));
	m_comparers.push_back(std::make_unique<PatternGod_ScoreComparer>(m_historyTL));
	
	// *** Add more comparers as needed ***

	if (m_currentTL)
		Set_CurrentTimeLine(m_currentTL); // Set the current time line for all comparers if it exists
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
	return true; // Return true if both time lines are valid
}

int PatternGod::ScanForOneMinPatterns_MultiThreaded(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D)
{
	if (!ScanPreCondition())
		return 0; // Return 0 if preconditions for scanning are not met

	if (m_comparers.empty())
		InitComparers(); // Initialize comparers if not already done

	int totalScans = 0;


	PatternGodComparerResultsMap resultPerComparer; // Map to hold results for each comparer
	auto start = std::chrono::high_resolution_clock::now();
	for (const auto& comparer : m_comparers)
	{
		resultPerComparer[comparer->Get_ComparerType()] = std::vector<std::pair<ChangeNode*, ScoresResult>>(); // Initialize the result vector for each comparer
		if (comparer->IsScanEnabled())
			totalScans += comparer->ScanForOneMinPatterns_MultiThreaded(resultPerComparer[comparer->Get_ComparerType()]); // Scan for patterns using each comparer
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	Logger::getInstance().WriteLog("Performance: Total Scan Time: " + std::to_string(duration) + " ms", false, true, LogType::LOG);
	// Need to merge results using ResultsFusion Engine...
	resultNodes2D = PatternGod_ResultFusionEngine::FuseResults(resultPerComparer, m_historyDB_Map); // Fuse results from all comparers

	return totalScans; // Return the total number of scans performed
}
