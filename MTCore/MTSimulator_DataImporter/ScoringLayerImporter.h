#pragma once
#include <string>
#include <mutex>
#include <queue>
#include <vector>

#include "ChangeData.h"
#include "QueueStruct.h"

#include "BaseLayerImporter.h"

// Outer: pair<timepoint, pair<history nodes, score>>
#define ScoringLayer_QueueItemType std::pair<long long, std::vector<std::pair<ChangeNode*, ScoresResult>>>
class ScoringLayerImporter : public BaseLayerImporter<std::pair<long long, std::vector<std::pair<ChangeNode*, ScoresResult>>>>
{
public:
	ScoringLayerImporter(std::string filePath, std::vector<ChangeNode*>* historyTL, DBMap* historyDBMap) : BaseLayerImporter(filePath,historyTL, historyDBMap)
	{
	}
	virtual QueueStruct<std::pair<long long, std::vector<std::pair<ChangeNode*, ScoresResult>>>> Create_QueueItem(const std::string& line, long lineIndex) override;
	virtual long long Get_DataTime(const std::pair<long long, std::vector<std::pair<ChangeNode*, ScoresResult>>>& item) override { return item.first; }
private:

};

