#pragma once
#include "PatternGod_PolynomComparer.h"
#include "PatternGod_ScoreComparer.h"
#include "PatternGod_ResultFusionEngine.h"
#include "PatternGod_PolynomFilterEngine.h"
#include <chrono>

class PatternGod
{
private:
	std::vector<std::unique_ptr<PatternGod_BaseComparer>> m_comparers; // Vector to hold all comparers
	PatternGod_PolynomFilterEngine* m_polynomFilterEngine = nullptr; // Polynom filter engine for pre-filtering patterns

	std::vector<ChangeNode*>* m_historyTL;
	DBMap* m_historyDB_Map = nullptr; // Database map for the history time line
	bool m_historyDB_Owner = true; // Flag to indicate ownership of the history DB map
	std::vector<ChangeNode*>* m_currentTL;
public:

	PatternGod(std::vector<ChangeNode*>* historyTimeLine, std::vector<ChangeNode*>* currentTimeLine);

	PatternGod(std::vector<ChangeNode*>* historyTimeLine);
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
		if (m_polynomFilterEngine != nullptr)
			delete m_polynomFilterEngine; // Free the polynom filter engine
		m_polynomFilterEngine = nullptr;
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

	int ScanForOneMinPatterns_MultiThreaded(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D);
};