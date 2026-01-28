#pragma once
#include "GenericJsonExporter.h"
#include "OrderStruct.h"
#include "ChangeData.h"
#include "DealStruct.h"
#include "SimulatorActiveOrder.h"
#include "DirectionStats.h"

class MTSimulatorDataExporter
{
private:
	GenericJsonExporter* m_dealsDataExporter = nullptr; // Exporter for closed deals data
	GenericJsonExporter* m_accountStateExporter = nullptr; // Exporter for account state data
	GenericJsonExporter* m_directionStatsExporter = nullptr; // Exporter for direction statistics data
	int m_simulationId;
public:
	MTSimulatorDataExporter(const std::string& exporterName, int simulationId, const std::string& dstPathTo, int batchSize= 100)
	{
		m_simulationId = simulationId;
		OpenDir(dstPathTo);
		m_dealsDataExporter = new GenericJsonExporter(exporterName + "_ClosedDeals", dstPathTo + "\\" + exporterName + "_ClosedDeals.json", batchSize);
		m_accountStateExporter = new GenericJsonExporter(exporterName + "_AccountState", dstPathTo + "\\" + exporterName + "_AccountState.json", batchSize);
		m_directionStatsExporter = new GenericJsonExporter(exporterName + "_DirectionStats", dstPathTo + "\\" + exporterName + "_DirectionStats.json", batchSize);
	}
	void ExportCurrentSimulationState(double accountBalance, double equityBalance, double usedMargin, double freeMargin, double floatingPnL, double currProfit, const Deal& closedDeal); // Export current simulation state data
	void ExportDirectionStatsData(DirectionStats* stats); // Export direction statistics data

private:
	void ExportCurrentAccountState(double accountBalance, double equityBalance, double usedMargin, double freeMargin, double floatingPnL, double currProfit, long long currTimePoint);
	void ExportClosedDealsData(const Deal& closedDeal);

	void ExportDirectionStatsVotedPercentages_Helper(DirectionStats* stats, nlohmann::json& json,  StatsType statsT);
	void ExportPatterns_Helper(DirectionStats* stats, nlohmann::json& json);

	bool OpenDir(const std::string& folderName);

};

