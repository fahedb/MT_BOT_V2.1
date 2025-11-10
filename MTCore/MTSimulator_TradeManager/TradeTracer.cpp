#include "TradeTracer.h"
#include "Converts.h"
#include "TimeLineBuilder.h"


DirectionStats* TradeTracer::GetRelativeStats(OrderEntity* order, TradeTracingType forceTradeTracing) // Force trade tracing to be either dynamic or const
{
	if (!order) // If no order, return empty stats
	{
		Logger::getInstance().WriteLog("No valid order for TradeTracing", false, true, LogType::SPECIAL_LOG);
		return nullptr;
	}

	bool dynamicTracing = forceTradeTracing == TradeTracingType::DynamicTracing && m_tradeTracingPolicy->EnableDynamicTimeLine_TradeTracing; // Only if the policy allows it
	bool constTracing = forceTradeTracing == TradeTracingType::ConstTracing || forceTradeTracing == TradeTracingType::NONE; // The defualt tracing is const tracing

	if (dynamicTracing)
		return DynamicTradeTracing(order); // Generating a new stats with the order scanned timeline + last x bars
	else if (constTracing)
		return ConstTradeTracing(order); // Getting the last stats for the order
	else
		return nullptr;

}

DirectionStats* TradeTracer::DynamicTradeTracing(OrderEntity* order)
{
	Logger& logger = Logger::getInstance();

	logger.WriteLog("Start Tracing order, Seq: " + std::to_string(order->Get_OrderSeqNum()), false, true, LogType::TRADE_TRACE_LOG);

	long long orderTimePoint = Converts::ToLongLong(order->Get_TimePoint()); // The timepoint where the order was created
	Static_CurrentTimeLineRep* currTimeLineRep = Static_CurrentTimeLineRep::getInstance();
	DirectionStatsCache* statsCache = DirectionStatsCache::getInstance();

	int timePointIdx = currTimeLineRep->Get_TraceTimeLine()->Get_DBMap()->GetIdx_Of_TimePoint(orderTimePoint);
	if (timePointIdx < 0)
	{
		Logger::getInstance().WriteLog("Could not find ChangeNode for the TimePoint of order " + std::to_string(order->Get_OrderSeqNum()), false, true, LogType::TRADE_TRACE_LOG);
		return nullptr;
	}

	// Calc the start index of the timeline we should scan
	int startIdx = (timePointIdx - m_confPolicy->Pattern_CurrentTimeLineLen) >= 0 ? timePointIdx - m_confPolicy->Pattern_CurrentTimeLineLen : 0;

	// Getting the timeline we should scan
	std::vector<ChangeNode*> toScanPeriod = std::vector<ChangeNode*>(currTimeLineRep->Get_TraceTimeLine()->Get_TimeLine()->begin() + startIdx, currTimeLineRep->Get_TraceTimeLine()->Get_TimeLine()->end());
	m_patternScanner->Set_CurrentTimeLine(&toScanPeriod);

	std::vector<std::pair< ChangeNode*, ScoresResult>> potentialMatches_WResults;
	m_patternScanner->ScanForOneMinPatterns_MultiThreaded(potentialMatches_WResults);
	m_patternScanner->PolynomEngineFilter(potentialMatches_WResults);

	// Our stats should be until the end of the future len
	int futureLen = (m_confPolicy->Pattern_CurrentTimeLineLen + m_statsPolicy->Score_Future_Vision_Candles) - toScanPeriod.size();
	// If the future length is negative, we should set it to 1, That means, if we a pattern that is longer than the current timeLine + future len we will get the stats for a single bar
	// This logic is to limit the scanning only for the future length of the pattern
	if (futureLen <= 0)
		futureLen = 1;

	DirectionStats* stats = new DirectionStats(std::move(m_statisticsEngine->DetectCommonDirection(&potentialMatches_WResults, futureLen)));

	std::vector<TimeLineScored> scoredTimeLine = TimeLineBuilder::Build_TimeLineScored_Patterns(m_historyTL, &potentialMatches_WResults, toScanPeriod.size(), futureLen);
	TimeLine currTL(toScanPeriod);

	stats->Add_Patterns(&scoredTimeLine,true);
	stats->Add_CurrPattern(currTL,true);

	DynamicDirectionStatsCache::getInstance()->Add_NewStat(*stats, true); // Add the stats to the dynamic cache
	return stats;
}
