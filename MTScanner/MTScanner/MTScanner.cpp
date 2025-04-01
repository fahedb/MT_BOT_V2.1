#include "DataLoad.h"

#include "EURUSD_CurrentTable.h"
#include "XAUUSD_OneMinTable.h"
#include "XAUUSD_CurrentTable.h"
#include "PatternGod.h"
#include "SynchronizeCurrentDBBase.h"
#include "DataFeeder.h"
#include "DataBaseConnection.h"
#include "DataLoad.h"
#include "Configrations.h"
#include <thread> 
#include <future>
#include "Logger.h"
#include "MTChart.h"
#include "ExchangeData.h"
#include <queue>
#include "MyJSONHandler.h"

std::vector<ChangeNode*> LoadHistoryToVector();
void OneMinutePatternGodLogic(PatternGod* patternScan, std::vector<ChangeNode*> currentTimeLine);
void LoadDataFromFileToDBLogic();
void DefualtProgramStart();
void UpdateCurrentTimeLine(std::vector<ChangeNode*>* timeLine, BasePricesTable* table);
void LoadJson_Data_ToDBFile();
bool IsItDataLoading(int argc, char* argv[]);
std::vector<ChangeNode*> LoadHistoryToVector_MultiThreaded(XAUUSD_OneMinTable* tb);
std::vector<ChangeNode*> LoadChangeNodesForThread(int threadIdx, XAUUSD_OneMinTable* tb);
std::string m_lastTimePoint = "";

int main(int argc, char* argv[])
{	
	std::thread http_thread(ExchangeData::Start_DataExchange);

	if (IsItDataLoading(argc, argv))
	{
		LoadJson_Data_ToDBFile();
	}
	else {
		DefualtProgramStart();
	}
	
	http_thread.join();
}

bool IsItDataLoading(int argc,char* argv[]) // temp Function, need to implemnt a class for flags
{
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-dataLoad") == 0) {
			return true;
		}
	}
	return false;

}
void LoadAllConfigsToHttpCore()
{
	ExchangeData exh;
	std::vector<std::vector<std::string>> allConfigs = Configrations::getInstance()->Get_AllConfigRows_DB();
	for (int i = 0; i < allConfigs.size(); i++)
	{
		exh.Push_NewConfig(allConfigs.at(i));
	}
}
void UpdateConfigs()
{
	ExchangeData exh;
	std::pair<std::string, std::string> conf = exh.Get_ConfToUpdate();
	Configrations* configarions = Configrations::getInstance();
	while (conf.first.length())
	{
		configarions->UpdateConfigValue_DB(conf.first, conf.second);
		conf = exh.Get_ConfToUpdate();
	}
}

void DefualtProgramStart()
{
	Logger& logger = Logger::getInstance();
	logger.WriteLog("Defualt Program Start", false, true);
	
	LoadAllConfigsToHttpCore();
	XAUUSD_OneMinTable tb(Configrations::getInstance()->Get_DB_FileDir());
	std::vector<ChangeNode*> history = LoadHistoryToVector_MultiThreaded(&tb); // Load all data from a file to vector of ChangeNodes pointers
	

	std::future<void> cleanupTask = std::async(std::launch::async, [&tb]() {
		tb.RemoveSelect();
		});
	
	PatternGod patternScaner(&history); // Init a PatternGod object for patterns Scanning
	XAUUSD_CurrentTable table;
	
	std::vector<ChangeNode*> chnVec;
	UpdateCurrentTimeLine(&chnVec, &table);
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
	int timeBetweenScans = policy->TimeBetweenScans;
	while (true)
	{
		UpdateConfigs();
		logger.WriteLog("Sleep for " + std::to_string(timeBetweenScans) + " seconds...",true,true);
		Sleep(timeBetweenScans * 1000); // Sleep for 5 seconds before start a new scan
		Configrations::getInstance()->RefreshPolicy(); // Refresh all policies 
		UpdateCurrentTimeLine(&chnVec, &table);
		OneMinutePatternGodLogic(&patternScaner,chnVec); // Start scanning
	}

}

void UpdateCurrentTimeLine(std::vector<ChangeNode*>* timeLine, BasePricesTable* table)
{
	if (!timeLine)
		return;

	ExchangeData exChData;

	if (exChData.IsCurr_QueueLocked())
		return;

	std::string body = exChData.Get_NextCandleStick_M1();
	Logger& logger = Logger::getInstance();
	// Remove the first element
	if (body.length()) {
		logger.WriteLog("Recieved a new bar by HTTP Client");
		nlohmann::json_abi_v3_11_3::json select = nlohmann::json::parse(body);

		JsonSelectionNode* selectionJ = new JsonSelectionNode(select);

		table->Insert_NewCandleBody(selectionJ);
		ChangeNode* chn = table->GetNextChangeNode();
		if (chn) {
			while (timeLine->size() > Configrations::getInstance()->Get_Policy()->Pattern_CurrentTimeLineLen)
			{
				delete timeLine->at(0);
				timeLine->erase(timeLine->begin());
			}
			if (timeLine->size() >= Configrations::getInstance()->Get_Policy()->Pattern_CurrentTimeLineLen) // Delete old objects
			{
				delete timeLine->at(0);
				timeLine->erase(timeLine->begin());
			}
			timeLine->push_back(chn); // Push new candle data
		}

		UpdateCurrentTimeLine(timeLine, table); // Keep calling this till there is no data in the http queue
	}
	else {
		logger.WriteLog("No new Bars to recieve");
	}
	return;
}

