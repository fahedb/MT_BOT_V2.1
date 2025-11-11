#pragma once
#include "BaseLayerImporter.h"

class FilterLayerDataImporter : public BaseLayerImporter<std::pair<long long, std::vector<std::pair<ChangeNode*, ScoresResult>>>>
{
	public:
	FilterLayerDataImporter(std::string filePath, std::vector<ChangeNode*>* historyTL, DBMap* historyDBMap) : BaseLayerImporter(filePath, historyTL, historyDBMap)
	{
	}
protected:
	virtual QueueStruct<std::pair<long long, std::vector<std::pair<ChangeNode*, ScoresResult>>>> Create_QueueItem(const std::string& line, long lineIndex) override;
};

