#include "PatternGod.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include "ScoreCalcs.h"
#include "Configrations.h"
#include "Logger.h"
#include <mutex>
#include <future>

#define MAX_DIFF 4

enum NodeDirection {
	D_Stable=0,    // Defaults to 0
	D_Up,    // Defaults to 1
	D_Down     // Defaults to 2
};


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


void PatternGod::BuildDBHistoryMap() // Function to build a map for the time points indexes - not used any more, moved the logic to TimeLine object
{
	for (int i = 0; i < m_historyTL->size(); i++)
	{
		const std::string& timeKey = m_historyTL->at(i)->point_DateTime;
		if (!m_historyDB_Map->ContainsKey(timeKey)) {
			m_historyDB_Map->InsetIntoMap(timeKey, i);
		}
		else {
			Logger::getInstance().WriteLog("!!!!! Duplicate point_DateTime in DB: " + timeKey + " !!!!!!", false, true,LogType::SPECIAL_LOG);
		}
	}
}


ScoresResult PatternGod::CompareOneMinNodesPeriods(int startIdx) // compare between periods of time in the history time line and the whole current time line
{
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
	ScoresResult periodScore;

	int currentSize = static_cast<int>(m_currentTL->size());
	int historyRemaining = static_cast<int>(m_historyTL->size()) - startIdx;
	int limit = (currentSize < historyRemaining) ? currentSize : historyRemaining;
	int currTotalScore = 0;
	int maxScorePossible = policy->maxScorePossibleForChangeNodes;
	int totalMinScoretoAct = limit * policy->Score_Min_TotalScore_ToAct_PerCandle;

	for (int i = 0; i < limit; ++i)
	{
		periodScore += ProxyScoreCalcs::CompareChangeNodes(m_currentTL->at(i), m_historyTL->at(static_cast<size_t>(startIdx) + static_cast<size_t>(i)));
		currTotalScore += periodScore.totalScore.total;

		int remainingCandles = limit - i;
		int maxFutureScore = remainingCandles * maxScorePossible;
		if (currTotalScore + maxFutureScore < totalMinScoretoAct)
			break;

	}

	return periodScore;
}

int PatternGod::ScanForOneMinPatterns_MultiThreaded(std::vector<ChangeNode*>* resultNodes) // Compare every X candles of the history to the current timeLine
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
		int estimatedItems = (end - start) / 5 + 10;
		ScoresResult localMaxScore;
		
		std::vector<ChangeNode*> localResults;
		localResults.reserve(estimatedItems);

		std::vector<std::pair< ChangeNode*, ScoresResult>> localResults2D;
		localResults2D.reserve(estimatedItems);
		
		for (int i = start; i < end; i++) {
			ScoresResult resScore = CompareOneMinNodesPeriods(i);
			int localScoreToAct = scoreToAct;
			if (resScore.totalScore.total > localScoreToAct) {
				if (i + m_currentTL->size() < m_historyTL->size()) {
					localResults.push_back(m_historyTL->at(i + m_currentTL->size() - 1)); // Get the end of the period
					localResults2D.push_back(std::pair<ChangeNode*, ScoresResult>(m_historyTL->at(i + m_currentTL->size() - 1), ScoresResult(resScore)));
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
	SortScores(resultNodes2D);
	FilterDuplicatesTimeLines(resultNodes2D);
	filterByHighScores(resultNodes2D);
	resultNodes->clear();
	for (int i = 0; i < resultNodes2D.size(); i++)
	{
		resultNodes->push_back(resultNodes2D.at(i).first);
	}

	std::ostringstream logStream;
	logStream << "All patterns time points:\n";

	for (const auto& result : resultNodes2D) {
		const auto& time = result.first->point_DateTime;
		const auto& score = result.second;

		logStream
			<< "\nTimePoint: " << time << "\n"
			<< "    | Total Score: " << score.totalScore.total << "\n"
			<< "    | EMA: " << score.emaScore.total << "\n"
			<< "    | RSI: " << score.rsiScore.total << "\n"
			<< "    | ZLSMA: " << score.zlsmaScore.total;
	}

	logger.WriteLog(logStream.str(), false, true);

	return static_cast<int>(resultNodes->size());

}

void PatternGod::filterByHighScores(std::vector<std::pair< ChangeNode*, ScoresResult>>& nodesResults2D)
{
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
	if (nodesResults2D.size() <= policy->Patter_Max_Patterns_To_Find) {
		// If the vector has X or fewer elements, nothing needs to be done
		return;
	}

	// Use partial_sort to sort the vector and get top X scores
	std::partial_sort(nodesResults2D.begin(), nodesResults2D.begin() + policy->Patter_Max_Patterns_To_Find, nodesResults2D.end(),
		[](const std::pair<ChangeNode*, ScoresResult>& a, const std::pair<ChangeNode*, ScoresResult>& b) {
			return a.second.totalScore.total > b.second.totalScore.total; // Sort by 'total' in descending order
		});

	// Resize the vector to keep only the top 10 elements
	nodesResults2D.resize(policy->Patter_Max_Patterns_To_Find); 


	double maxScore = nodesResults2D.front().second.totalScore.total;
	double thresholdRatio = policy->Pattern_TopScore_ThresholdRatio; // score that is at least 95% of the max score
	
	std::vector<std::pair<ChangeNode*, ScoresResult>> topResults;

	for (const auto& result : nodesResults2D) { // Filter all scores less than X% threshold ratio from the maxScore found
		if (result.second.totalScore.total >= maxScore * thresholdRatio)
			topResults.push_back(result);
		else
			break;
	}

	nodesResults2D = std::move(topResults);

}

void PatternGod::FilterDuplicatesTimeLines(std::vector<std::pair<ChangeNode*, ScoresResult>>& nodesResults2D) // Filter out all timePoints that is in the same 30minuts
{
	if (nodesResults2D.empty()) return;

	std::vector<std::pair<ChangeNode*, ScoresResult>> filtered;
	std::unordered_set<int> usedIndices;

	for (const auto& result : nodesResults2D)
	{
		int currIdx = m_historyDB_Map->GetIdx_Of_TimePoint(result.first->point_DateTime);
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

void PatternGod::SortScores(std::vector<std::pair< ChangeNode*, ScoresResult>>& nodesResults2D) // Sort the scores from the high to the low, usuallu need to do this after filtering them
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


bool Time_Check(std::string histTimePoint,std::string currTimePoint,TimeWindow timeWin) // YYYYMMDDHHMM, Not completed function!!!, DO NOT USE IT YET !!!
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