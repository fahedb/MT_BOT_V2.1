#include "DataExporter.h"

std::mutex DataExporter::m_jsonFileLocker;
std::mutex DataExporter::m_csvFileLocker;


void DataExporter::Export_All_Statistics_Data(DirectionStats& stats, bool tradeDecRes)
{
	std::vector<std::string> summery_logs = PrepareSummeryLogs(stats); // Prepare all summery logs in vector

	for (int i = 0; i < summery_logs.size(); i++)
	{
		Logger::getInstance().WriteLog(summery_logs.at(i), false, true); // Send summery logs to logs
		if (tradeDecRes)Logger::getInstance().WriteLog(summery_logs.at(i), false, true, LogType::SPECIAL_LOG); // if there is a trade, send also to SPECIAL log
	}
	
	Export_DataToJson(stats, false);
	Send_For_PNGExport(stats);
	if (tradeDecRes) Export_DataToJson(stats, true);

	if (tradeDecRes) Export_DataToCSV(stats);
}

std::vector<std::string> DataExporter::PrepareSummeryLogs(DirectionStats& stats)
{
	int totalCount_general = stats.Get_TotalCount(StatsType::General);
	double general_upPercent = (double(stats.Get_UpCount(StatsType::General)) / totalCount_general) * 100.0;
	double general_downPercent = (double(stats.Get_DownCount(StatsType::General)) / totalCount_general) * 100.0;
	double general_blurPercent = (double(stats.Get_BlurCount(StatsType::General)) / totalCount_general) * 100.0;

	int totalCount_first = stats.Get_TotalCount(StatsType::First);
	double first_upPercent = (double(stats.Get_UpCount(StatsType::First)) / totalCount_first) * 100.0;
	double first_downPercent = (double(stats.Get_DownCount(StatsType::First)) / totalCount_first) * 100.0;
	double first_blurPercent = (double(stats.Get_BlurCount(StatsType::First)) / totalCount_first) * 100.0;

	Logger::getInstance().WriteLog("Patterns count found: " + std::to_string(stats.Get_TotalCount(StatsType::General)), false, true);

	std::string general_summeryLog = "Stats Summary (General decession calcs !!):\n" +
		std::string("Time Point: ") + stats.timePoint + "\n" +
		std::string("Patterns Count: ") + std::to_string(stats.Get_TotalCount(StatsType::General)) + "\n" +
		std::string("UP: ") + std::to_string(general_upPercent) + "%\n" +
		"DOWN: " + std::to_string(general_downPercent) + "%\n" +
		"NOISE: " + std::to_string(general_blurPercent) + "%\n" +
		"Mean Pip Change: " + std::to_string(stats.mean(StatsType::General)) + "\n" +
		"StdDev Pip Change: " + std::to_string(stats.stddev(StatsType::General)) + "\n" +
		"Mean indexes: " + std::to_string(stats.meanIndexes(StatsType::General)) + "\n\n\n" +
		"Trade res:" + (stats.m_tradeRes ? " True" : " False");

	std::string first_summeryLog = "Stats Summary (First decession calcs !!):\n" +
		std::string("Time Point: ") + stats.timePoint + "\n" +
		std::string("UP: ") + std::to_string(first_upPercent) + "%\n" +
		"DOWN: " + std::to_string(first_downPercent) + "%\n" +
		"NOISE: " + std::to_string(first_blurPercent) + "%\n" +
		"Mean Pip Change: " + std::to_string(stats.mean(StatsType::First)) + "\n" +
		"StdDev Pip Change: " + std::to_string(stats.stddev(StatsType::First)) + "\n" +
		"Mean indexes: " + std::to_string(stats.meanIndexes(StatsType::First));

	auto indexes_first = stats.get_Indexes_and_changePips(StatsType::First);
	auto indexes_general = stats.get_Indexes_and_changePips(StatsType::General);

	std::string indexesSummery_first = "Indexes Summery (first)\n";
	std::string indexesSummery_general = "Indexes Summery (general)\n";

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
	general_summeryLog += std::string("\n\n") + indexesSummery_general;
	first_summeryLog += std::string("\n\n") + indexesSummery_first;

	std::string stepsMeansLog = "Steps Means details:\n\n";
	auto stepMeans_vec = stats.GetAllMeansUpTo();
	for (int i = 0; i < stepMeans_vec.size(); i++)
	{
		stepsMeansLog += "Index " + std::to_string(i + 1) + " : " + std::to_string(stepMeans_vec.at(i)) + "\n";
	}


	std::vector<std::string> summery_logs;
	summery_logs.push_back(general_summeryLog);
	summery_logs.push_back(first_summeryLog);
	summery_logs.push_back(stepsMeansLog);
	return summery_logs;
}

