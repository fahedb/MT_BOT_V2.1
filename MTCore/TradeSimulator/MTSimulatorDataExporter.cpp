#include "MTSimulatorDataExporter.h"
#include "MTExportTagsDef.h"
#include "Converts.h"

void MTSimulatorDataExporter::ExportCurrentSimulationState(double accountBalance, double equityBalance, double usedMargin, double freeMargin, double floatingPnL, double currProfit, const Deal& closedDeal )
{
	ExportClosedDealsData(closedDeal);
	ExportCurrentAccountState(accountBalance, equityBalance, usedMargin, freeMargin, floatingPnL, currProfit, closedDeal.closeTime);
}

void MTSimulatorDataExporter::ExportClosedDealsData(const Deal& closedDeal)
{ // Json builder 
	nlohmann::json dealJson;
	dealJson[DealTag_Symbol] = closedDeal.symbol;
	dealJson[DealTag_Ticket] = closedDeal.ticket;
	dealJson[DealTag_OpenTime] = closedDeal.openTime;
	dealJson[DealTag_CloseTime] = closedDeal.closeTime;
	dealJson[DealTag_LotSize] = closedDeal.lotSize;
	dealJson[DealTag_OpenPrice] = closedDeal.openPrice;
	dealJson[DealTag_ClosePrice] = closedDeal.closePrice;
	dealJson[DealTag_StopLossPrice] = closedDeal.stopLossPrice;
	dealJson[DealTag_TakeProfitPrice] = closedDeal.takeProfitPrice;
	dealJson[DealTag_Type] = OrderTypeUtils::Get_OrderType_String(closedDeal.type);
	dealJson[DealTag_Profit] = closedDeal.profit;
	dealJson[DealTag_ProfitInPips] = closedDeal.profitInPips;
	dealJson[DealTag_CloseReason] = ClosureReasonUtils::ToString(closedDeal.closeReason);
	dealJson[Tag_SimulationID] = m_simulationId;
	dealJson[DealTag_NumOfBarsHeld] = closedDeal.numOfBarsHeld;
	m_dealsDataExporter->AddDataItem(dealJson);

}

void MTSimulatorDataExporter::ExportDirectionStatsData(DirectionStats* stats)
{
	// Implementation for exporting direction statistics data
	// This can be expanded based on the specific fields in DirectionStats
	if(!stats)
		return;
	// Json builder
	nlohmann::json directionStatsJson;
	directionStatsJson[DirectionStatsTag_TimePoint] = Converts::ToLongLong(stats->Get_TimePoint());
	directionStatsJson[Tag_SimulationID] = m_simulationId;
	directionStatsJson[DirectionStatsTag_Pair] = CoinsPairUtils::Get_PairType_str(stats->Get_PairCoin());
	directionStatsJson[DirectionStatsTag_TotalVotes] = stats->Get_Total(StatsType::General);
	
	ExportDirectionStatsVotedPercentages_Helper(stats, directionStatsJson[DirectionStatsTag_General_VotedPercentages], StatsType::General);
	ExportDirectionStatsVotedPercentages_Helper(stats, directionStatsJson[DirectionStatsTag_First_VotedPercentages], StatsType::First);
	ExportDirectionStatsVotedPercentages_Helper(stats, directionStatsJson[DirectionStatsTag_Highest_VotedPercentages], StatsType::Highest);

	ExportPatterns_Helper(stats, directionStatsJson[DirectionStatsTag_SimilarPatterns]);
	m_directionStatsExporter->AddDataItem(directionStatsJson);
}

