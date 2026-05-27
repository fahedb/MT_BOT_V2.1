#include "PatternGod_PolynomFilterEngine.h"
#include "TimeLineBuilder.h"
#include "PolynomScoreCalcs.h"
#include "LOGGER.H"
#include "Static_CurrentTimeLineRep.h"

PatternGod_PolynomFilterEngine::PatternGod_PolynomFilterEngine(std::vector<ChangeNode*>* history, const int polyLen, bool EnableScoreCaching, double wEval, double wRMSE, double wCoeff) {
	m_historyDB_Map = new DBMap();
	m_historyTL = history;
	TimeLineBuilder::Build_TimeLineDB(history, m_historyDB_Map);
	m_currentTL = Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLine();
	m_polyLen = polyLen;
	m_enableScoreCaching = EnableScoreCaching;
	m_wEval = wEval;
	m_wRMSE = wRMSE;
	m_wCoeff = wCoeff;
}
PatternGod_PolynomFilterEngine::~PatternGod_PolynomFilterEngine() {
	if (m_historyDB_Map != nullptr) {
		delete m_historyDB_Map; // Free the history DB map
		m_historyDB_Map = nullptr;
	}
}

bool PatternGod_PolynomFilterEngine::ProccessPreCondition(int patternLen)
{
	if (m_currentTL == nullptr || m_historyTL == nullptr || m_historyDB_Map == nullptr)
	{
		Logger::getInstance().WriteLog("PatternGod_PolynomFilterEngine: Current or History time line or DB map is not set.", true, true, LogType::ERR);
		return false;
	}
	// Get newest Time point
	long long newestTp = Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLine()->back()->point_DateTimeLong;
	if (newestTp != m_currentTL->back()->point_DateTimeLong)
	{
		Logger::getInstance().WriteLog("PatternGod_PolynomFilterEngine: Current time line does not match the newest time point.", true, true, LogType::ERR);
		return false;
	}
	if (patternLen <= 1)
	{
		Logger::getInstance().WriteLog("PatternGod_PolynomFilterEngine: Pattern length must be greater than 1.", true, true, LogType::ERR);
		return false;
	}
	return true;
}

std::vector<std::pair<ChangeNode*, ScoresResult>> PatternGod_PolynomFilterEngine::ProcessPatterns(std::vector<ChangeNode*>* headsOfPatterns, int patternLen) // NOTE: HeadsOfPatterns is the last node of each pattern
{
	std::vector<std::pair<ChangeNode*, ScoresResult>> patternsRes;
	if (!ProccessPreCondition(patternLen))
		return patternsRes;
	
	// Iterate over each head of the pattern
	for (int i = 0; i < headsOfPatterns->size(); i++)
	{
		ChangeNode* head = headsOfPatterns->at(i);
		if (head == nullptr) continue;
		// Find the index of the *tail* in the history DB map
		int startIdx = m_historyDB_Map->GetIdx_Of_TimePoint(head->point_DateTimeLong);

		if (startIdx < 0) continue; // If the head is not found, skip to the next one
		// Ensure we don't go out of bounds
		if (startIdx + patternLen > m_historyTL->size()) continue;

		// Collect the pattern nodes
		std::vector<ChangeNode*> patternNodes;
		patternNodes.reserve(patternLen);
		for (int j = 0; j < patternLen; j++)
		{
			patternNodes.push_back(m_historyTL->at(startIdx + j));
		}
		int resCacheHits = 0, resCacheMisses = 0, polyCacheHits = 0, polyCacheMisses = 0;

		ScoresResult scoreRes = PolynomScoreCalcs::CompareTimeLines(&patternNodes, m_currentTL, 0, resCacheHits, resCacheMisses, polyCacheHits, polyCacheMisses, 0, 0, m_polyLen, m_enableScoreCaching, m_wEval, m_wRMSE, m_wCoeff);
		patternsRes.emplace_back(head, scoreRes);
		// For example, you might want to store them, analyze them, etc.
	}
	return patternsRes;
}
