#include "ComputeUnitRunner.h"
#include "..\MTCore\MTSimulator_DataExporter\LayersJsonKeys.h"
#include "../MTCore/MTSimulator_Configurations/ConfigsDefs.h"
#include "Converts.h"

void ComputeUnitRunner::LoadAllConfigsFromHTTPServer(ComputeUnitConfigs& computeUnitConfigs)
{
	std::vector<ConfigStruct> configs = m_httpServer->Get_RequestHandler()->Get_AllConfigs(true);
	computeUnitConfigs.EnableScoreCaching = Get_ConfigValue(configs, CONFIG_DEF_ENABLE_SCORE_CACHEING) == "true";
	computeUnitConfigs.scanFilters = Get_ConfigValue(configs, CONFIG_DEF_SCAN_FILTERS);
	computeUnitConfigs.historyPathFile = Get_ConfigValue(configs, "HISTORY_FILE_PATH", "C:\\");
	computeUnitConfigs.polyLen = Converts::ToInt(Get_ConfigValue(configs, CONFIGS_POLYNOM_LEN_FOR_SCAN));
	computeUnitConfigs.wEval = Converts::ToDouble(Get_ConfigValue(configs, CONFIGS_POLYNOM_EVAL_WEIGHT));
	computeUnitConfigs.wRMSE = Converts::ToDouble(Get_ConfigValue(configs, CONFIGS_POLYNOM_RMSE_WEIGHT));
	computeUnitConfigs.wCoeff = Converts::ToDouble(Get_ConfigValue(configs, CONFIGS_POLYNOM_COEFF_WEIGHT));
	computeUnitConfigs.currentTimeLineSize = Converts::ToInt(Get_ConfigValue(configs, CONFIG_DEF_CURR_TIME_LINE_LEN));
	computeUnitConfigs.EnablePolynomScan = Get_ConfigValue(configs, CONFIG_DEF_ENABLE_POLYNOM_SCAN) == "true";
	computeUnitConfigs.EnablePolynomTimeLineCaching = Get_ConfigValue(configs, CONFIG_DEF_ENABLE_POLYNOM_TIME_LINE_CACHING) == "true";
}

void ComputeUnitRunner::LoadHistoryData(std::vector<ChangeNode*>& historyTimeLine)
{
	XAUUSD_OneMinTable table(m_computeUnitConfigs.historyPathFile);
	HistoryTimeLineLoader loader(historyTimeLine, &table, m_computeUnitConfigs.historyPathFile);
	loader.Run();
}

void ComputeUnitRunner::LoadCurrentTimeLine(std::vector<ChangeNode*>& currentTimeLine, bool waitTillTimeLineIsFull)
{
	XAUUSD_CurrentTable table;
	while (waitTillTimeLineIsFull && currentTimeLine.size() < m_computeUnitConfigs.currentTimeLineSize)
	{
		HTTPComputeServerSyncer::UpdateCurrentTimeLine(&table, m_httpServer);
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep for a short time to avoid busy waiting
		m_httpServer->Get_RequestHandler()->Wait_For_NewBar_M1();
	}
	currentTimeLine = *Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLine();
}

void ComputeUnitRunner::SubmitScanResult(std::string currTP, std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D)
{
	nlohmann::json j;
	for (int i = 0; i < resultNodes2D.size(); i++)
	{
		j[currTP][JsonKey_ScoredTimePoints].push_back({
			{JsonKey_datetime, resultNodes2D[i].first->point_DateTime},
			{JsonKey_score, resultNodes2D[i].second.totalScore.total}
			});
	}
	m_httpServer->Get_RequestHandler()->Notify_ScanResult_For_NewBar(j.dump());
}

std::string ComputeUnitRunner::Get_ConfigValue(const std::vector<ConfigStruct>& configs, const std::string& label, const std::string& defaultValue)
{
	for (const auto& config : configs)
	{
		if (config.label == label)
		{
			return config.value;
		}
	}
	return defaultValue; // Return default value if the config is not found
}
