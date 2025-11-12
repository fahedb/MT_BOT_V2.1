#pragma once
#include "BaseLayerImporter.h"
#include "OrderStruct.h"

class TradeLayerDataImporter : public BaseLayerImporter<std::unique_ptr<OrderEntity>>
{
	public:
	TradeLayerDataImporter(std::string filePath, std::vector<ChangeNode*>* historyTL, DBMap* historyDBMap) : BaseLayerImporter(filePath, historyTL, historyDBMap)
	{
	}
protected:
	virtual QueueStruct<std::unique_ptr<OrderEntity>> Create_QueueItem(const std::string& line, long lineIndex) override;

};

