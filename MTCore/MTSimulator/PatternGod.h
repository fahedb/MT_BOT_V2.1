#pragma once

#include "PatternGod_PolynomComparer.h"
#include "PatternGod_ScoreComparer.h"
#include "PatternGod_ResultFusionEngine.h"

class PatternGod
{
private:
	std::vector<std::unique_ptr<PatternGod_BaseComparer>> m_comparers; // Vector to hold all comparers

	std::vector<ChangeNode*>* m_historyTL;
	std::vector<ChangeNode*>* m_currentTL;
public:

	PatternGod(std::vector<ChangeNode*>* historyTimeLine, std::vector<ChangeNode*>* currentTimeLine)
	{
		m_historyTL = historyTimeLine;
		m_currentTL = currentTimeLine;
	}

	PatternGod(std::vector<ChangeNode*>* historyTimeLine)
	{
		m_historyTL = historyTimeLine;
		m_currentTL = nullptr; // Initialize current time line as nullptr
	}
	PatternGod(TimeLine* historyTLP)
	{
		m_currentTL = nullptr;
		m_historyTL = historyTLP->Get_TimeLine();
	}
	~PatternGod()
	{

	}
	void InitComparers(); // Initialize the comparers with the history time line
	void Set_CurrentTimeLine(std::vector<ChangeNode*>* currentTimeLine) { for (auto& comparer : m_comparers) comparer->Set_CurrentTimeLine(currentTimeLine); }
	bool ScanPreCondition() const;

	// TODO: Implement the scanning logic for patterns through the comparers...

	int ScanForOneMinPatterns_MultiThreaded(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D)
	{
		if (!ScanPreCondition())
			return 0; // Return 0 if preconditions for scanning are not met

		if (m_comparers.empty())
			InitComparers(); // Initialize comparers if not already done

		int totalScans = 0;
		

		robin_hood::unordered_map<PatternGodComparerType, std::vector<std::pair<ChangeNode*, ScoresResult>>> resultPerComparer; // Map to hold results for each comparer
		
		int idx = 0;
		for (auto& comparer : m_comparers)
		{
			resultPerComparer[comparer->Get_ComparerType()] = std::vector<std::pair<ChangeNode*, ScoresResult>>(); // Initialize the result vector for each comparer

			totalScans += comparer->ScanForOneMinPatterns_MultiThreaded(resultPerComparer[comparer->Get_ComparerType()]); // Scan for patterns using each comparer
			idx++;
		}

		// Need to merge results using ResultsFusion Engine...
		resultNodes2D = PatternGod_ResultFusionEngine::FuseResults(&resultPerComparer); // Fuse results from all comparers

		return totalScans; // Return the total number of scans performed
	}
};

