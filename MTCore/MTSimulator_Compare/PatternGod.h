#pragma once
#include "PatternGod_ScoreComparer.h"
#include "PatternGod_PolynomFilterEngine.h"
#include "TimeLine.h"

#include <chrono>

class PatternGod
{
private:
	bool m_cudaEngineInitialized = false; // Flag to indicate if the CUDA engine is initialized

	PatternGod_ScoreComparer* m_scoreComparer = nullptr; // Score comparer for scoring patterns
	PatternGod_PolynomFilterEngine* m_polynomFilterEngine = nullptr; // Polynom filter engine for pre-filtering patterns

	std::vector<ChangeNode*>* m_historyTL;
	DBMap* m_historyDB_Map = nullptr; // Database map for the history time line
	bool m_historyDB_Owner = true; // Flag to indicate ownership of the history DB map
	std::vector<ChangeNode*>* m_currentTL;
public:

	PatternGod(std::vector<ChangeNode*>* historyTimeLine, std::vector<ChangeNode*>* currentTimeLine, const std::string& scanFilters, const int polyLen, bool EnableScoreCaching, double wEval, double wRMSE, double wCoeff);

	~PatternGod()
	{
		if (m_historyDB_Map != nullptr && m_historyDB_Owner)
			delete m_historyDB_Map; // Free the history DB map
		if (m_polynomFilterEngine != nullptr)
			delete m_polynomFilterEngine; // Free the polynom filter engine
		m_polynomFilterEngine = nullptr;
	}

	int ScanForOneMinPatterns(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D);
	void Set_CurrentTimeLine(std::vector<ChangeNode*>* currentTimeLine) { 
		m_currentTL = currentTimeLine;	
	} // Update the current time line

private:
	void InitComparers(const std::string& scanFilters, const int polyLen, bool EnableScoreCaching, double wEval, double wRMSE, double wCoeff); // Initialize the comparers with the history time line
	bool ScanPreCondition() const;
	int ScoringEngineScan(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D);
	int PolynomEngineFilter(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D);
};