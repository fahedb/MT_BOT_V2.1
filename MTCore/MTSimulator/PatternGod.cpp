#include "PatternGod.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include "ScoreCalcs.h"
#include "Configrations.h"
#include "Logger.h"
#include <mutex>
#include <future>
#include "ScoreResultsCache.h"
#include "ThreadPool.h"
#include "MTSimulatorUtils.h"

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
			Logger::getInstance().WriteLog("!!!!! Duplicate point_DateTime in DB: " + timeKey + " !!!!!!", false, true,LogType::ERR);
		}
	}
}


ScoresResult PatternGod::CompareOneMinNodesPeriods(int startIdx,int threadIdx) // compare between periods of time in the history time line and the whole current time line
{
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
	ScoresResult periodScore;

	int currentSize = static_cast<int>(m_currentTL->size());
	int historyRemaining = static_cast<int>(m_historyTL->size()) - startIdx;
	int limit = (currentSize < historyRemaining) ? currentSize : historyRemaining;
	int currTotalScore = 0;
	int maxScorePossible = policy->maxScorePossibleForChangeNodes;
	int totalMinScoretoAct = limit * policy->Score_Min_TotalScore_ToAct_PerCandle;
	ScoreResultsCache* scoreResultsCache = ScoreResultsCache::getInstance();
	std::vector<std::pair<std::pair<long long, long long>, ScoresResult>> scoreResults;

	scoreResults.reserve(limit); // Reserve space for the expected number of results to avoid multiple reallocations
	const bool enableCaching = policy->EnableScoreCaching;


	for (int i = 0; i < limit; ++i)
	{
		int remainingCandles = limit - i;
		int maxFutureScore = remainingCandles * maxScorePossible;
		if (currTotalScore + maxFutureScore < totalMinScoretoAct) // If the current total score plus the maximum possible future score is less than the minimum score to act, skip further calculations
			break;

		if (enableCaching)
		{
			ScoresResult* cachedScore = nullptr;
			try {
				cachedScore = scoreResultsCache->GetScoreResult(m_historyTL->at(static_cast<size_t>(startIdx) + static_cast<size_t>(i))->point_DateTimeLong, m_currentTL->at(i)->point_DateTimeLong, threadIdx);
			}
			catch (const std::exception& e) {
				Logger::getInstance().WriteLog("Error in getting cached result: " + std::string(e.what()), false, false, LogType::ERR);
				return ScoresResult(); // Return an empty ScoresResult on error
			}

			if (cachedScore) // If score already calculated, use it
			{
				periodScore += *cachedScore;
				currTotalScore += cachedScore->totalScore.total;
				continue;
			}

		}
		ScoresResult myScore;
		try {
			// Compare the change nodes and calculate the score
			// This function should be defined in ProxyScoreCalcs or similar utility class
			// It compares two ChangeNode objects and returns a ScoresResult object
			// If an error occurs, it will throw an exception which we catch below
			myScore = ProxyScoreCalcs::CompareChangeNodes(m_currentTL->at(i), m_historyTL->at(static_cast<size_t>(startIdx) + static_cast<size_t>(i)));
		}
		catch (const std::exception& e) {
			Logger::getInstance().WriteLog("Error in comparing change nodes: " + std::string(e.what()), false, false, LogType::ERR);
			return ScoresResult(); // Return an empty ScoresResult on error
		}

		periodScore += myScore;
		currTotalScore += myScore.totalScore.total;

		if (enableCaching)
		{
			try {
				// Cache the score result for future insertion in the ScoreResultsCache singletone class
				ScoreResultsCacheUtils::CacheScoreResult_InTempVector(scoreResults, m_historyTL->at(static_cast<size_t>(startIdx) + static_cast<size_t>(i))->point_DateTimeLong, m_currentTL->at(i)->point_DateTimeLong, myScore);
			}
			catch (const std::exception& e) {
				Logger::getInstance().WriteLog("Error in caching score result: " + std::string(e.what()), false, false, LogType::ERR);
				return ScoresResult(); // Return an empty ScoresResult on error
			}
		}
	}

	if (enableCaching)
	{
		ScoreResultsCacheUtils::CacheScoreResult_ToCache(scoreResults, threadIdx);
	}

	return periodScore;
}

