#include "PatternGod.h"
#include "TimeLineBuilder.h"
#include "LOGGER.H"
#include "CONFIGRATIONS.H"
#include "CPP_To_Cuda_Tunnel.h"

void PatternGod::CudaInit()
{
	if (!m_currentTL || !m_historyTL)
	{
		Logger::getInstance().WriteLog("PatternGod::CudaInit - Current or History time line is not set", false, true, LogType::ERR);
		return;
	}

	if (!m_cudaEngine && CPP_To_Cuda_Tunnel::IsCudaAvailable())
	{
		m_cudaEngine = new CudaEngine_ScoreCalcs(m_historyTL, m_currentTL); // Initialize the CUDA engine with history and current time lines
		m_cudaEngineInitialized = true;
	}
}

PatternGod::PatternGod(std::vector<ChangeNode*>* historyTimeLine, std::vector<ChangeNode*>* currentTimeLine)
{
	m_historyTL = historyTimeLine;
	m_currentTL = currentTimeLine;

	m_historyDB_Map = new DBMap(); // Initialize the history DB map
	TimeLineBuilder::Build_TimeLineDB(m_historyTL, m_historyDB_Map); // Build the DB map for the history time line
	m_historyDB_Owner = true;
	InitComparers(); // Initialize the comparers with the history time line

}

PatternGod::PatternGod(std::vector<ChangeNode*>* historyTimeLine)
{
	m_historyTL = historyTimeLine;
	m_currentTL = nullptr; // Initialize current time line as nullptr

	m_historyDB_Map = new DBMap(); // Initialize the history DB map
	TimeLineBuilder::Build_TimeLineDB(m_historyTL, m_historyDB_Map); // Build the DB map for the history time line
	m_historyDB_Owner = true; // Set ownership of the history DB map
	InitComparers(); // Initialize the comparers with the history time line
}

void PatternGod::InitComparers() // Initialize the comparers with the history time line
{
	m_comparers.push_back(std::make_unique<PatternGod_PolynomComparer>(m_historyTL));
	m_comparers.push_back(std::make_unique<PatternGod_ScoreComparer>(m_historyTL));
	m_polynomFilterEngine = new PatternGod_PolynomFilterEngine(m_historyTL);
	// *** Add more comparers as needed ***

	if (m_currentTL)
		Set_CurrentTimeLine(m_currentTL); // Set the current time line for all comparers if it exists
}

bool PatternGod::ScanPreCondition() const
{
	if (m_historyTL == nullptr || m_historyTL->empty())
	{
		Logger::getInstance().WriteLog("History time line is not set or is empty.", true, true, LogType::ERR);
		return false; // Return false if the history time line is not set or is empty
	}

	if (m_currentTL == nullptr || m_currentTL->empty())
	{
		Logger::getInstance().WriteLog("Current time line is not set or is empty.", true, true, LogType::ERR);
		return false; // Return false if the current time line is not set or is empty
	}
	return true; // Return true if both time lines are valid
}

bool PatternGod::CudaPreCondition() const
{
	if(! m_cudaEngine)
		{
		Logger::getInstance().WriteLog("CUDA Engine is not initialized.", true, true, LogType::ERR);
		return false; // Return false if the CUDA engine is not initialized
	}
	if (!m_cudaEngine->IsCudaDeviceAvailable())
	{
		Logger::getInstance().WriteLog("No CUDA device available.", true, true, LogType::ERR);
		return false; // Return false if no CUDA device is available
	}
	return true; // Return true if CUDA preconditions are met
}

int PatternGod::ScanForOneMinPatterns_MultiThreaded(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D)
{
	if (!ScanPreCondition())
		return 0; // Return 0 if preconditions for scanning are not met

	if (m_comparers.empty())
		InitComparers(); // Initialize comparers if not already done

	int totalScans = 0;
	PatternGodComparerResultsMap resultPerComparer; // Map to hold results for each comparer
	auto start = std::chrono::high_resolution_clock::now();
	if(!m_cudaEngine)
		CudaInit(); // Initialize CUDA engine if not already done
	std::vector<std::pair<ChangeNode*, ScoresResult>> cudaResults;
	if (m_cudaEngine && CudaPreCondition())
	{
		auto start = std::chrono::high_resolution_clock::now();
		m_cudaEngine->Set_NewCurrentTimeLine(m_currentTL); // Update current time line in CUDA engine
		auto settingNewTLTime = std::chrono::high_resolution_clock::now();
		std::cout << "CUDA Engine - Setting New Current TL Time: " 
			<< std::chrono::duration_cast<std::chrono::milliseconds>(settingNewTLTime - start).count() 
			<< " ms" << std::endl;

		totalScans = m_cudaEngine->ScanForOneMinPatterns_CUDA(cudaResults);
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		Logger::getInstance().WriteLog("Performance: CUDA Scan Time: " + std::to_string(duration) + " ms", false, true, LogType::LOG);

		resultPerComparer[PatternGodComparerType::CudaScoreComparer] = cudaResults; // Store CUDA results
	}
	else {
		for (const auto& comparer : m_comparers)
		{
			resultPerComparer[comparer->Get_ComparerType()] = std::vector<std::pair<ChangeNode*, ScoresResult>>(); // Initialize the result vector for each comparer
			if (comparer->IsScanEnabled())
				totalScans += comparer->ScanForOneMinPatterns_MultiThreaded(resultPerComparer[comparer->Get_ComparerType()]); // Scan for patterns using each comparer
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	Logger::getInstance().WriteLog("Performance: Total Scan Time: " + std::to_string(duration) + " ms", false, true, LogType::LOG);

	TradeManagerPolicy* tradeManPolicy = Configrations::getInstance()->Get_Policy<TradeManagerPolicy>(PolicyType::TradeManagerPolicy);
	
	for(auto& resVec : resultPerComparer)
	{
		auto& tempVec = resultPerComparer[resVec.first];
		// Add to resultNodes2D
		resultNodes2D.insert(resultNodes2D.end(), tempVec.begin(), tempVec.end());
	}
	return totalScans; // Return the total number of scans performed
}

int PatternGod::PolynomEngineFilter(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D)
{
	if (!resultNodes2D.size())
	{
		Logger::getInstance().WriteLog("Results vector is empty, No filter is applied.", false, true, LogType::ERR);
		return 0;
	}

	std::vector<ChangeNode*>heads = TimeLineBuilder::SplitChangeNodeFromScores(&resultNodes2D);

	auto tempVec = m_polynomFilterEngine->ProcessPatterns(
		&heads,
		m_currentTL->size());
	FilterLayerPolicy* filterLayerPolicy = Configrations::getInstance()->Get_Policy<FilterLayerPolicy>(PolicyType::FilterLayerPolicy);
	PatternGodUtils::filterByThresholdScore(tempVec, filterLayerPolicy->Pattern_TopScore_ThresholdRatio);
	PatternGodUtils::filterByHighScores(tempVec, filterLayerPolicy->Pattern_Max_Patterns_To_Find);
	resultNodes2D = tempVec;
}