void MTSimulatorDataExporter::ExportDirectionStatsVotedPercentages_Helper(DirectionStats* stats, nlohmann::json& json, StatsType statsT)
{
	robin_hood::unordered_map<CommonDirection, double> votedPercentages = stats->Get_VotedPercentages(StatsType::General);
	json[DirectionStatsTag_VeotedPercentages_TotalVotes] = stats->Get_Total(statsT);
	json[DirectionStatsTag_VeotedPercentages_UP_Percent] = votedPercentages[CommonDirection::UP];
	json[DirectionStatsTag_VeotedPercentages_DOWN_Percent] = votedPercentages[CommonDirection::DOWN];
	json[DirectionStatsTag_VeotedPercentages_BLUR_Percent] = votedPercentages[CommonDirection::BLUR];
	json[DirectionStatsTag_VeotedPercentages_Mean] = stats->mean(statsT);
	json[DirectionStatsTag_VeotedPercentages_PureMean_UP] = stats->Get_PureMean(statsT, CommonDirection::UP);
	json[DirectionStatsTag_VeotedPercentages_PureMean_DOWN] = stats->Get_PureMean(statsT, CommonDirection::DOWN);
	json[DirectionStatsTag_VeotedPercentages_StdDev] = stats->stddev(statsT);
	json[DirectionStatsTag_VeotedPercentages_MeanIndexes] = stats->meanIndexes(statsT);
}

void MTSimulatorDataExporter::ExportPatterns_Helper(DirectionStats* stats, nlohmann::json& json)
{
	return; // cancelled temporarily as patterns export is not needed for now, and it can be heavy on performance. We can enable it later if we need to analyze patterns data.
	std::vector<TimeLineScored>* patterns = stats->Get_SimilarPatterns();
	bool skippedTimeLineRecord = false;
	for( int i =0 ; i < patterns->size(); i++)
	{
		nlohmann::json patternJson;

		TimeLine* timeLine = patterns->at(i).Get_TimeLine();
		if (!timeLine)
		{
			skippedTimeLineRecord = true;
			continue;
		}

		std::vector<ChangeNode*>* timeLineVec = timeLine->Get_TimeLine();
		if (!timeLineVec || !timeLineVec->size())
		{
			skippedTimeLineRecord = true;
			continue;
		}

		ChangeNode* chN = timeLineVec->front();
		if (!chN)
		{
			skippedTimeLineRecord = true;
			continue;
		}
		patternJson[DirectionStatsTag_SimilarPattern_PatternTimePoint] = chN->point_DateTimeLong;
		// Add other pattern details as needed
		json.push_back(patternJson);
	}
	if (skippedTimeLineRecord)
		Logger::getInstance().WriteLog("MTSimulatorDataExporter::ExportPatterns_Helper: Skipped time line records for stats json export, For time point: " + stats->Get_TimePoint(), false, false, LogType::LOG);
}

bool MTSimulatorDataExporter::OpenDir(const std::string& folderName)
{
	// Convert std::string to std::wstring because Windows API functions often use wide characters
	std::wstring wideFolderName(folderName.begin(), folderName.end());

	// Create the directory
	BOOL result = CreateDirectoryW(wideFolderName.c_str(), nullptr);
	bool res = false;
	if (result == 0) {
		// If creation fails, use GetLastError to get more info about the error
		DWORD error = GetLastError();
		if (error == ERROR_ALREADY_EXISTS) {
			res = true;
		}
		else {
			res = false;
		}
	}
	else {
		res = true;
	}
	return res;

}

void MTSimulatorDataExporter::ExportCurrentAccountState(double accountBalance, double equityBalance, double usedMargin, double freeMargin, double floatingPnL, double currProfit,long long currTimePoint)
{
	// Json builder 
	nlohmann::json accountStateJson;
	accountStateJson[AccountStateTag_AccountBalance] = accountBalance;
	accountStateJson[AccountStateTag_EquityBalance] = equityBalance;
	accountStateJson[AccountStateTag_UsedMargin] = usedMargin;
	accountStateJson[AccountStateTag_FreeMargin] = freeMargin;
	accountStateJson[AccountStateTag_FloatingPnL] = floatingPnL;
	accountStateJson[AccountStateTag_CurrProfit] = currProfit;
	accountStateJson[Tag_SimulationID] = m_simulationId;
	accountStateJson[AccountStateTag_CurrTimePoint] = currTimePoint; // Current time as timestamp
	m_accountStateExporter->AddDataItem(accountStateJson);

}

