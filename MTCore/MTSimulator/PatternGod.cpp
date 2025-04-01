#include "PatternGod.h"
#include "OrdersToExecute.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include "ScoreCalcs.h"
#include "Configrations.h"
#include "Logger.h"
#include <mutex>
#include "PatternExporter.h"
#include <future>
#include "..\..\HTTPCore\ExchangeData.h"
#define MAX_DIFF 4

enum NodeDirection {
	D_Stable=0,    // Defaults to 0
	D_Up,    // Defaults to 1
	D_Down     // Defaults to 2
};


void PatternGod::Send_For_PNGExport(std::vector<ChangeNode*>* potentioalRes)
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
		int indexStart = m_historyDB_Map.GetIdx_Of_TimePoint(potentioalRes->at(i)->point_DateTime);
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

std::string PatternGod::GetFormattedCurrentTime()
{
	// Get current time as time_point
	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	// Convert to tm structure
	std::tm now_tm;
#ifdef _WIN32
	localtime_s(&now_tm, &now_time_t); // Use localtime_s for thread safety in Windows
#else
	localtime_r(&now_time_t, &now_tm); // Use localtime_r for thread safety in Unix/Linux
#endif

	// Create a string stream to format the output
	std::ostringstream oss;
	oss << std::put_time(&now_tm, "%Y%m%d%H%M%S") << std::setw(3) << std::setfill('0') << milliseconds.count();

	return oss.str();
}