void LoadDataFromFileToDBLogic() // Old Logic
{
	bool sw = true;
	while (sw)
	{
		int lastIdx = 0;
		DataBaseConnection Db_work2;
		DataLoad dl("C:\\Users\\fahed.b\\Desktop\\Data\\1MinData\\AllData.txt", "EURUSD_OneMinTimeLinePrices", Db_work2.Get_EnvHandler(), Db_work2.Get_DBConnection());
		sw = dl.Load(&lastIdx);
	}
}

void OneMinutePatternGodLogic(PatternGod* patternScan, std::vector<ChangeNode*> currentTimeLine)
{
	//std::vector<ChangeNode*> currentTimeLine; // Vector to store curr changeNodes
	Logger& logger = Logger::getInstance();


	if (!currentTimeLine.size())
	{
		logger.WriteLog("No current time line.", false, true, LogType::ERR);
		logger.WriteLog("Sleeping for 5 minutes.", false, true, LogType::ERR);
		Sleep(5 * 60000);
		return;
	}
	
	logger.WriteLog("current time line size: " + std::to_string(currentTimeLine.size()));
	if (currentTimeLine.at(0)->point_DateTime == m_lastTimePoint) // If the last time point scanned == to the current time point (start of the current vector) don't continue
	{
		currentTimeLine.clear();
		std::cout << "\n\nNo new data, Last point time: " << m_lastTimePoint << std::endl;
		return;
	}
	m_lastTimePoint = currentTimeLine.at(0)->point_DateTime;
	logger.WriteLog("!!!!!!!!!!!!		Start New Period Scan		!!!!!!!!!", true,true);
	logger.WriteLog("Current TimePoint: " + m_lastTimePoint, false, true);
	logger.WriteLog("Scanning period bars count: " + std::to_string(currentTimeLine.size()), false, true);

	std::vector<ChangeNode*> results; // Vector to the Head of every time point

	patternScan->Set_NewCurrentPeriod(&currentTimeLine); // Set a new Current Period for PatternScan object for the new scan
	ExchangeData exh;

	if (!exh.IsScanningAllowed())
	{
		logger.WriteLog("Scanning Stopped by user, skipping scanning...", false, true);
		return;
	}

	if (patternScan->ScanForOneMinPatterns_MultiThreaded(&results)) // Scan for patterns
	{
		int orderRes = patternScan->DetectCommonDirection(&results);
		if (orderRes != -1) // Check for common direction
		{
			logger.WriteLog("New Order Set.", false, true);
			exh.Set_NewOrder("XAUUSD", orderRes);
		}
	}
	else
		logger.WriteLog("Didn't find any patterns with min score !!", false, true);

	currentTimeLine.clear(); // Clear the current time line and get ready for the next scan

}
std::vector<ChangeNode*> LoadHistoryToVector_MultiThreaded(XAUUSD_OneMinTable* tb) // Need to change name
{

	Logger& logger = Logger::getInstance();
	//std::unique_ptr<XAUUSD_OneMinTable> tb = std::make_unique<XAUUSD_OneMinTable>(Configrations::getInstance()->Get_DB_FileDir());
	tb->Set_DATA_DB_FileName(Configrations::getInstance()->Get_DB_FileDir() + tb->GetDB_PhysicalFileName_Json());

	logger.WriteLog("Pulling data from DB...", false, true);
	tb->Set_MultiThreadedRead();
	tb->SelectDataFromJsonFile();

	logger.WriteLog("Done pulling from DB.", false, true);
	logger.WriteLog("Start Loading data into RAM...", false, true);

	std::vector<ChangeNode*> hitoryTimeLine;
	int threadCount = static_cast<int>(std::thread::hardware_concurrency() - 1);

	logger.WriteLog("Trying to reserve " + std::to_string(tb->GetSelected_Count_MultiThreaded(0) * threadCount) + " Data cells...");

	hitoryTimeLine.reserve(static_cast<size_t>(tb->GetSelected_Count_MultiThreaded(0) * threadCount ));
	logger.WriteLog("Done reserving.");

	logger.WriteLog("Threads Available for parallel Build: " + std::to_string(threadCount), false, true);

	int chunkSize = tb->GetSelected_Count_MultiThreaded(0);
	logger.WriteLog("Every thread should build " + std::to_string(chunkSize) + " CHange nodes.", false, true);

	std::mutex loggerMutex, resultMutex;
	std::vector< std::vector<ChangeNode*>> reads2D(threadCount);

	// Function to process a chunk of the loop
	auto processChunk = [&](int threadIndex, XAUUSD_OneMinTable* tableObj) {

		{
			std::lock_guard<std::mutex> lock(loggerMutex);
			logger.WriteLog("Thread [" + std::to_string(threadIndex) + "] Started Building ChangeNodes...");
		}

		reads2D[threadIndex] = std::move(LoadChangeNodesForThread(threadIndex, tableObj)); // קריאה מקובץ

		{
			std::lock_guard<std::mutex> lock(loggerMutex);
			logger.WriteLog("Thread [" + std::to_string(threadIndex) + "] Done Building " + std::to_string(reads2D[threadIndex].size()) + " ChangeNodes.");
		}

	};

	// שימוש ב-std::async לניהול תהליכונים אוטומטי
	std::vector<std::future<void>> futures;
	for (int t = 0; t < threadCount; ++t) {
		futures.emplace_back(std::async(std::launch::async, processChunk, t,tb));
	}

	// שימוש ב-get() במקום join()
	for (auto& f : futures) {
		f.get(); // מחכה לסיום כל המשימות
	}

	// Merge results
	for (int i = 0; i < threadCount; i++)
	{
		std::vector<ChangeNode*> vec = reads2D.at(i);
		for (int t = 0; t < vec.size(); t++)
		{
			hitoryTimeLine.push_back(vec.at(t));
		}
		reads2D.at(i).clear();
	}
	reads2D.clear();

	ExchangeData exh;
	exh.DoneLoadingData();	

	return hitoryTimeLine;
}

