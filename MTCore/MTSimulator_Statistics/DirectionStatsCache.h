#include <vector>
#include "DirectionStats.h"

#pragma once
class DirectionStatsCache
{
	std::vector<DirectionStats*> statsRep;
public:
	void Add_NewStat(DirectionStats& dirStat,bool moveMemory = false);
	void Add_NewStat(DirectionStats* dirStat);
	DirectionStats* Get_Stats(size_t idx);
	DirectionStats* Get_Stats(const std::string& timePoint);
	DirectionStats* Get_LastStats();
	int Get_IdxOfStats(const std::string& timePoint);

	DirectionStatsCache(const DirectionStatsCache&) = delete;
	DirectionStatsCache& operator=(const DirectionStatsCache&) = delete;

	// Static method to get the instance of the Singleton
	static DirectionStatsCache* getInstance() {
		static DirectionStatsCache instance; // Guaranteed to be destroyed and instantiated on first use
		return &instance;
	}
private:
	DirectionStatsCache();
};



class DynamicDirectionStatsCache
{
	std::vector<DirectionStats*> statsRep;

public:
	void Add_NewStat(DirectionStats& dirStat, bool moveMemory = false);
	void Add_NewStat(DirectionStats* dirStat);
	DirectionStats* Get_Stats(size_t idx);
	DirectionStats* Get_Stats(const std::string& timePoint);
	DirectionStats* Get_LastStats();
	int Get_IdxOfStats(const std::string& timePoint);

	DynamicDirectionStatsCache(const DynamicDirectionStatsCache&) = delete;
	DynamicDirectionStatsCache& operator=(const DynamicDirectionStatsCache&) = delete;

	// Static method to get the instance of the Singleton
	static DynamicDirectionStatsCache* getInstance() {
		static DynamicDirectionStatsCache instance; // Guaranteed to be destroyed and instantiated on first use
		return &instance;
	}
private:
	DynamicDirectionStatsCache();
};