bool PatternGod::Trade_Decession_Logic(DirectionStats stats)
{
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();

	double upPercent = (double(  stats.generalUpCount) /   stats.generalTotal) * 100.0;
	double downPercent = (double(stats.generalDownCount) / stats.generalTotal) * 100.0;
	double blurPercent = (double(stats.generalBlurCount) / stats.generalTotal) * 100.0;

	double meanPips = stats.mean_general();

	if (stats.generalTotal >= policy->Patter_Min_Patterns_ToTake_Action)
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

void PatternGod::Send_TraddeSignal(std::string pair, int type)
{
	ExchangeData().Set_NewOrder("XAUUSD", type);
	m_orderCode++; Cur_Orders_InRow++;
}

int PatternGod::DetectCommonDirection(std::vector<ChangeNode*>* potentioalRes)
{
	if (!potentioalRes || potentioalRes->empty())
		return -1;

	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
	DirectionStats stats;

	for (ChangeNode* node : *potentioalRes)
	{
		int generalPipChange = 0;
		int firstPipChange = 0;

		CommonDirection generalDir = CalcGeneralPipsChange(node, &stats,policy->Pattern_Min_PipsChange_ToCount);
		CommonDirection firstDir = CalcFirstPipsChange(node,&stats ,policy->Pattern_Min_PipsChange_ToCount );
		
	}
	PrepareAndSend_FinalDecLog(stats, potentioalRes);
	Send_For_PNGExport(potentioalRes);

	return -1;
}

CommonDirection PatternGod::CalcFirstPipsChange(ChangeNode* start, DirectionStats* stats, int pipsChangeToCount)
{
	int indexStart = m_historyDB_Map.GetIdx_Of_TimePoint(start->point_DateTime);
	int futureLen = Configrations::getInstance()->Get_Policy()->Score_Future_Vision_Candles;
	int pipChange = 0;
	int candlesCounter = 0;
	bool upD = false, downD = false;
	for (int i = indexStart; i < indexStart + futureLen && i < m_historyTL->size(); i++)
	{
		pipChange += m_historyTL->at(i)->Pip_Change;
		candlesCounter++;
		if (pipChange >= pipsChangeToCount) 
		{
			upD = true; 
			break;
		
		}
		if (pipChange <= -pipsChangeToCount)
		{
			downD = true;
			break;
		}
	}
	CommonDirection dire;
	if (upD) dire = CommonDirection::UP;
	else if (downD) dire = CommonDirection::DOWN;
	else dire = CommonDirection::BLUR;

	stats->add_first(pipChange, dire, candlesCounter);
	return dire;
}


CommonDirection PatternGod::CalcGeneralPipsChange(ChangeNode* start,DirectionStats* stats, int pipsChangeToCount)
{
	int indexStart = m_historyDB_Map.GetIdx_Of_TimePoint(start->point_DateTime);
	int futureLen = Configrations::getInstance()->Get_Policy()->Score_Future_Vision_Candles;
	int pipChange = 0;
	int candlesCounter = 0;

	bool upD = false, downD = false;
	
	for (int i = indexStart; i < indexStart + futureLen && i < m_historyTL->size(); i++)
	{
		candlesCounter++;
		pipChange += m_historyTL->at(i)->Pip_Change;
		if (pipChange >= pipsChangeToCount)
		{
			upD = true;
		}

		if (pipChange <= -pipsChangeToCount) {
			downD = true;
		}

		if (upD && downD) break;
	}

	CommonDirection dire;
	if(upD) dire = CommonDirection::UP;
	else if (downD) dire = CommonDirection::DOWN;
	else dire = CommonDirection::BLUR;

	stats->add_general(pipChange, dire, candlesCounter);
	return dire;
}

void PatternGod::BuildDBHistoryMap()
{
	for (int i = 0; i < m_historyTL->size(); i++)
	{
		const std::string& timeKey = m_historyTL->at(i)->point_DateTime;
		if (!m_historyDB_Map.ContainsKey(timeKey)) {
			m_historyDB_Map.InsetIntoMap(timeKey, i);
		}
		else {
			Logger::getInstance().WriteLog("!!!!! Duplicate point_DateTime in DB: " + timeKey + " !!!!!!", false, true);
		}
	}
}
void PatternGod::Export_DataToCSV(DirectionStats stats)
{

	double general_upPercent = (double(stats.generalUpCount) / stats.generalTotal) * 100.0;
	double general_downPercent = (double(stats.generalDownCount) / stats.generalTotal) * 100.0;
	double general_blurPercent = (double(stats.generalBlurCount) / stats.generalTotal) * 100.0;

	double first_upPercent = (double(stats.firstUpCount) / stats.firstTotal) * 100.0;
	double first_downPercent = (double(stats.firstDownCount) / stats.firstTotal) * 100.0;
	double first_blurPercent = (double(stats.firstBlurCount) / stats.firstTotal) * 100.0;
	
	CSVExportData exData("ordersSummery.csv");
	exData.InsertNewColumn(std::to_string(general_upPercent));
	exData.InsertNewColumn(std::to_string(general_downPercent));
	exData.InsertNewColumn(std::to_string(general_blurPercent));
	exData.InsertNewColumn(std::to_string(stats.mean_general()));
	exData.InsertNewColumn(std::to_string(stats.stddev_general()));
	exData.InsertNewColumn(std::to_string(stats.meanIndexes_general()));


	exData.InsertNewColumn(std::to_string(first_upPercent));
	exData.InsertNewColumn(std::to_string(first_downPercent));
	exData.InsertNewColumn(std::to_string(first_blurPercent));
	exData.InsertNewColumn(std::to_string(stats.mean_first()));
	exData.InsertNewColumn(std::to_string(stats.stddev_first()));
	exData.InsertNewColumn(std::to_string(stats.meanIndexes_first()));

	exData.NewRow();
	exData.DoneInsertingData();
}
ScoresResult PatternGod::CheckForOneMinNodeMatch(ChangeNode* historyNode, ChangeNode* currentNode)
{
	ScoresResult score;
	// Calc all the scores
	//score.basicScore = BasincCandleScoreCalcs::CalcScore(historyNode, currentNode);
	score.basicScore = 0;
	score.rsiScore = RsiScoreCalcs::Calc_AllLensScore(historyNode, currentNode); 
	score.emaScore = EmaScoreCalcs::Calc_AllLensScore(historyNode, currentNode);
	score.zlsmaScore = ZlsmaScoreCalcs::Calc_AllLensScore(historyNode, currentNode);
	//score.gravityScore = GravityScoreCalcs::CalcScore(historyNode, currentNode); 

	// Sum all the scores into one Object
	score.calcTotalResult();
	return score;
}


ScoresResult PatternGod::CompareOneMinNodesPeriods(int startIdx) // TODO: compare between periods of time in the history time line and the whole current time line
{
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
	ScoresResult periodScore;

	int currentSize = static_cast<int>(m_currentTL->size());
	int historyRemaining = static_cast<int>(m_historyTL->size()) - startIdx;
	int limit = (currentSize < historyRemaining) ? currentSize : historyRemaining;

	for (int i = 0; i < limit; ++i)
	{
		periodScore += CheckForOneMinNodeMatch(m_currentTL->at(i), m_historyTL->at(startIdx + i));
	}

	return periodScore;
}

int PatternGod::ScanForOneMinPatters(std::vector<ChangeNode*>* resultNodes)
{
	Logger& logger = Logger::getInstance();
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
	int scoreToAct = static_cast<int>(m_currentTL->size()) * static_cast<int>(policy->Score_Min_TotalScore_ToAct_PerCandle);
	ScoresResult maxScore;
	maxScore.totalScore.total = 0;
	logger.WriteLog("Score to Act: " + std::to_string(scoreToAct),false,true);
	for (int i = 1; i < m_historyTL->size(); i++)
	{
		ScoresResult resScore = CompareOneMinNodesPeriods(i);
		if (resScore.totalScore.total > scoreToAct)// need to check node building 
		{
			
			if (i + m_currentTL->size() < m_historyTL->size())
			{
				resultNodes->push_back(m_historyTL->at(i + m_currentTL->size()));
			}
		}
		if (maxScore.totalScore.total < resScore.totalScore.total)
			maxScore = resScore;
	}
	logger.WriteLog("Max Total Score found: " + std::to_string(maxScore.totalScore.total), false, true);
	logger.WriteLog("Attached RSI Score: " + std::to_string(maxScore.rsiScore.total), false, true);
	logger.WriteLog("Attached EMA Score: " + std::to_string(maxScore.emaScore.total), false, true);
	logger.WriteLog("Attached Basic Score: " + std::to_string(maxScore.basicScore), false, true);
	logger.WriteLog("Patterns Found: " + std::to_string(resultNodes->size()), false, true);
	return static_cast<int>(resultNodes->size());
}
int PatternGod::ScanForOneMinPatterns_MultiThreaded(std::vector<ChangeNode*>* resultNodes)
{
		// Mutex for thread-safe access to shared resources
	std::mutex loggerMutex, resultMutex, local2DResult;

	std::vector < std::pair< ChangeNode*, ScoresResult>> resultNodes2D;
	Logger& logger = Logger::getInstance();
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
	int scoreToAct = static_cast<int> (m_currentTL->size()) * static_cast<double> (policy->Score_Min_TotalScore_ToAct_PerCandle);
	ScoresResult maxScore;

	int threadCount = std::thread::hardware_concurrency(); // Number of threads
	int historySize = static_cast<int>(m_historyTL->size());
	int chunkSize = historySize / threadCount;

	logger.WriteLog("Score to Act: " + std::to_string(scoreToAct), false, true);

	// Function to process a chunk of the loop
	auto processChunk = [&](int start, int end, int threadIndex) {
		ScoresResult localMaxScore;
		std::vector<ChangeNode*> localResults;
		std::vector<std::pair< ChangeNode*, ScoresResult>> localResults2D;

		for (int i = start; i < end; i++) {
			ScoresResult resScore = CompareOneMinNodesPeriods(i);
			if (resScore.totalScore.total > scoreToAct) {
				if (i + m_currentTL->size() < m_historyTL->size()) {
					localResults.push_back(m_historyTL->at(i + m_currentTL->size()));
					localResults2D.push_back(std::pair<ChangeNode*, ScoresResult>(m_historyTL->at(i + m_currentTL->size()), ScoresResult(resScore)));
				}
			}
			else
			{
				i += 10;
			}
			if (localMaxScore.totalScore.total < resScore.totalScore.total)
				localMaxScore = resScore;
		}

		// Update shared resources safely
		{
			std::lock_guard<std::mutex> lock(loggerMutex);
			logger.WriteLog("Thread [" + std::to_string(threadIndex) + "] Done Processing...");
		}

		{
			std::lock_guard<std::mutex> lock(resultMutex);
			resultNodes->insert(resultNodes->end(), localResults.begin(), localResults.end());
			
			if (maxScore.totalScore.total < localMaxScore.totalScore.total)
				maxScore = localMaxScore;
		}

		{
			std::lock_guard<std::mutex> lock(local2DResult);
			resultNodes2D.insert(resultNodes2D.end(), localResults2D.begin(), localResults2D.end());
		}
	};

	// Launch threads
	std::vector<std::thread> threads;
	for (int t = 0; t < threadCount; ++t) {
		int start = t * chunkSize;
		int end = (t == threadCount - 1) ? historySize : (t + 1) * chunkSize;
		threads.emplace_back(processChunk, start, end, t); // Pass thread index as `t`
	}

	// Join threads
	for (auto& th : threads) {
		th.join();
	}
	logger.WriteLog("Sorting Scores...");
	SortScores(resultNodes2D);
	logger.WriteLog("Filtering duplicates time points...");
	FilterDuplicatesTimeLines(resultNodes2D);
	logger.WriteLog("Filtering Top" + std::to_string(policy->Patter_Max_Patterns_To_Find) + "scores...",false,true);
	filterByHighScores(resultNodes2D);
	resultNodes->clear();
	for (int i = 0; i < resultNodes2D.size(); i++)
	{
		resultNodes->push_back(resultNodes2D.at(i).first);
	}

	std::string allTimePoints = "All paterns time points:\n";
	for (const auto& result : resultNodes2D)
	{
		const std::string& time = result.first->point_DateTime;
		const ScoresResult& score = result.second;

		allTimePoints +=
			"\nTimePoint: " + time + "\n" +
			"    | Total Score: " + std::to_string(score.totalScore.total) + "\n" +
			"    | EMA: " + std::to_string(score.emaScore.total) + "\n" +
			"    | RSI: " + std::to_string(score.rsiScore.total) + "\n"
			"    | ZLSMA: " + std::to_string(score.zlsmaScore.total);
	}

	logger.WriteLog(allTimePoints, false, true);

	return static_cast<int>(resultNodes->size());

}

void PatternGod::PrepareAndSend_FinalDecLog(DirectionStats& stats, std::vector<ChangeNode*>* potentioalRes)
{

	double general_upPercent = (double(stats.generalUpCount) / stats.generalTotal) * 100.0;
	double general_downPercent = (double(stats.generalDownCount) / stats.generalTotal) * 100.0;
	double general_blurPercent = (double(stats.generalBlurCount) / stats.generalTotal) * 100.0;


	double first_upPercent = (double(stats.firstUpCount) / stats.firstTotal) * 100.0;
	double first_downPercent = (double(stats.firstDownCount) / stats.firstTotal) * 100.0;
	double first_blurPercent = (double(stats.firstBlurCount) / stats.firstTotal) * 100.0;

	bool tradeDecRes = Trade_Decession_Logic(stats); // Take a deccession to enter a trade

	Logger::getInstance().WriteLog("Patterns count found: " + std::to_string(potentioalRes->size()), false, true);

	std::string general_summeryLog = "Stats Summary (General decession calcs !!):\n" +
		std::string("UP: ") + std::to_string(general_upPercent) + "%\n" +
		"DOWN: " + std::to_string(general_downPercent) + "%\n" +
		"NOISE: " + std::to_string(general_blurPercent) + "%\n" +
		"Mean Pip Change: " + std::to_string(stats.mean_general()) + "\n" +
		"StdDev Pip Change: " + std::to_string(stats.stddev_general()) +"\n" +
		"Mean indexes: " + std::to_string(stats.meanIndexes_general());

	std::string first_summeryLog = "Stats Summary (First decession calcs !!):\n" +
		std::string("UP: ") + std::to_string(first_upPercent) + "%\n" +
		"DOWN: " + std::to_string(first_downPercent) + "%\n" +
		"NOISE: " + std::to_string(first_blurPercent) + "%\n" +
		"Mean Pip Change: " + std::to_string(stats.mean_first()) + "\n" +
		"StdDev Pip Change: " + std::to_string(stats.stddev_first()) + "\n" +
		"Mean indexes: " + std::to_string(stats.meanIndexes_first());

	auto indexes_first = stats.get_Indexes_and_changePips_First();
	auto indexes_general = stats.get_Indexes_and_changePips_General();

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


	Logger::getInstance().WriteLog(general_summeryLog, false, true);
	Logger::getInstance().WriteLog(indexesSummery_first, false, true);
	Logger::getInstance().WriteLog(first_summeryLog, false, true);
	Logger::getInstance().WriteLog(indexesSummery_general, false, true);

	if (tradeDecRes) Logger::getInstance().WriteLog(general_summeryLog, false, true, LogType::SPECIAL_LOG);
	if (tradeDecRes) Logger::getInstance().WriteLog(indexesSummery_general, false, true, LogType::SPECIAL_LOG);

	if (tradeDecRes) Logger::getInstance().WriteLog(first_summeryLog, false, true, LogType::SPECIAL_LOG);
	if (tradeDecRes) Logger::getInstance().WriteLog(indexesSummery_first, false, true, LogType::SPECIAL_LOG);
	if (tradeDecRes) Export_DataToCSV(stats);

}

void PatternGod::filterByHighScores(std::vector<std::pair< ChangeNode*, ScoresResult>>& nodesResults2D)
{
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
	if (nodesResults2D.size() <= policy->Patter_Max_Patterns_To_Find) {
		// If the vector has 10 or fewer elements, nothing needs to be done
		return;
	}

	// Use partial_sort to sort the vector and get top 10 scores
	std::partial_sort(nodesResults2D.begin(), nodesResults2D.begin() + policy->Patter_Max_Patterns_To_Find, nodesResults2D.end(),
		[](const std::pair<ChangeNode*, ScoresResult>& a, const std::pair<ChangeNode*, ScoresResult>& b) {
			return a.second.totalScore.total > b.second.totalScore.total; // Sort by 'total' in descending order
		});

	// Resize the vector to keep only the top 10 elements
	nodesResults2D.resize(policy->Patter_Max_Patterns_To_Find); 


	double maxScore = nodesResults2D.front().second.totalScore.total;
	double thresholdRatio = policy->Pattern_TopScore_ThresholdRatio; // score that is at least 95% of the max score
	
	std::vector<std::pair<ChangeNode*, ScoresResult>> topResults;

	for (const auto& result : nodesResults2D) {
		if (result.second.totalScore.total >= maxScore * thresholdRatio)
			topResults.push_back(result);
		else
			break;
	}

	nodesResults2D = std::move(topResults);

}

void PatternGod::FilterDuplicatesTimeLines(std::vector<std::pair<ChangeNode*, ScoresResult>>& nodesResults2D)
{
	if (nodesResults2D.empty()) return;

	std::vector<std::pair<ChangeNode*, ScoresResult>> filtered;
	std::unordered_set<int> usedIndices;

	for (const auto& result : nodesResults2D)
	{
		int currIdx = m_historyDB_Map.GetIdx_Of_TimePoint(result.first->point_DateTime);
		bool isDuplicate = false;

		// בדוק רק אם קיים אינדקס קרוב בטווח של 30
		for (int offset = -29; offset <= 29; ++offset)
		{
			if (usedIndices.find(currIdx + offset) != usedIndices.end()) {
				isDuplicate = true;
				break;
			}
		}

		if (!isDuplicate)
		{
			filtered.push_back(result);
			usedIndices.insert(currIdx);
		}
	}

	nodesResults2D = std::move(filtered);
}

void PatternGod::SortScores(std::vector<std::pair< ChangeNode*, ScoresResult>>& nodesResults2D)
{
	if (nodesResults2D.size() <= 10) {
		// If the vector has 10 or fewer elements, nothing needs to be done
		return;
	}

	// Use partial_sort to sort the vector and get top 10 scores
	std::partial_sort(nodesResults2D.begin(), nodesResults2D.begin() + (nodesResults2D.size() -1), nodesResults2D.end(),
		[](const std::pair<ChangeNode*, ScoresResult>& a, const std::pair<ChangeNode*, ScoresResult>& b) {
			return a.second.totalScore.total > b.second.totalScore.total; // Sort by 'total' in descending order
		});

	// Implement a logic to to filter by time sessions !!

}


bool Time_Check(std::string histTimePoint,std::string currTimePoint,TimeWindow timeWin) // YYYYMMDDHHMM, Not completed function!!!, DO NOT USE IT
{
	//TODO: Need to check if the date is valid
	std::string histTimeToCheck;
	std::string currTimeToCheck;
	int hist;
	int curr;

	switch (timeWin)
	{
	case w_Year:
		histTimeToCheck += histTimePoint[4];
		histTimeToCheck += histTimePoint[5];

		currTimeToCheck += currTimePoint[4];
		currTimeToCheck += currTimePoint[5];

		hist = std::stoi(histTimeToCheck);
		curr = std::stoi(currTimeToCheck);

		if (abs(hist - curr) <= 4) // Need to implement config for the diff
			return true;
		break;
	case w_Month:
		histTimeToCheck += histTimePoint[6];
		histTimeToCheck += histTimePoint[7];

		currTimeToCheck += currTimePoint[6];
		currTimeToCheck += currTimePoint[7];

		hist = std::stoi(histTimeToCheck);
		curr = std::stoi(currTimeToCheck);

		if (abs(hist - curr) <= 10) // TODO: Need to implement a config for the diff
			return true;
		break;
	case w_Day:
		histTimeToCheck += histTimePoint[8];
		histTimeToCheck += histTimePoint[9];

		currTimeToCheck += currTimePoint[8];
		currTimeToCheck += currTimePoint[9];

		hist = std::stoi(histTimeToCheck);
		curr = std::stoi(currTimeToCheck);

		if (abs(hist - curr) <= 4) // TODO: Need to implement a config for the diff
			return true;
		break;
	default:
		break;
	}
	return false;
}