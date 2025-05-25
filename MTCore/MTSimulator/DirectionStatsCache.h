#include <vector>
#include "DirectionStats.h"

#pragma once
class DirectionStatsCache
{
	std::vector<DirectionStats*> statsRep;

public:
	void Add_NewStat(DirectionStats dirStat)
	{
		int maxStats = Configrations::getInstance()->Get_Policy()->SavedDirectionStaticsNum;
		while (statsRep.size() >= maxStats)
		{
			DirectionStats* stats = statsRep.at(0);
			statsRep.erase(statsRep.begin());
			delete stats;
		}
		statsRep.push_back(new DirectionStats(dirStat));
	}

	void Add_NewStat(DirectionStats* dirStat)
	{
		int maxStats = Configrations::getInstance()->Get_Policy()->SavedDirectionStaticsNum;
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

	DirectionStats* Get_Stats(std::string timePoint)
	{
		for (int i = 0; i < statsRep.size(); i++)
			if (statsRep.at(i)->Get_TimePoint() == timePoint)
				return statsRep.at(i);
		return nullptr;
	}
	DirectionStats* Get_LastStats() { return statsRep.at(statsRep.size() - 1); }
	
	int Get_IdxOfStats(std::string timePoint)
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
		ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
		statsRep.reserve(policy->SavedDirectionStaticsNum);
	}
};

