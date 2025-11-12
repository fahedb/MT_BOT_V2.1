#include "StatisticsLayerDataExporter.h"
#include "LayersJsonKeys.h"

void StatisticsLayerDataExporter::AddStatisticsData(DirectionStats* stats)
{
	if (!stats)
		return;
	nlohmann::json statsJson;
	BuildVotedDirectionsJson(statsJson, stats, StatsType::General);
	BuildVotedDirectionsJson(statsJson, stats, StatsType::First);
	BuildVotedDirectionsJson(statsJson, stats, StatsType::Highest);

	BuildStepsMeanJson(statsJson, stats, CommonDirection::UP);
	BuildStepsMeanJson(statsJson, stats, CommonDirection::DOWN);

	m_dataSummary.push_back(statsJson);
}

void StatisticsLayerDataExporter::BuildVotedDirectionsJson(nlohmann::json& parentJson, DirectionStats* stats, StatsType sType)
{
	if (!stats)
		return;
	VotedDirectionStats& dir = stats->GetGroupStats(sType);

	auto votedPercentages = dir.GetVotedPercentages();
	parentJson[stats->Get_TimePoint()][JsonKey_VotedDirections][StatsType_Utils::GetStatsTypeStr(sType)] = {
		{ JsonKey_TotalVotes,				dir.getTotal() },
		{ JsonKey_UP_Percent,				static_cast<int>(votedPercentages[CommonDirection::UP]) },
		{ JsonKey_DOWN_Percent,			static_cast<int>(votedPercentages[CommonDirection::DOWN]) },
		{ JsonKey_BLUR_Percent,			static_cast<int>(votedPercentages[CommonDirection::BLUR]) },
		{ JsonKey_MeanPipsChange,			static_cast<int>(dir.mean()) },
		{ JsonKey_PureMeanPipsChange_UP,	static_cast<int>(dir.pure_mean(CommonDirection::UP)) },
		{ JsonKey_PureMeanPipsChange_DOWN,	static_cast<int>(dir.pure_mean(CommonDirection::DOWN)) },
		{ JsonKey_StdDevPipsChange,		static_cast<int>(dir.stddev()) },
		{ JsonKey_MeanIndexes,				static_cast<int>(dir.meanIndexes()) }
	};

}

void StatisticsLayerDataExporter::BuildStepsMeanJson(nlohmann::json& parentJson, DirectionStats* stats, CommonDirection commonDir)
{
	StepsMean& stepsMean = stats->GetStepMeans(commonDir);
	std::vector<double> means = stepsMean.GetAllMeansUpTo();
	std::string commonDirStr = CommonDirection_Utils::GetCommonDirectionStr(commonDir);

	for (int i = 0; i < means.size(); i++)
		parentJson[stats->Get_TimePoint()][commonDirStr + JsonKey_StepsMean_Suffix].push_back(static_cast<int>(means[i]));
}
