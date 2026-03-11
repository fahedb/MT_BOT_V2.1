#include "PatternGod_BaseComparer.h"
#include "ThreadPool.h"
#include "TimeLineBuilder.h"
#include "MTSimulatorUtils.h"
#include "CONFIGRATIONS.H"
#include "Logger.h"

PatternGod_BaseComparer::PatternGod_BaseComparer(std::vector<ChangeNode*>* history) : m_historyTL(history)
{
	m_historyDB_Map = new DBMap();
	m_currDB_Map = new DBMap();
	TimeLineBuilder::Build_TimeLineDB(history, m_historyDB_Map);
	m_cacheHits.resize(MTThreadPool::getInstance()->Get_ThreadsNum(), 0); // Initialize cache hits for each thread
	m_cacheMisses.resize(MTThreadPool::getInstance()->Get_ThreadsNum(), 0); // Initialize cache misses for each thread
}

PatternGod_BaseComparer::PatternGod_BaseComparer(std::vector<ChangeNode*>* history, std::vector<ChangeNode*>* curr) : m_historyTL(history), m_currentTL(curr)
{
	m_historyDB_Map = new DBMap();
	m_currDB_Map = new DBMap();
	TimeLineBuilder::Build_TimeLineDB(history, m_historyDB_Map);
	TimeLineBuilder::Build_TimeLineDB(curr, m_currDB_Map);
	m_cacheHits.resize(MTThreadPool::getInstance()->Get_ThreadsNum(), 0); // Initialize cache hits for each thread
	m_cacheMisses.resize(MTThreadPool::getInstance()->Get_ThreadsNum(), 0); // Initialize cache misses for each thread
}

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

void PatternGod_BaseComparer::ResetMyCacheCounters(int threadIdx)
{
	if (threadIdx >= 0 && threadIdx < m_cacheHits.size())
	{
		m_cacheHits[threadIdx] = 0;
		m_cacheMisses[threadIdx] = 0;
	}
}

int PatternGod_BaseComparer::ScanForOneMinPatterns_MultiThreaded(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D) // Scan for patterns in the history time line compared to the current time line
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
	int threadCount = MTThreadPool::getInstance()->Get_ThreadsNum(); // Number of threads
	int historySize = static_cast<int>(m_historyTL->size());
	int chunkSize = historySize / threadCount;

	logger.WriteLog("Score to Act: " + std::to_string(scoreToAct), false, true);

	std::vector<std::vector < std::pair< ChangeNode*, ScoresResult>>> resultNodesPerThread(threadCount);
	ScoringLayerPolicy* scoringLayerPolicy = Configrations::getInstance()->Get_Policy<ScoringLayerPolicy>(PolicyType::ScoringLayerPolicy);
	TradeManagerPolicy* tradeManPolicy = Configrations::getInstance()->Get_Policy<TradeManagerPolicy>(PolicyType::TradeManagerPolicy);

	MTThreadPool* tPool = MTThreadPool::getInstance();
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
				try {

					this->ProccessScanChunk(start, end, t, scoreToAct, &resultMutex, &maxScore, localResultsNodes);
					prom->set_value();
				}
				catch (const std::exception& ex) { // Catch any standard exceptions
					// Handle exceptions and set the promise to avoid hanging
					std::cerr << "Exception in thread: " << ex.what() << std::endl;
					prom->set_exception(std::current_exception()); // Set the exception to the promise to propagate it to the future
				}
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
	PatternGodUtils::filterByHighScores(resultNodes2D, scoringLayerPolicy->Num_Of_Top_Results_To_Filter_Out);
	logger.WriteLog(PatternGodUtils::BuildScanResultLog(resultNodes2D), false, true);
	
	m_scanCounter++;
	if(m_scanCounter >= 1000)
		m_scanCounter = 0;

	m_MaxScoreFound = maxScore.totalScore.total;
	return static_cast<int>(resultNodes2D.size());
}

void PatternGod_BaseComparer::ProccessScanChunk(int startIdx, int endIdx, int threadIndex, int totalScoreToAct, std::mutex* maxResultLock, ScoresResult* maxScore, std::vector<std::pair<ChangeNode*, ScoresResult>>* resultNodesPerThread)
{
	m_cacheHits[threadIndex] = 0; // Reset cache hits for this thread
	m_cacheMisses[threadIndex] = 0; // Reset cache misses for this thread
	int estimatedItems = 20;
	Logger& logger = Logger::getInstance();
	resultNodesPerThread->reserve(estimatedItems);
	int localScoreToAct = totalScoreToAct;
	ScoresResult maxLocalScore;
	const int currTlSize = m_currentTL->size(), histTlSize = m_historyTL->size();
	for (int i = startIdx; i < endIdx; i++) {
		ScoresResult resScore = CompareOneMinNodesPeriods(i, threadIndex, totalScoreToAct);

		if (resScore.totalScore.total > localScoreToAct) {
			if (i + currTlSize < histTlSize) {
				resultNodesPerThread->emplace_back(m_historyTL->at(i + currTlSize - 1), resScore);
			}
		}
		else
		{
			if (m_scanCounter % 8 != 0) // Every 8 scans, we do a full scan without skips to load cache better
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
	// Write logs for this thread
	logger.WriteLog("Thread [" + std::to_string(threadIndex) + "] processed indices [" + std::to_string(startIdx) + " - " + std::to_string(endIdx - 1) + "]"
		+ "\nfound " + std::to_string(resultNodesPerThread->size()) + " results"
		+ "\nmax score: " + std::to_string(maxLocalScore.totalScore.total)
		+ "\ncache hits : " + std::to_string(m_cacheHits[threadIndex])
		+ "\ncache misses : " + std::to_string(m_cacheMisses[threadIndex])
		, false, false, LogType::LOG);
}

void PatternGod_BaseComparer::Set_CurrentTimeLine(std::vector<ChangeNode*>* curr) {
	m_currentTL = curr;
	m_currDB_Map->Clear(); // Clear the current DB map before building it again
	TimeLineBuilder::Build_TimeLineDB(curr, m_currDB_Map);

}
