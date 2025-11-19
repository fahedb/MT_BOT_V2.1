#pragma once
#include "BaseLayerImporter.h"
#include "DirectionStats.h"
#include "Converts.h"
#define StatisticsLayer_QueueItemType DirectionStats
class StatisticsLayerDataImporter : public BaseLayerImporter<DirectionStats>
{
	public:
	StatisticsLayerDataImporter(std::string filePath, std::vector<ChangeNode*>* historyTL, DBMap* historyDBMap) : BaseLayerImporter(filePath, historyTL, historyDBMap)
	{
	}
	virtual long long Get_DataTime(const DirectionStats& item) override { return Converts::ToLongLong(item.Get_TimePoint()); }
protected:
	virtual QueueStruct<DirectionStats> Create_QueueItem(const std::string& line, long lineIndex) override;

private:
	static void ParseAndSetVotedStats(const nlohmann::json& votedJson, StatsType sType, DirectionStats& dirStats);
	static void ParseAndSetStepMeans(const nlohmann::json& stepsMeanJson,CommonDirection dir, DirectionStats& dirStats);
};

