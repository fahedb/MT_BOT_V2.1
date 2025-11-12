#pragma once

enum class StatsType
{
	General = 0,
	First = 1,
	Highest = 2
};

class StatsType_Utils
{
public:
	static std::string GetStatsTypeStr(StatsType type)
	{
		switch (type)
		{
		case StatsType::General:
			return "General";
		case StatsType::First:
			return "First";
		case StatsType::Highest:
			return "Highest";
		default:
			return "Unknown";
		}
		return "Unknown";
	}
	static StatsType GetStatsTypeEnum(std::string type)
	{
		if (type == "General")
			return StatsType::General;
		else if (type == "First")
			return StatsType::First;
		else if (type == "Highest")
			return StatsType::Highest;
		else
			return StatsType::General; // Default to General if unknown
	}
};