#pragma once

#include "PatternGod_PolynomComparer.h"
#include "PatternGod_ScoreComparer.h"
#include "PatternGod_ResultFusionEngine.h"

class PatternGod
{
private:
	std::vector<std::unique_ptr<PatternGod_BaseComparer>> m_comparers; // Vector to hold all comparers

	std::vector<ChangeNode*>* m_historyTL;
	DBMap* m_historyDB_Map = nullptr; // Database map for the history time line
	bool m_historyDB_Owner = true; // Flag to indicate ownership of the history DB map
	std::vector<ChangeNode*>* m_currentTL;
public:

	PatternGod(std::vector<ChangeNode*>* historyTimeLine, std::vector<ChangeNode*>* currentTimeLine)
	{
		m_historyTL = historyTimeLine;
		m_currentTL = currentTimeLine;

		m_historyDB_Map = new DBMap(); // Initialize the history DB map
		TimeLineBuilder::Build_TimeLineDB(m_historyTL,m_historyDB_Map); // Build the DB map for the history time line
		m_historyDB_Owner = true;
		InitComparers(); // Initialize the comparers with the history time line

	}

	PatternGod(std::vector<ChangeNode*>* historyTimeLine)
	{
		m_historyTL = historyTimeLine;
		m_currentTL = nullptr; // Initialize current time line as nullptr

		m_historyDB_Map = new DBMap(); // Initialize the history DB map
		TimeLineBuilder::Build_TimeLineDB(m_historyTL, m_historyDB_Map); // Build the DB map for the history time line
		m_historyDB_Owner = true; // Set ownership of the history DB map
		InitComparers(); // Initialize the comparers with the history time line
	}
	PatternGod(TimeLine* historyTLP)
	{
		m_currentTL = nullptr;
		m_historyTL = historyTLP->Get_TimeLine();
		m_historyDB_Map = historyTLP->Get_DBMap();
		m_historyDB_Owner = false; // Do not own the history DB map, it is managed by the TimeLine object
		InitComparers(); // Initialize the comparers with the history time line
	}
	~PatternGod()
	{
		if (m_historyDB_Map != nullptr && m_historyDB_Owner)
			delete m_historyDB_Map; // Free the history DB map
	}
	void RefreshComparers()
	{
		for (auto& comparer : m_comparers)
		{
			comparer->RefreshComparer(); // Reinitialize the comparer
		}
	}
	void InitComparers(); // Initialize the comparers with the history time line
	void Set_CurrentTimeLine(std::vector<ChangeNode*>* currentTimeLine) { 
		for (auto& comparer : m_comparers) comparer->Set_CurrentTimeLine(currentTimeLine);
		m_currentTL = currentTimeLine; // Set the current time line for patternGod
	}
	bool ScanPreCondition() const;

	// TODO: Implement the scanning logic for patterns through the comparers...

	int ScanForOneMinPatterns_MultiThreaded(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D)
	{
		if (!ScanPreCondition())
			return 0; // Return 0 if preconditions for scanning are not met

		if (m_comparers.empty())
			InitComparers(); // Initialize comparers if not already done

		int totalScans = 0;
		

		PatternGodComparerResultsMap resultPerComparer; // Map to hold results for each comparer
		
		for (const auto& comparer : m_comparers)
		{
			resultPerComparer[comparer->Get_ComparerType()] = std::vector<std::pair<ChangeNode*, ScoresResult>>(); // Initialize the result vector for each comparer
			if (comparer->IsScanEnabled())
				totalScans += comparer->ScanForOneMinPatterns_MultiThreaded(resultPerComparer[comparer->Get_ComparerType()]); // Scan for patterns using each comparer
		}

		// Need to merge results using ResultsFusion Engine...
		resultNodes2D = PatternGod_ResultFusionEngine::FuseResults(resultPerComparer,m_historyDB_Map); // Fuse results from all comparers

		return totalScans; // Return the total number of scans performed
	}
};

