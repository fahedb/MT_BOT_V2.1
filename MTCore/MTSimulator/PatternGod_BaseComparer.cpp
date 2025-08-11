#include "PatternGod_BaseComparer.h"

PatternGod_BaseComparer::~PatternGod_BaseComparer()
{
	if (m_historyDB_Map != nullptr) {
		delete m_historyDB_Map; // Free the history DB map
		m_historyDB_Map = nullptr;
	}
	if (m_currDB_Map != nullptr) {
		delete m_currDB_Map; // Free the current DB map
		m_currDB_Map = nullptr;
	}
}

inline int PatternGod_BaseComparer::ScanForOneMinPatterns_MultiThreaded(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D) // Scan for patterns in the history time line compared to the current time line
{
	std::mutex resultMutex, local2DResultMutex;

	Logger& logger = Logger::getInstance();
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy<ConfigrationsPolicy>(PolicyType::ConfigrationsPolicy);
	if (m_currentTL == nullptr) {
		logger.WriteLog("Current time line is not set for comparison.", true, true, LogType::ERR);
		return 0;
	}

	int scoreToAct = static_cast<int> (m_currentTL->size()) * static_cast<double> (m_ScoreToAct);
	if (scoreToAct <= 0)
		scoreToAct = 0;

	ScoresResult maxScore;
	int threadCount = ThreadPool::getInstance()->Get_ThreadsNum(); // Number of threads
	int historySize = static_cast<int>(m_historyTL->size());
	int chunkSize = historySize / threadCount;

	logger.WriteLog("Score to Act: " + std::to_string(scoreToAct), false, true);

	std::vector<std::vector < std::pair< ChangeNode*, ScoresResult>>> resultNodesPerThread(threadCount);
	ThreadPool* tPool = ThreadPool::getInstance();
	Logger::getInstance().WriteLog("Polynom Scan Started", false, true, LogType::LOG); // Log before stop low priority tasks
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


	PatternGodUtils::SortScores(resultNodes2D);
	PatternGodUtils::FilterDuplicatesTimeLines(resultNodes2D, m_historyDB_Map);
	PatternGodUtils::filterByHighScores(resultNodes2D);
	logger.WriteLog(PatternGodUtils::BuildScanResultLog(resultNodes2D), false, true);

	return static_cast<int>(resultNodes2D.size());
}

inline void PatternGod_BaseComparer::ProccessScanChunk(int startIdx, int endIdx, int threadIndex, int scoreToAct, std::mutex* maxResultLock, ScoresResult* maxScore, std::vector<std::pair<ChangeNode*, ScoresResult>>* resultNodesPerThread)
{

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
			i += m_NumOfSkips_ForLowScoreScan;
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
