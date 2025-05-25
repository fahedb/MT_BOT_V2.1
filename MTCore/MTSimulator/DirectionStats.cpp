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

void DirectionStats::Add_Patterns(std::vector<TimeLineScored>* patterns)
{
	for (int i = 0; i < patterns->size(); i++)
	{
		m_patterns_wScores.push_back(patterns->at(i));
	}
}

DirectionGroupStats& DirectionStats::GetGroupStatsByName(const std::string& son)
{
	if (son == "First") return first;
	if (son == "General") return general;
	if (son == "Highest") return highest;
	return general;
}