void DataExporter::Export_DataToJson(DirectionStats& stats, bool tradeDecRes)
{
	auto j = JasonHandler::Build_Json_ForStats(stats);
	std::string filePath = "C:\\MTDB\\";
	filePath += "AllSummery.json";
	std::lock_guard<std::mutex> lock(m_jsonFileLocker);
	std::ofstream file(filePath, std::ios::app);
	file << j.dump() << "\n";
	file.close();

	if (tradeDecRes)
	{
		std::string ordersfilePath = "C:\\MTDB\\";
		ordersfilePath += "OrderSummery.json";
		std::ofstream ordersFile(ordersfilePath, std::ios::app);
		ordersFile << j.dump() << "\n";
		ordersFile.close();
	}

}

void DataExporter::Export_DataToCSV(DirectionStats& stats)
{
	int totalCount_general = stats.Get_TotalCount(StatsType::General);
	double general_upPercent = (double(stats.Get_UpCount(StatsType::General)) / totalCount_general) * 100.0;
	double general_downPercent = (double(stats.Get_DownCount(StatsType::General)) / totalCount_general) * 100.0;
	double general_blurPercent = (double(stats.Get_BlurCount(StatsType::General)) / totalCount_general) * 100.0;

	int totalCount_first = stats.Get_TotalCount(StatsType::First);
	double first_upPercent = (double(stats.Get_UpCount(StatsType::First)) / totalCount_first) * 100.0;
	double first_downPercent = (double(stats.Get_DownCount(StatsType::First)) / totalCount_first) * 100.0;
	double first_blurPercent = (double(stats.Get_BlurCount(StatsType::First)) / totalCount_first) * 100.0;

	std::lock_guard<std::mutex> lock(m_csvFileLocker);

	CSVExportData exData("ordersSummery.csv");
	exData.InsertNewColumn(stats.timePoint);

	exData.InsertNewColumn(std::to_string(general_upPercent));
	exData.InsertNewColumn(std::to_string(general_downPercent));
	exData.InsertNewColumn(std::to_string(general_blurPercent));
	exData.InsertNewColumn(std::to_string(stats.mean(StatsType::General)));
	exData.InsertNewColumn(std::to_string(stats.stddev(StatsType::General)));
	exData.InsertNewColumn(std::to_string(stats.meanIndexes(StatsType::General)));


	exData.InsertNewColumn(std::to_string(first_upPercent));
	exData.InsertNewColumn(std::to_string(first_downPercent));
	exData.InsertNewColumn(std::to_string(first_blurPercent));
	exData.InsertNewColumn(std::to_string(stats.mean(StatsType::First)));
	exData.InsertNewColumn(std::to_string(stats.stddev(StatsType::First)));
	exData.InsertNewColumn(std::to_string(stats.meanIndexes(StatsType::First)));

	exData.NewRow();
	exData.DoneInsertingData();
}

void DataExporter::Send_For_PNGExport(DirectionStats& stats)
{
	std::vector<std::vector<ChangeNode*>* > patternsVec;
	for (int i = 0; i < stats.m_patterns.size(); i++)
	{
		patternsVec.push_back(stats.m_patterns.at(i).Get_TimeLine());
	}
	PatternExporter::Export_All_Pattens_ToPNG(stats.m_currPattern->Get_TimeLine(), &patternsVec);
}

