#include "DirectionStats.h"

DirectionGroupStats& DirectionStats::GetGroupStats(StatsType type)
{
	switch (type)
	{
	case StatsType::General:
		return general;
		break;
	case StatsType::First:
		return first;
		break;
	case StatsType::Highest:
		return highest;
		break;
	default:
		return general;
		break;
	}
}

void DirectionStats::Add_Patterns(std::vector<TimeLineScored>* patterns,bool moveMemory)
{
	if (patterns == nullptr || patterns->empty())
	{
		Logger::getInstance().WriteLog("Add_Patterns: patterns is null or empty, nothing to add.", false, true, LogType::ERR);
		return;
	}

	for (int i = 0; i < patterns->size(); i++)
	{
		if (!moveMemory)
			m_patterns_wScores.push_back(patterns->at(i));
		else
			m_patterns_wScores.emplace_back(std::move(patterns->at(i)));
	}
}

DirectionGroupStats& DirectionStats::GetGroupStatsByName(const std::string& son)
{
	if (son == "First") return first;
	if (son == "General") return general;
	if (son == "Highest") return highest;
	return general;
}