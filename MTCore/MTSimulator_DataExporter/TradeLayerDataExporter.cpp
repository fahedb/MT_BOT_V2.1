#include "TradeLayerDataExporter.h"
#include "LayersJsonKeys.h"
void TradeLayerDataExporter::AddTradeData(OrderEntity* order)
{
	
	nlohmann::json orderJson;
	orderJson[order->Get_TimePoint()] = {
		{ JsonKey_OrderSeqNum			, order->Get_OrderSeqNum() },
		{ JsonKey_OrderType				, order->Get_OrderType_str() },
		{ JsonKey_OrderStatus			, order->Get_OrderStatus_str() },
		{ JsonKey_StatisticsTimePoint	, order->Get_StatisticsTimePoint() },
		{ JsonKey_PairType				, order->Get_PairType_str() },
		{ JsonKey_IsActive				, order->IsActive() },
		{ JsonKey_StopLoss_pips			, order->Get_StopLoss() },
		{ JsonKey_TakeProfit_pips		, order->Get_TakeProfit() }
	};
	m_dataSummary.push_back(orderJson);
}
