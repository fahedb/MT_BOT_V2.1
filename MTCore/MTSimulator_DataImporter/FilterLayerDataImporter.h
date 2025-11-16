#pragma once
#include "BaseLayerImporter.h"

#define FilterLayer_QueueItemType std::pair<long long, std::vector<std::pair<ChangeNode*, ScoresResult>>>
class FilterLayerDataImporter : public BaseLayerImporter<std::pair<long long, std::vector<std::pair<ChangeNode*, ScoresResult>>>>
{
	public:
	FilterLayerDataImporter(std::string filePath, std::vector<ChangeNode*>* historyTL, DBMap* historyDBMap) : BaseLayerImporter(filePath, historyTL, historyDBMap)
	{
	}
	virtual long long Get_DataTime(const std::pair<long long, std::vector<std::pair<ChangeNode*, ScoresResult>>>& item) override { return item.first; }
protected:
	virtual QueueStruct<std::pair<long long, std::vector<std::pair<ChangeNode*, ScoresResult>>>> Create_QueueItem(const std::string& line, long lineIndex) override;
};

