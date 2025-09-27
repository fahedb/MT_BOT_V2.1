#include "DirectionStatsCache.h"
#include "CONFIGRATIONS.H"

PatternGodStatisticsPolicy* m_glob_statsPolicy = nullptr;

void DirectionStatsCache::Add_NewStat(DirectionStats& dirStat, bool moveMemory)
{
	int maxStats = m_glob_statsPolicy->SavedDirectionStaticsLen;
	while (statsRep.size() >= maxStats)
	{
		DirectionStats* stats = statsRep.at(0);
		statsRep.erase(statsRep.begin());
		delete stats;
	}
	if (!moveMemory)
		statsRep.push_back(new DirectionStats(dirStat));
	else
		statsRep.emplace_back(new DirectionStats(std::move(dirStat)));
}

void DirectionStatsCache::Add_NewStat(DirectionStats* dirStat)
{
	int maxStats = m_glob_statsPolicy->SavedDirectionStaticsLen;
	while (statsRep.size() >= maxStats)
	{
		DirectionStats* stats = statsRep.at(0);
		statsRep.erase(statsRep.begin());
		delete stats;
	}
	statsRep.push_back(dirStat);
}

DirectionStats* DirectionStatsCache::Get_Stats(size_t idx)
{
	if (idx >= statsRep.size())
		return nullptr;
	return statsRep.at(idx);
}

DirectionStats* DirectionStatsCache::Get_Stats(const std::string& timePoint)
{
	for (int i = 0; i < statsRep.size(); i++)
		if (statsRep.at(i)->Get_TimePoint() == timePoint)
			return statsRep.at(i);
	return nullptr;
}

DirectionStats* DirectionStatsCache::Get_LastStats() {
	if (statsRep.empty())
		return nullptr;
	return statsRep.at(statsRep.size() - 1);
}

int DirectionStatsCache::Get_IdxOfStats(const std::string& timePoint)
{
	for (int i = 0; i < statsRep.size(); i++)
		if (statsRep.at(i)->Get_TimePoint() == timePoint)
			return i;
	return -1;
}

DirectionStatsCache::DirectionStatsCache()
{
	if (!m_glob_statsPolicy)
		m_glob_statsPolicy = Configrations::getInstance()->Get_Policy<PatternGodStatisticsPolicy>(PolicyType::PatternGodStatisticsPolicy);

	statsRep.reserve(m_glob_statsPolicy->SavedDirectionStaticsLen);
}


void DynamicDirectionStatsCache::Add_NewStat(DirectionStats& dirStat, bool moveMemory)
{
	int maxStats = m_glob_statsPolicy->SavedDirectionStaticsLen;
	while (statsRep.size() >= maxStats)
	{
		DirectionStats* stats = statsRep.at(0);
		statsRep.erase(statsRep.begin());
		delete stats;
	}
	if (!moveMemory)
		statsRep.push_back(new DirectionStats(dirStat));
	else
		statsRep.emplace_back(new DirectionStats(std::move(dirStat)));
}

void DynamicDirectionStatsCache::Add_NewStat(DirectionStats* dirStat)
{
	int maxStats = m_glob_statsPolicy->SavedDirectionStaticsLen;
	while (statsRep.size() >= maxStats)
	{
		DirectionStats* stats = statsRep.at(0);
		statsRep.erase(statsRep.begin());
		delete stats;
	}
	statsRep.push_back(dirStat);
}

DirectionStats* DynamicDirectionStatsCache::Get_Stats(size_t idx)
{
	if (idx >= statsRep.size())
		return nullptr;
	return statsRep.at(idx);
}

DirectionStats* DynamicDirectionStatsCache::Get_Stats(const std::string& timePoint)
{
	for (int i = 0; i < statsRep.size(); i++)
		if (statsRep.at(i)->Get_TimePoint() == timePoint)
			return statsRep.at(i);
	return nullptr;
}

DirectionStats* DynamicDirectionStatsCache::Get_LastStats() {
	if (statsRep.empty())
		return nullptr;
	return statsRep.at(statsRep.size() - 1);
}

int DynamicDirectionStatsCache::Get_IdxOfStats(const std::string& timePoint)
{
	for (int i = 0; i < statsRep.size(); i++)
		if (statsRep.at(i)->Get_TimePoint() == timePoint)
			return i;
	return -1;
}

DynamicDirectionStatsCache::DynamicDirectionStatsCache()
{
	if (!m_glob_statsPolicy)
		m_glob_statsPolicy = Configrations::getInstance()->Get_Policy<PatternGodStatisticsPolicy>(PolicyType::PatternGodStatisticsPolicy);
	statsRep.reserve(m_glob_statsPolicy->SavedDirectionStaticsLen);
}
