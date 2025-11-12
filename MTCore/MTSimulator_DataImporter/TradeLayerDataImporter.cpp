#include "TradeLayerDataImporter.h"
#include "Converts.h"
#include "nlohmann\json.hpp"

QueueStruct<std::unique_ptr<OrderEntity>> TradeLayerDataImporter::Create_QueueItem(const std::string& line, long lineIndex)
{
	// Direct sons: 
	//JsonKey_OrderSeqNum			
	//JsonKey_OrderType
	//JsonKey_OrderStatus
	//JsonKey_StatisticsTimePoint
	//JsonKey_PairType
	//JsonKey_IsActive
	//JsonKey_StopLoss_pips
	//JsonKey_TakeProfit_pips

	nlohmann::json jsonData;
	try {

		jsonData = nlohmann::json::parse(line);
	}
	catch (const std::exception& e) {
		// Handle parsing error (e.g., log the error)
		throw std::runtime_error("ScoringLayerImporter::Create_QueueItem : JSON parsing error at line " + std::to_string(lineIndex) + ": " + e.what());
	}
	
	auto it = jsonData.begin();
	long long currTP = Converts::ToLongLong(it.key());
	long OrderSeqNum = Converts::ToLong(jsonData.at(JsonKey_OrderSeqNum).get<std::string>());
	OrderType orderType = OrderTypeUtils::Get_OrderType_FromString(jsonData.at(JsonKey_OrderType).get<std::string>());
	long long statisticsTimePoint = Converts::ToLongLong(jsonData.at(JsonKey_StatisticsTimePoint).get<std::string>());
	CoinsPair pairType = CoinsPairUtils::Get_PairType(jsonData.at(JsonKey_PairType).get<std::string>());
	int sl_pips = jsonData.at(JsonKey_StopLoss_pips).get<int>();
	int tp_pips = jsonData.at(JsonKey_TakeProfit_pips).get<int>();


	OrderEntity order(OrderSeqNum, orderType, std::to_string(currTP), std::to_string(statisticsTimePoint), pairType, tp_pips, sl_pips);
	QueueStruct<std::unique_ptr<OrderEntity>> queueItem;
	queueItem.item = std::make_unique<OrderEntity>(std::move(order));
	queueItem.lineIndex = lineIndex;

}