#include <vector>
#include "DirectionStats.h"

#pragma once
class DirectionStatsCache
{
	std::vector<DirectionStats*> statsRep;
	PatternGodStatisticsPolicy* m_statsPolicy = nullptr;
public:
	void Add_NewStat(DirectionStats& dirStat,bool moveMemory = false)
	{
		int maxStats = m_statsPolicy->SavedDirectionStaticsLen;
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

	void Add_NewStat(DirectionStats* dirStat)
	{
		int maxStats = m_statsPolicy->SavedDirectionStaticsLen;
		while (statsRep.size() >= maxStats)
		{
			DirectionStats* stats = statsRep.at(0);
			statsRep.erase(statsRep.begin());
			delete stats;
		}
		statsRep.push_back(dirStat);
	}


	DirectionStats* Get_Stats(size_t idx)
	{
		if (idx >= statsRep.size())
			return nullptr;
		return statsRep.at(idx);
	}

	DirectionStats* Get_Stats(const std::string& timePoint)
	{
		for (int i = 0; i < statsRep.size(); i++)
			if (statsRep.at(i)->Get_TimePoint() == timePoint)
				return statsRep.at(i);
		return nullptr;
	}
	DirectionStats* Get_LastStats() { 
		if (statsRep.empty())
			return nullptr;
		return statsRep.at(statsRep.size() - 1); 
	}
	
	int Get_IdxOfStats(const std::string& timePoint)
	{
		for (int i = 0; i < statsRep.size(); i++)
			if (statsRep.at(i)->Get_TimePoint() == timePoint)
				return i;
		return -1;
	}


	DirectionStatsCache(const DirectionStatsCache&) = delete;
	DirectionStatsCache& operator=(const DirectionStatsCache&) = delete;

	// Static method to get the instance of the Singleton
	static DirectionStatsCache* getInstance() {
		static DirectionStatsCache instance; // Guaranteed to be destroyed and instantiated on first use
		return &instance;
	}
private:
	DirectionStatsCache()
	{
		m_statsPolicy = Configrations::getInstance()->Get_Policy<PatternGodStatisticsPolicy>(PolicyType::PatternGodStatisticsPolicy);
		statsRep.reserve(m_statsPolicy->SavedDirectionStaticsLen);
	}
};



class DynamicDirectionStatsCache
{
	std::vector<DirectionStats*> statsRep;
	PatternGodStatisticsPolicy* m_statsPolicy = nullptr;

public:
	void Add_NewStat(DirectionStats& dirStat, bool moveMemory = false)
	{
		int maxStats = m_statsPolicy->SavedDirectionStaticsLen;
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

	void Add_NewStat(DirectionStats* dirStat)
	{
		int maxStats = m_statsPolicy->SavedDirectionStaticsLen;
		while (statsRep.size() >= maxStats)
		{
			DirectionStats* stats = statsRep.at(0);
			statsRep.erase(statsRep.begin());
			delete stats;
		}
		statsRep.push_back(dirStat);
	}


	DirectionStats* Get_Stats(size_t idx)
	{
		if (idx >= statsRep.size())
			return nullptr;
		return statsRep.at(idx);
	}

	DirectionStats* Get_Stats(const std::string& timePoint)
	{
		for (int i = 0; i < statsRep.size(); i++)
			if (statsRep.at(i)->Get_TimePoint() == timePoint)
				return statsRep.at(i);
		return nullptr;
	}
	DirectionStats* Get_LastStats() {
		if (statsRep.empty())
			return nullptr;
		return statsRep.at(statsRep.size() - 1);
	}

	int Get_IdxOfStats(const std::string& timePoint)
	{
		for (int i = 0; i < statsRep.size(); i++)
			if (statsRep.at(i)->Get_TimePoint() == timePoint)
				return i;
		return -1;
	}


	DynamicDirectionStatsCache(const DynamicDirectionStatsCache&) = delete;
	DynamicDirectionStatsCache& operator=(const DynamicDirectionStatsCache&) = delete;

	// Static method to get the instance of the Singleton
	static DynamicDirectionStatsCache* getInstance() {
		static DynamicDirectionStatsCache instance; // Guaranteed to be destroyed and instantiated on first use
		return &instance;
	}
private:
	DynamicDirectionStatsCache()
	{
		m_statsPolicy = Configrations::getInstance()->Get_Policy<PatternGodStatisticsPolicy>(PolicyType::PatternGodStatisticsPolicy);
		statsRep.reserve(m_statsPolicy->SavedDirectionStaticsLen);
	}
};

