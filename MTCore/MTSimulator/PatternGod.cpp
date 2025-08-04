#include "PatternGod.h"

inline void PatternGod::InitComparers() // Initialize the comparers with the history time line
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
