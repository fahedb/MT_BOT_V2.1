#include "StatisticsLayerDataImporter.h"

QueueStruct<DirectionStats> StatisticsLayerDataImporter::Create_QueueItem(const std::string& line, long lineIndex)
{
	// Example line:
	// {"20250602010000000":{"DOWN_StepsMean":[-46.0,-60.5,-147.0,-194.0,-142.0,-252.0,-275.0],"UP_StepsMean":[58.0,17.0,57.0,82.0,100.0,148.5,162.5],"VotedDirections":{"First":{"BLUR_Percent":0.0,"DOWN_Percent":59.72027972027972,"MeanIndexes":4.333333333333333,"MeanPipsChange":67.0,"PureMeanPipsChange_DOWN":-49.0,"PureMeanPipsChange_UP":116.0,"StdDevPipsChange":157.61345120261785,"TotalVotes":3,"UP_Percent":40.27972027972028},"General":{"BLUR_Percent":0.0,"DOWN_Percent":59.72027972027972,"MeanIndexes":7.0,"MeanPipsChange":16.666666666666668,"PureMeanPipsChange_DOWN":-91.66666666666667,"PureMeanPipsChange_UP":108.33333333333333,"StdDevPipsChange":212.85728134649804,"TotalVotes":3,"UP_Percent":40.27972027972028},"Highest":{"BLUR_Percent":0.0,"DOWN_Percent":59.72027972027972,"MeanIndexes":6.0,"MeanPipsChange":24.666666666666668,"PureMeanPipsChange_DOWN":-91.66666666666667,"PureMeanPipsChange_UP":116.33333333333333,"StdDevPipsChange":216.35207931106697,"TotalVotes":3,"UP_Percent":40.27972027972028}}}}
	nlohmann::json jsonData;
	try {
		jsonData = nlohmann::json::parse(line);
	}
	catch (const std::exception& e) {
		// Handle parsing error (e.g., log the error)
		throw std::runtime_error("StatisticsLayerDataImporter::Create_QueueItem : JSON parsing error at line " + std::to_string(lineIndex) + ": " + e.what());
	}
	DirectionStats dirStats;
	dirStats.Activate();
	auto it = jsonData.begin();
	dirStats.Set_TimePoint(it.key());

	std::string commonDirStr = CommonDirection_Utils::GetCommonDirectionStr(CommonDirection::UP);
	dirStats.GetStepMeans(CommonDirection::UP).SetImportedMeans(it.value().at(commonDirStr + JsonKey_StepsMean_Suffix).get<std::vector<double>>());

	auto& votedJson = it.value().at(JsonKey_VotedDirections);
	ParseAndSetVotedStats(votedJson.at(StatsType_Utils::GetStatsTypeStr(StatsType::First)), StatsType::First, dirStats);
	ParseAndSetVotedStats(votedJson.at(StatsType_Utils::GetStatsTypeStr(StatsType::General)), StatsType::General, dirStats);
	ParseAndSetVotedStats(votedJson.at(StatsType_Utils::GetStatsTypeStr(StatsType::Highest)), StatsType::Highest, dirStats);
}

void StatisticsLayerDataImporter::ParseAndSetVotedStats(const nlohmann::json& votedJson, StatsType sType, DirectionStats& dirStats)
{
	// JsonKey_TotalVotes				
	// JsonKey_UP_Percent				
	// JsonKey_DOWN_Percent			
	// JsonKey_BLUR_Percent			
	// JsonKey_MeanPipsChange			
	// JsonKey_PureMeanPipsChange_UP	
	// JsonKey_PureMeanPipsChange_DOWN 
	// JsonKey_StdDevPipsChange		
	// JsonKey_MeanIndexes				
	VotedDirectionStats& vStats = dirStats.GetGroupStats(sType);
	int totalVotes = votedJson.at(JsonKey_TotalVotes).get<int>();
	
	double upPercent = votedJson.at(JsonKey_UP_Percent).get<double>();
	double downPercent = votedJson.at(JsonKey_DOWN_Percent).get<double>();
	double blurPercent = votedJson.at(JsonKey_BLUR_Percent).get<double>();

	double meanPipChange = votedJson.at(JsonKey_MeanPipsChange).get<double>();
	double purePipChange_UP = votedJson.at(JsonKey_PureMeanPipsChange_UP).get<double>();
	double purePipChange_DOWN = votedJson.at(JsonKey_PureMeanPipsChange_DOWN).get<double>();
	double stdev = votedJson.at(JsonKey_StdDevPipsChange).get<double>();
	double meanIdx = votedJson.at(JsonKey_MeanIndexes).get<double>();

	vStats.ImportStatsData(upPercent, downPercent, blurPercent,
		meanIdx, meanPipChange,
		purePipChange_UP, purePipChange_DOWN,
		stdev, totalVotes);
}

void StatisticsLayerDataImporter::ParseAndSetStepMeans(const nlohmann::json& stepsMeanJson,CommonDirection dir, DirectionStats& dirStats)
{
	dirStats.GetStepMeans(dir).SetImportedMeans(stepsMeanJson.get<std::vector<double>>());
}
