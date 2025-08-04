#include "ChangeData.h"
#include <future>
#include "MTSimulatorUtils.h"
#include "PatternGodUtils.h"
#include "ScoreResultsCachePolynom.h"
#include "PolynomScoreCalcs.h"
#include "TimeLineBuilder.h"

#pragma once

enum class PatternGodComparerType
{
	PolynomComparer,
	ScoreComparer,
	NONE
	// Add more comparer types as needed
};
struct PatternGodComparerTypeList { std::vector<PatternGodComparerType> types = { PatternGodComparerType::PolynomComparer , PatternGodComparerType::ScoreComparer }; };


class PatternGod_BaseComparer
{
protected:
	std::vector<ChangeNode*>* m_historyTL;
	DBMap* m_historyDB_Map = nullptr;

	std::vector<ChangeNode*>* m_currentTL = nullptr;
	DBMap* m_currDB_Map = nullptr;

	int m_scanCounter = 0; // Counter for the number of scans performed
	int m_NumOfSkips_ForLowScoreScan = 0; 

	PatternGodComparerType m_comparerType = PatternGodComparerType::NONE; // Type of the comparer

	PatternGod_BaseComparer(std::vector<ChangeNode*>* history) : m_historyTL(history)
	{
		m_historyDB_Map = new DBMap();
		m_currDB_Map = new DBMap();
		TimeLineBuilder::Build_TimeLineDB(history, m_historyDB_Map);
	}
	PatternGod_BaseComparer(std::vector<ChangeNode*>* history, std::vector<ChangeNode*>* curr) : m_historyTL(history), m_currentTL(curr)
	{
		m_historyDB_Map = new DBMap();
		m_currDB_Map = new DBMap();
		TimeLineBuilder::Build_TimeLineDB(history, m_historyDB_Map);
		TimeLineBuilder::Build_TimeLineDB(curr, m_currDB_Map);
	}
	~PatternGod_BaseComparer()
	{
		if (m_historyDB_Map != nullptr) {
			delete m_historyDB_Map; // Free the history DB map
			m_historyDB_Map = nullptr;
		}
		if (m_currDB_Map != nullptr) {
			delete m_currDB_Map; // Free the current DB map
			m_currDB_Map = nullptr;
		}
	}


public:
	PatternGodComparerType Get_ComparerType() const { return m_comparerType; } // Get the comparer type
	virtual int ScanForOneMinPatterns_MultiThreaded(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D);

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
	virtual void Set_CurrentTimeLine(std::vector<ChangeNode*>* curr) {
		m_currentTL = curr;
		m_currDB_Map->Clear(); // Clear the current DB map before building it again
		TimeLineBuilder::Build_TimeLineDB(curr, m_currDB_Map);

	} // Set the current time line for comparison

};

