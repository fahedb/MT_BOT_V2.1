#include <future>
#pragma once
#include "ChangeData.h"
#include "PatternGodUtils.h"
#include "ComparersEnums.h"

typedef robin_hood::unordered_map<PatternGodComparerType, std::vector<std::pair<ChangeNode*, ScoresResult>>> PatternGodComparerResultsMap;

class PatternGod_BaseComparer
{
protected:
	bool m_isEnabled = true; // Flag to indicate if the comparer is enabled
	std::vector<ChangeNode*>* m_historyTL;
	DBMap* m_historyDB_Map = nullptr;

	std::vector<ChangeNode*>* m_currentTL = nullptr;
	DBMap* m_currDB_Map = nullptr;

	int m_scanCounter = 0; // Counter for the number of scans performed
	int m_NumOfSkips_ForLowScoreScan = 0; 
	double m_ScoreToAct = 0;
	std::vector<int> m_cacheHits; // Counter for cache hits per thread
	std::vector<int> m_cacheMisses; // Counter for cache misses per thread

	PatternGodComparerType m_comparerType = PatternGodComparerType::NONE; // Type of the comparer

	PatternGod_BaseComparer(std::vector<ChangeNode*>* history);
	PatternGod_BaseComparer(std::vector<ChangeNode*>* history, std::vector<ChangeNode*>* curr);
	int m_MaxScoreFound = 0;
public:
	~PatternGod_BaseComparer(); 

	virtual void RefreshComparer() = 0;
	
	PatternGodComparerType Get_ComparerType() const { return m_comparerType; } // Get the comparer type
	virtual int ScanForOneMinPatterns_MultiThreaded(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D);

	void Set_ScoreToAct(double scoreToAct) { m_ScoreToAct = scoreToAct; } // Set the score to act threshold
protected:
	
	virtual void ProccessScanChunk(int startIdx, int endIdx, int threadIndex, int scoreToAct,
		std::mutex* maxResultLock,
		ScoresResult* maxScore, std::vector<std::pair<ChangeNode*, ScoresResult>>* resultNodesPerThread);
protected:
	// ---------------------------------------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------------------------------------
	// Need to implement this function in derived classes
	// This function compares one minute nodes periods in the history time line

	virtual ScoresResult CompareOneMinNodesPeriods(int startIdx, int threadIdx) = 0; // Pure virtual function to be implemented by derived classes

	virtual void Set_ComparerType() = 0; // Set the comparer type, to be implemented by derived classes (m_comparerType)

	// ---------------------------------------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------------------------------------

public:
	virtual void Set_CurrentTimeLine(std::vector<ChangeNode*>* curr); // Set the current time line for comparison
	virtual bool IsScanEnabled() { return m_isEnabled; } // Check if the scan is enabled

};

