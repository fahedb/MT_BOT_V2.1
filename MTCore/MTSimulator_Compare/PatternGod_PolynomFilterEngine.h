#pragma once
#include "ChangeData.h"
#include "DBMap.h"

class PatternGod_PolynomFilterEngine
{
private:
	DBMap* m_historyDB_Map = nullptr;

	std::vector<ChangeNode*>* m_currentTL = nullptr;
	std::vector<ChangeNode*>* m_historyTL = nullptr;
public:
	PatternGod_PolynomFilterEngine(std::vector<ChangeNode*>* history);
	~PatternGod_PolynomFilterEngine();
	void Set_CurrentTimeLine(std::vector<ChangeNode*>* curr) { m_currentTL = curr; }
	bool ProccessPreCondition(int patternLen);
	std::vector<std::pair<ChangeNode*, ScoresResult>> ProcessPatterns(std::vector<ChangeNode*>* headsOfPatterns, int patternLen);
};