int PatternGod::ScanForOneMinPatterns_MultiThreaded(std::vector < std::pair< ChangeNode*, ScoresResult>>& resultNodes2D) // Compare every X candles of the history to the current timeLine
{
		// Mutex for thread-safe access to shared resources
	std::mutex resultMutex, local2DResultMutex;

	Logger& logger = Logger::getInstance();
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
	int scoreToAct = static_cast<int> (m_currentTL->size()) * static_cast<double> (policy->Score_Min_TotalScore_ToAct_PerCandle);
	ScoresResult maxScore;

	int threadCount = ThreadPool::getInstance()->Get_ThreadsNum()	; // Number of threads
	ResourceManager::getInstance()->Set_Data(THREADS_COUNT, std::to_string(threadCount)); // Set the number of threads in the ResourceManager

	int historySize = static_cast<int>(m_historyTL->size());
	int chunkSize = historySize / threadCount;

	logger.WriteLog("Score to Act: " + std::to_string(scoreToAct), false, true);

	std::vector<std::vector < std::pair< ChangeNode*, ScoresResult>>> resultNodesPerThread(threadCount);
	ThreadPool* tPool = ThreadPool::getInstance();

	Logger::getInstance().WriteLog("Scan Started", false, true, LogType::LOG); // Log before stop low priority tasks

	tPool->Set_PauseLowPriorityTasks(true);
	std::vector<std::future<void>> futures;
	for (int t = 0; t < threadCount; ++t) {

		int start = t * chunkSize;
		int end = (t == threadCount - 1) ? historySize : (t + 1) * chunkSize;

		auto prom = std::make_shared<std::promise<void>>();
		futures.push_back(prom->get_future());
		std::vector<std::pair<ChangeNode*, ScoresResult>>* localResultsNodes = &resultNodesPerThread[t];
		tPool->enqueueTask(
			[this, t, start, end, scoreToAct, &resultMutex, &local2DResultMutex, &maxScore, &resultNodes2D, localResultsNodes, prom]() {
				this->ProccessScanChunk(start, end, t, scoreToAct, &resultMutex, &maxScore, localResultsNodes);
				prom->set_value();
			},
			TaskPriority::High
		);
	}

	for (auto& f : futures)
		f.get(); // Wait till the end
	Logger::getInstance().WriteLog("Scan Ended", false, true, LogType::LOG);
	tPool->Set_PauseLowPriorityTasks(false);
	MTSimulatorUtils::MergeResultsNodes(&resultNodes2D, &resultNodesPerThread);

	SortScores(resultNodes2D);
	FilterDuplicatesTimeLines(resultNodes2D);
	filterByHighScores(resultNodes2D);


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
			<< "    | ZLSMA: " << score.zlsmaScore.total << "\n"
			<< "	| VOLUME: " << score.volumeScore.total;
	}

	logger.WriteLog(logStream.str(), false, true);

	return static_cast<int>(resultNodes2D.size());

}

// Function to process a chunk of the loop
void PatternGod::ProccessScanChunk(int startIdx, int endIdx, int threadIndex, int scoreToAct,
	std::mutex* maxResultLock,
	ScoresResult* maxScore,std::vector<std::pair< ChangeNode*, ScoresResult>>* resultNodesPerThread) {

	int estimatedItems = 20;
	Logger& logger = Logger::getInstance();
	resultNodesPerThread->reserve(estimatedItems);
	int localScoreToAct = scoreToAct;
	ScoresResult maxLocalScore;
	const int currTlSize = m_currentTL->size(), histTlSize = m_historyTL->size();
	for (int i = startIdx; i < endIdx; i++) {
		ScoresResult resScore = CompareOneMinNodesPeriods(i, threadIndex);

		if (resScore.totalScore.total > localScoreToAct) {
			if (i + currTlSize < histTlSize) {
				resultNodesPerThread->emplace_back(m_historyTL->at(i + currTlSize - 1), resScore);
			}
		}
		else
		{
			i += 10;
		}
		if (maxLocalScore.totalScore.total < resScore.totalScore.total)
			maxLocalScore = resScore;

	}

	{
		std::lock_guard<std::mutex> lock(*maxResultLock);
		if (maxScore->totalScore.total < maxLocalScore.totalScore.total)
			*maxScore = maxLocalScore;
	}
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