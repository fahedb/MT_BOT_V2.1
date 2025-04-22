#include <vector>
#include "DirectionStats.h"

#pragma once
class DirectionStatsCache
{
	std::vector<DirectionStats> statsRep;

public:
	void Add_NewStat(DirectionStats dirStat)
	{
		int maxStats = Configrations::getInstance()->Get_Policy()->SavedDirectionStaticsNum;
		while (statsRep.size() >= maxStats)
		{
			statsRep.erase(statsRep.begin());
		}
		statsRep.push_back(dirStat);
	}

	DirectionStats* Get_Stats(size_t idx)
	{
		if (idx >= statsRep.size())
			return nullptr;
		return &statsRep.at(idx);
	}

	DirectionStats* Get_Stats(std::string timePoint)
	{
		for (int i = 0; i < statsRep.size(); i++)
			if (statsRep.at(i).timePoint == timePoint)
				return &statsRep.at(i);
		return nullptr;
	}

private:


	DirectionStatsCache()
	{

	}
	DirectionStatsCache(const DirectionStatsCache&) = delete;
	DirectionStatsCache& operator=(const DirectionStatsCache&) = delete;

	// Static method to get the instance of the Singleton
	static DirectionStatsCache* getInstance() {
		static DirectionStatsCache instance; // Guaranteed to be destroyed and instantiated on first use
		return &instance;
	}
};

