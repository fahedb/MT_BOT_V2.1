#pragma once
#include "ChangeData.h"
#include "DBMap.h"

class PatternGod_PolynomFilterEngine
{
private:
	DBMap* m_historyDB_Map = nullptr;

	std::vector<ChangeNode*>* m_currentTL = nullptr;
	std::vector<ChangeNode*>* m_historyTL = nullptr;
	int m_polyLen;
	bool m_enableScoreCaching;
	double m_wEval, m_wRMSE, m_wCoeff;
public:
	PatternGod_PolynomFilterEngine(std::vector<ChangeNode*>* history, const int polyLen, bool EnableScoreCaching, double wEval, double wRMSE, double wCoeff);
	~PatternGod_PolynomFilterEngine();
	void Set_CurrentTimeLine(std::vector<ChangeNode*>* curr) { m_currentTL = curr; }
	bool ProccessPreCondition(int patternLen);
	std::vector<std::pair<ChangeNode*, ScoresResult>> ProcessPatterns(std::vector<ChangeNode*>* headsOfPatterns, int patternLen);
};

