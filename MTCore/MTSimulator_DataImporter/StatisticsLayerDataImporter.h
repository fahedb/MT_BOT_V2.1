#pragma once
#include "BaseLayerImporter.h"
#include "DirectionStats.h"
class StatisticsLayerDataImporter : public BaseLayerImporter<DirectionStats>
{
	public:
	StatisticsLayerDataImporter(std::string filePath, std::vector<ChangeNode*>* historyTL, DBMap* historyDBMap) : BaseLayerImporter(filePath, historyTL, historyDBMap)
	{
	}
protected:
	virtual QueueStruct<DirectionStats> Create_QueueItem(const std::string& line, long lineIndex) override;

private:
	static void ParseAndSetVotedStats(const nlohmann::json& votedJson, StatsType sType, DirectionStats& dirStats);
	static void ParseAndSetStepMeans(const nlohmann::json& stepsMeanJson,CommonDirection dir, DirectionStats& dirStats);
};

