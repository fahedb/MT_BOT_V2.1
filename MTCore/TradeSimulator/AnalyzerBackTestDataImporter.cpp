#include "AnalyzerBackTestDataImporter.h"

void AnalyzerBackTestDataImporter::BuildBackTestAnalyzeDB()
{
	Static_DBConnection* dbConn = Static_DBConnection::getInstance();
	std::string dbPathToFiles = ProccessFlags::GetSwitch("-backTestAnalyzeDBPath");
	std::string simulationName = ProccessFlags::GetSwitch("-backTestAnalyzeSimulationName");

	// Example: MTSimExporter_Sim1_ClosedDeals
	std::string closedDealsFilePath = dbPathToFiles + "\\MTSimExporter_" + simulationName + "_ClosedDeals.json";
	GenericJsonImporterToSQLDB closedDealsImporter(dbConn->GetConnection()->Get_EnvHandler(), dbConn->GetConnection()->Get_DBConnection(), Table_BackTest_Deals, closedDealsFilePath);


	// Example: MTSimExporter_Sim1_AccountState
	std::string accountStateFilePath = dbPathToFiles + "\\MTSimExporter_" + simulationName + "_AccountState.json";
	GenericJsonImporterToSQLDB accountStateImporter(dbConn->GetConnection()->Get_EnvHandler(), dbConn->GetConnection()->Get_DBConnection(), Table_BackTest_AccountStateEvents, accountStateFilePath);

	//Example: MTSimExporter_Sim1_DirectionStats
	std::string directionStatsFilePath = dbPathToFiles + "\\MTSimExporter_" + simulationName + "_DirectionStats.json";
	GenericJsonImporterToSQLDB directionStatsImporter(dbConn->GetConnection()->Get_EnvHandler(), dbConn->GetConnection()->Get_DBConnection(), Table_BackTest_DirectionStats, directionStatsFilePath);

	// NOTE: If needed, can add more tables to import here

	InitAccountStateTable(accountStateImporter);
	InitAccountStateTable(accountStateImporter);
	InitDirectionStatsTable(directionStatsImporter);

	closedDealsImporter.ImportDataToSQLDB();
	accountStateImporter.ImportDataToSQLDB();
	directionStatsImporter.ImportDataToSQLDB();

}
void AnalyzerBackTestDataImporter::InitClosedDealsTable(GenericJsonImporterToSQLDB& importer)
{
	importer.MapRootPath(Deals_Column_SimulationID, { Tag_SimulationID });
	importer.MapRootPath(Deals_Column_LotSize, { DealTag_LotSize });
	importer.MapRootPath(Deals_Column_OpenPrice, { DealTag_OpenPrice });
	importer.MapRootPath(Deals_Column_ClosePrice, { DealTag_ClosePrice });
	importer.MapRootPath(Deals_Column_TakeProfitPrice, { DealTag_TakeProfitPrice });
	importer.MapRootPath(Deals_Column_StopLossPrice, { DealTag_StopLossPrice });
	importer.MapRootPath(Deals_Column_Type, { DealTag_Type });
	importer.MapRootPath(Deals_Column_Profit, { DealTag_Profit });
	importer.MapRootPath(Deals_Column_ProfitInPips, { DealTag_ProfitInPips });
	importer.MapRootPath(Deals_Column_CloseReason, { DealTag_CloseReason });
	importer.MapRootPath(Deals_Column_NumOfBarsHeld, { DealTag_NumOfBarsHeld });

}
void AnalyzerBackTestDataImporter::InitAccountStateTable(GenericJsonImporterToSQLDB& importer)
{
	importer.MapRootPath(AccountState_Column_SimulationID, { Tag_SimulationID });
	importer.MapRootPath(AccountState_Column_CurrTimePoint, { AccountStateTag_CurrTimePoint });
	importer.MapRootPath(AccountState_Column_AccountBalance, { AccountStateTag_AccountBalance });
	importer.MapRootPath(AccountState_Column_EquityBalance, { AccountStateTag_EquityBalance });
	importer.MapRootPath(AccountState_Column_UsedMargin, { AccountStateTag_UsedMargin });
	importer.MapRootPath(AccountState_Column_FreeMargin, { AccountStateTag_FreeMargin });
	importer.MapRootPath(AccountState_Column_FloatingPnL, { AccountStateTag_FloatingPnL });
	importer.MapRootPath(AccountState_Column_CurrProfit, { AccountStateTag_CurrProfit });
}


void AnalyzerBackTestDataImporter::InitDirectionStatsTable(GenericJsonImporterToSQLDB& importer)
{
	// 1) Common root fields (same for all 3 rows)
	importer.MapRootPath(DirectionStats_Column_SimulationID, { Tag_SimulationID });
	importer.MapRootPath(DirectionStats_Column_TimePoint, { DirectionStatsTag_TimePoint });
	importer.MapRootPath(DirectionStats_Column_CoinsPair, { DirectionStatsTag_Pair });
	importer.MapRootPath(DirectionStats_Column_TradeResult, { DirectionStatsTag_TradeResult });

	// Optional: if you want the root TotalVotes (the one at top-level)
	importer.MapRootPath(DirectionStats_Column_TotalVotes, { DirectionStatsTag_TotalVotes });

	// 2) Variant fields (pulled from the variant object)
	importer.MapVariantPath(DirectionStats_Column_TotalVotes, { DirectionStatsTag_VeotedPercentages_TotalVotes });
	importer.MapVariantPath(DirectionStats_Column_UpPercentage, { DirectionStatsTag_VeotedPercentages_UP_Percent });
	importer.MapVariantPath(DirectionStats_Column_DownPercentage, { DirectionStatsTag_VeotedPercentages_DOWN_Percent });
	importer.MapVariantPath(DirectionStats_Column_BlurPercentage, { DirectionStatsTag_VeotedPercentages_BLUR_Percent });
	importer.MapVariantPath(DirectionStats_Column_Mean, { DirectionStatsTag_VeotedPercentages_Mean });
	importer.MapVariantPath(DirectionStats_Column_PureMean_Up, { DirectionStatsTag_VeotedPercentages_PureMean_UP });
	importer.MapVariantPath(DirectionStats_Column_PureMean_Down, { DirectionStatsTag_VeotedPercentages_PureMean_DOWN });
	importer.MapVariantPath(DirectionStats_Column_StdDev, { DirectionStatsTag_VeotedPercentages_StdDev });
	importer.MapVariantPath(DirectionStats_Column_MeanIndexes, { DirectionStatsTag_VeotedPercentages_MeanIndexes });

	// 3) Define variants (3 rows per JSON)
	importer.AddVariant("General", { DirectionStatsTag_General_VotedPercentages });
	importer.AddVariant("First", { DirectionStatsTag_First_VotedPercentages });
	importer.AddVariant("Highest", { DirectionStatsTag_Highest_VotedPercentages });

	// 4) VoteType constant per variant
	importer.SetVariantConstant("General", DirectionStats_Column_VoteType, "General");
	importer.SetVariantConstant("First", DirectionStats_Column_VoteType, "First");
	importer.SetVariantConstant("Highest", DirectionStats_Column_VoteType, "Highest");
}