std::vector<ChangeNode*> LoadChangeNodesForThread(int threadIdx, XAUUSD_OneMinTable* tb)
{
	std::vector<ChangeNode*> localHistoryTimeLine;
	localHistoryTimeLine.reserve(tb->GetSelected_Count_MultiThreaded(0));

	ChangeNode* changeN;
	changeN = tb->GetNextChangeNode_MultiThreaded(threadIdx);
	while (changeN && changeN->activePoint)
	{
		localHistoryTimeLine.push_back(changeN);
		changeN = tb->GetNextChangeNode_MultiThreaded(threadIdx);
	}

	return localHistoryTimeLine;
}




std::vector<ChangeNode*> LoadHistoryToVector()
{
	Logger& logger = Logger::getInstance();
	XAUUSD_OneMinTable tb(Configrations::getInstance()->Get_DB_FileDir());
	tb.Set_DATA_DB_FileName(Configrations::getInstance()->Get_DB_FileDir() + tb.GetDB_PhysicalFileName_Json());
	
	logger.WriteLog("Pulling data from DB...");
	tb.Set_MultiThreadedRead();
	tb.SelectDataFromJsonFile();

	logger.WriteLog("Done pulling from DB.");
	logger.WriteLog("Start Loading data into RAM...");

	std::vector<ChangeNode*> hitoryTimeLine;
	logger.WriteLog("Trying to reserve " + std::to_string(tb.GetSelected_Count()) + " Data cells...");
	hitoryTimeLine.reserve(static_cast<size_t>(tb.GetSelected_Count()));
	logger.WriteLog("Done reserving.");

	ChangeNode* changeN;
	changeN = tb.GetNextChangeNode();
	long changeNodesCounter = 0;
	while (changeN && changeN->activePoint)
	{
		changeNodesCounter++;
		hitoryTimeLine.push_back(changeN);
		changeN = tb.GetNextChangeNode(changeN);
		if (!(changeNodesCounter % 100000))
		{
			logger.WriteLog("Done building " + std::to_string(changeNodesCounter) + " ChangeNodes.");
		}
	}
	logger.WriteLog("Done Loading data into RAM.");
	logger.WriteLog("Start Realeasing Rows Memory...");
	
	tb.RemoveSelect();
	
	logger.WriteLog("Done Realeasing Rows Memory.");
	logger.WriteLog("History candles loaded to memory: " + std::to_string(hitoryTimeLine.size()));

	return hitoryTimeLine;
}


void LoadJson_Data_ToDBFile()
{
	std::string dbFilePath = Configrations::getInstance()->Get_DB_FileDir() + Configrations::getInstance()->Get_ConfigValue(DBFILE_XAUUSD_M1_DATA_JSON);
	ExchangeData exChData;
	std::ofstream file(dbFilePath);
	Logger& logger = Logger::getInstance();
	long counter = 1;
	logger.WriteLog("Start reciveing data!", true);
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		if (exChData.IsDBHist_QueueLocked())
			continue;

		std::string body = exChData.Get_NextCandleStick_M1_JSON_DB();
		if (body.length())
		{
			file << body << "\n";
			logger.WriteLog("Recieved a New Bar to Json File, Bar number: " + std::to_string(counter));
			counter++;
		}
	}
}