#pragma once
#include "BaseLayerDataExporter.h"
#include "OrderStruct.h"

class TradeLayerDataExporter : public BaseLayerDataExporter
{
	protected:
		virtual LayerType GetLayerType() const override { return LayerType::TradeManagementLayer; }
public:
	TradeLayerDataExporter(std::string name = "") : BaseLayerDataExporter(name) {}
	void AddTradeData(OrderEntity* order);
};

