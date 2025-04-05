#include "StatisticsDecession.h"
#include "JasonHandler.h"

void StatisticsDecession::Send_For_PNGExport(std::vector<ChangeNode*>* potentioalRes)
{
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
	if (!m_currentTL)
	{
		Logger::getInstance().WriteLog("Error: No Current TimeLine!!\nExport To PNG canceled");
		return;
	}
	std::vector<std::vector<ChangeNode*>*> similarPatterns;
	int timeLineLen = static_cast<int>(m_currentTL->size());
	for (int i = 0; i < potentioalRes->size(); i++)
	{
		int indexStart = m_historyDB_Map->GetIdx_Of_TimePoint(potentioalRes->at(i)->point_DateTime);
		std::vector<ChangeNode*>* currVector = new std::vector<ChangeNode*>(m_historyTL->begin() + static_cast<size_t>(indexStart - timeLineLen), m_historyTL->begin() + static_cast<size_t>(indexStart + policy->Score_Future_Vision_Candles));
		similarPatterns.push_back(currVector);
	}
	PatternExporter::Export_All_Pattens_ToPNG(m_currentTL, &similarPatterns);

	for (int i = 0; i < similarPatterns.size(); i++)
	{
		similarPatterns.at(i)->clear();
	}
	similarPatterns.clear();
}

void StatisticsDecession::Export_DataToJson(DirectionStats& stats, bool tradeDecRes)
{
	auto j = JasonHandler::Build_Json_ForStats(stats);
	std::string filePath = "C:\\MTDB\\";
	if (tradeDecRes)
		filePath += "OrderSummery.json";
	else
		filePath += "AllSummery.json";
	std::ofstream file(filePath, std::ios::app);
	file << j.dump() << "\n";
	file.close();
}

std::vector<std::string> StatisticsDecession::PrepareSummeryLogs(DirectionStats& stats)
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
		std::string("Patterns Count: ") + std::to_string(stats.Get_TotalCount(StatsType::General)) + "\n" +
		std::string("UP: ") + std::to_string(general_upPercent) + "%\n" +
		"DOWN: " + std::to_string(general_downPercent) + "%\n" +
		"NOISE: " + std::to_string(general_blurPercent) + "%\n" +
		"Mean Pip Change: " + std::to_string(stats.mean(StatsType::General)) + "\n" +
		"StdDev Pip Change: " + std::to_string(stats.stddev(StatsType::General)) + "\n" +
		"Mean indexes: " + std::to_string(stats.meanIndexes(StatsType::General));

	std::string first_summeryLog = "Stats Summary (First decession calcs !!):\n" +
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

void StatisticsDecession::Export_DataToCSV(DirectionStats& stats)
{
	int totalCount_general = stats.Get_TotalCount(StatsType::General);
	double general_upPercent = (double(stats.Get_UpCount(StatsType::General)) / totalCount_general) * 100.0;
	double general_downPercent = (double(stats.Get_DownCount(StatsType::General)) / totalCount_general) * 100.0;
	double general_blurPercent = (double(stats.Get_BlurCount(StatsType::General)) / totalCount_general) * 100.0;

	int totalCount_first = stats.Get_TotalCount(StatsType::First);
	double first_upPercent = (double(stats.Get_UpCount(StatsType::First)) / totalCount_first) * 100.0;
	double first_downPercent = (double(stats.Get_DownCount(StatsType::First)) / totalCount_first) * 100.0;
	double first_blurPercent = (double(stats.Get_BlurCount(StatsType::First)) / totalCount_first) * 100.0;

	CSVExportData exData("ordersSummery.csv");
	exData.InsertNewColumn(m_currentTL->at(m_currentTL->size() - 1)->point_DateTime);

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

bool StatisticsDecession::Trade_Decession_Logic(DirectionStats& stats)
{
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();

	int generalCount = stats.Get_TotalCount(StatsType::General);
	double upPercent = (double(stats.Get_UpCount(StatsType::General)) / generalCount) * 100.0;
	double downPercent = (double(stats.Get_DownCount(StatsType::General)) / generalCount) * 100.0;
	double blurPercent = (double(stats.Get_BlurCount(StatsType::General)) / generalCount) * 100.0;

	double meanPips = stats.mean(StatsType::General);

	if (generalCount >= policy->Patter_Min_Patterns_ToTake_Action)
	{
		bool isStrongUp = upPercent > policy->Pattern_Min_Percent_To_ExecuteOrder
			&& (meanPips) >= policy->Min_MeanPips_ToExecute_Order;

		bool isStrongDown = downPercent > policy->Pattern_Min_Percent_To_ExecuteOrder
			&& (meanPips) <= -policy->Min_MeanPips_ToExecute_Order;

		if (isStrongUp)
		{
			Send_TraddeSignal("XAUUSD", 0);
			return true;
		}
		else if (isStrongDown)
		{
			Send_TraddeSignal("XAUUSD", 1);
			return true;
		}
		else if (policy->Pattern_Activate_Alternative_ExecuteOrder)
		{
			bool isAlternativeAllowed = blurPercent <= policy->Pattern_Max_Noise_To_ExecuteOrder_Alternative
				&& std::fabs(downPercent - upPercent) >= policy->Pattern_Diff_Percent_To_ExecuteOrder_Alternative;

			if (isAlternativeAllowed)
			{
				bool isAlternativeUp = upPercent > downPercent && (meanPips) >= (policy->Min_MeanPips_ToExecute_Order + 10);
				bool isAlternativeDown = downPercent > upPercent && (meanPips) <= -(policy->Min_MeanPips_ToExecute_Order + 10);

				if (isAlternativeUp)
				{
					Send_TraddeSignal("XAUUSD", 0);
					return true;
				}
				else if (isAlternativeDown)
				{
					Send_TraddeSignal("XAUUSD", 1);
					return true;
				}
			}
		}
	}

	Cur_Orders_InRow = 0;
	return false;

}
