#include "DataExporter.h"

std::mutex DataExporter::m_jsonFileLocker;
std::mutex DataExporter::m_csvFileLocker;


void DataExporter::Export_All_Statistics_Data(DirectionStats& stats, bool tradeDecRes)
{
	ExportDataLayerPolicy* policy = Configrations::getInstance()->Get_Policy<ExportDataLayerPolicy>(PolicyType::ExportDataLayerPolicy);
	Logger& logger = Logger::getInstance();
	std::vector<std::string> summery_logs = PrepareSummeryLogs(stats); // Prepare all summary logs in vector

	for (int i = 0; i < summery_logs.size(); i++)
	{
		logger.WriteLog(summery_logs.at(i), false, true); // Send summary logs to logs
		if (tradeDecRes)Logger::getInstance().WriteLog(summery_logs.at(i), false, true, LogType::SPECIAL_LOG); // if there is a trade, send also to SPECIAL log
	}

	if (policy->Enable_Patterns_PNG_Export)
	{
		logger.WriteLog("Start export pngs...");
		Send_For_PNGExport(stats);
		logger.WriteLog("Done export pngs");
	}

	logger.WriteLog("Done export all data");

}

std::vector<std::string> DataExporter::PrepareSummeryLogs(DirectionStats& stats)
{
	int totalCount_general = stats.Get_Total(StatsType::General);
	auto generalDirectionMap = stats.Get_VotedPercentages(StatsType::General);

	double general_upPercent =		generalDirectionMap[CommonDirection::UP];
	double general_downPercent =	generalDirectionMap[CommonDirection::DOWN];
	double general_blurPercent =	generalDirectionMap[CommonDirection::BLUR];

	int totalCount_first = stats.Get_Total(StatsType::First);
	auto firstDirectionMap = stats.Get_VotedPercentages(StatsType::First);

	double first_upPercent =	firstDirectionMap[CommonDirection::UP];
	double first_blurPercent =	firstDirectionMap[CommonDirection::BLUR];
	double first_downPercent =	firstDirectionMap[CommonDirection::DOWN];

	int totalCount_highest = stats.Get_Total(StatsType::Highest);
	auto highestDirectionMap = stats.Get_VotedPercentages(StatsType::Highest);

	double highest_upPercent = highestDirectionMap[CommonDirection::UP];
	double highest_downPercent = highestDirectionMap[CommonDirection::DOWN];
	double highest_blurPercent = highestDirectionMap[CommonDirection::BLUR];

	Logger::getInstance().WriteLog("Patterns count found: " + std::to_string(totalCount_general), false, true);

	std::string general_summeryLog = "Stats Summary (General decession calcs !!):\n" +
		std::string("Time Point: ") + stats.Get_TimePoint() + "\n" +
		std::string("Patterns Count: ") + std::to_string(totalCount_general) + "\n" +
		std::string("UP: ") + std::to_string(general_upPercent) + "%\n" +
		"DOWN: " + std::to_string(general_downPercent) + "%\n" +
		"NOISE: " + std::to_string(general_blurPercent) + "%\n" +
		"Mean Pip Change: " + std::to_string(stats.mean(StatsType::General)) + "\n" +
		"StdDev Pip Change: " + std::to_string(stats.stddev(StatsType::General)) + "\n" +
		"Mean indexes: " + std::to_string(stats.meanIndexes(StatsType::General)) + "\n\n\n" +
		"Trade res:" + (stats.Get_TradeRes() ? " True" : " False");

	std::string first_summeryLog = "Stats Summary (First decession calcs !!):\n" +
		std::string("Time Point: ") + stats.Get_TimePoint() + "\n" +
		std::string("UP: ") + std::to_string(first_upPercent) + "%\n" +
		"DOWN: " + std::to_string(first_downPercent) + "%\n" +
		"NOISE: " + std::to_string(first_blurPercent) + "%\n" +
		"Mean Pip Change: " + std::to_string(stats.mean(StatsType::First)) + "\n" +
		"StdDev Pip Change: " + std::to_string(stats.stddev(StatsType::First)) + "\n" +
		"Mean indexes: " + std::to_string(stats.meanIndexes(StatsType::First));

	std::string highest_summeryLog = "Stats Summary (Highest decession calcs !!):\n" +
		std::string("Time Point: ") + stats.Get_TimePoint() + "\n" +
		std::string("UP: ") + std::to_string(highest_upPercent) + "%\n" +
		"DOWN: " + std::to_string(highest_downPercent) + "%\n" +
		"NOISE: " + std::to_string(highest_blurPercent) + "%\n" +
		"Mean Pip Change: " + std::to_string(stats.mean(StatsType::Highest)) + "\n" +
		"StdDev Pip Change: " + std::to_string(stats.stddev(StatsType::Highest)) + "\n" +
		"Mean indexes: " + std::to_string(stats.meanIndexes(StatsType::Highest));


	auto indexes_first = stats.Get_Indexes_and_changePips(StatsType::First);
	auto indexes_general = stats.Get_Indexes_and_changePips(StatsType::General);
	auto indexes_highest = stats.Get_Indexes_and_changePips(StatsType::Highest);

	std::string indexesSummery_first = "Indexes Summery (first)\n";
	std::string indexesSummery_general = "Indexes Summery (general)\n";
	std::string indexesSummery_highest = "Indexes Summery (highest)\n";

	for (int i = 0; i < indexes_first.size(); i++)
	{
		indexesSummery_first += std::to_string(i) + std::string(": ") + std::to_string(indexes_first.at(i).first) + std::string(" | ")
			+ std::to_string(indexes_first.at(i).second) + std::string("\n");
	}

	for (int i = 0; i < indexes_general.size(); i++)
	{
		indexesSummery_general += std::to_string(i) + std::string(": ") + std::to_string(indexes_general.at(i).first) + std::string(" | ")
			+ std::to_string(indexes_general.at(i).second) + std::string("\n");
	}

	for (int i = 0; i < indexes_highest.size(); i++)
	{
		indexesSummery_highest += std::to_string(i) + std::string(": ") + std::to_string(indexes_highest.at(i).first) + std::string(" | ")
			+ std::to_string(indexes_highest.at(i).second) + std::string("\n");
	}

	general_summeryLog += std::string("\n\n") + indexesSummery_general;
	first_summeryLog += std::string("\n\n") + indexesSummery_first;
	highest_summeryLog += std::string("\n\n") + indexesSummery_highest;

	std::string generalStepsMeansLog = "*General* Steps Means details:\n\n";
	auto generalStepMeans_vec = stats.GetAllMeansUpTo(CommonDirection::NONE);
	for (int i = 0; i < generalStepMeans_vec.size(); i++)
	{
		generalStepsMeansLog += "Index " + std::to_string(i + 1) + " : " + std::to_string(generalStepMeans_vec.at(i)) + "\n";
	}


	std::string positiveStepsMeansLog = "*Positive* Steps Means details:\n\n";
	auto positiveStepMeans_vec = stats.GetAllMeansUpTo(CommonDirection::UP);
	for (int i = 0; i < positiveStepMeans_vec.size(); i++)
	{
		positiveStepsMeansLog += "Index " + std::to_string(i + 1) + " : " + std::to_string(positiveStepMeans_vec.at(i)) + "\n";
	}


	std::string negativeStepsMeansLog = "*Negative* Steps Means details:\n\n";
	auto negativeStepMeans_vec = stats.GetAllMeansUpTo(CommonDirection::DOWN);
	for (int i = 0; i < negativeStepMeans_vec.size(); i++)
	{
		negativeStepsMeansLog += "Index " + std::to_string(i + 1) + " : " + std::to_string(negativeStepMeans_vec.at(i)) + "\n";
	}

	std::vector<std::string> summery_logs;
	summery_logs.push_back(general_summeryLog);
	summery_logs.push_back(first_summeryLog);
	summery_logs.push_back(highest_summeryLog);
	summery_logs.push_back(generalStepsMeansLog);
	summery_logs.push_back(positiveStepsMeansLog);
	summery_logs.push_back(negativeStepsMeansLog);
	return summery_logs;
}

void DataExporter::Send_For_PNGExport(DirectionStats& stats)
{
	std::vector<std::vector<ChangeNode*>* > patternsVec;
	TimeLine* currPattern = stats.Get_CurrPattern();
	std::vector<TimeLineScored>* similarPatterns = stats.Get_SimilarPatterns();
	for (int i = 0; i < similarPatterns->size(); i++)
	{
		patternsVec.push_back(similarPatterns->at(i).Get_TimeLine().Get_TimeLine());
	}
	PatternExporter::Export_All_Pattens_ToPNG(currPattern->Get_TimeLine(), &patternsVec);
}

