#pragma once
#include "BaseLayerImporter.h"
#include "OrderStruct.h"
#define TradeLayer_QueueItemType std::unique_ptr<OrderEntity>
class TradeLayerDataImporter : public BaseLayerImporter<std::unique_ptr<OrderEntity>>
{
	public:
	TradeLayerDataImporter(std::string filePath, std::vector<ChangeNode*>* historyTL, DBMap* historyDBMap) : BaseLayerImporter(filePath, historyTL, historyDBMap)
	{
	}
	virtual long long Get_DataTime(const std::unique_ptr<OrderEntity>& item) override;
protected:
	virtual QueueStruct<std::unique_ptr<OrderEntity>> Create_QueueItem(const std::string& line, long lineIndex) override;

};

