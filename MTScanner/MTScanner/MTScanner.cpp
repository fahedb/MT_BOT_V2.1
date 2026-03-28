#include "ProccessFlags.h"
#include "CONFIGRATIONS.H"
#include "DataBaseConnection.h"
#include "PolynomTimeLineCache.h"
#include "ScoreResultsCachePolynom.h"
#include "ThreadPool.h"


#include "PatternGodScanner.h"
void DefualtProgramStart();
void LoadJson_Data_ToDBFile();
void Init();

int main(int argc, char* argv[])
{
	Init();
	ProccessFlags::InitProccessFlags(argc, argv);
	
	std::thread http_thread(ExchangeData::Start_DataExchange);
	if (ProccessFlags::FindSwitch("-dataLoad"))
	{
		LoadJson_Data_ToDBFile();
	}
	else {
		DefualtProgramStart();
	}
	http_thread.join();
}

void Init()
{
	PolynomComparerPolicy* polyPolicy = Configrations::getInstance()->Get_Policy<PolynomComparerPolicy>(PolicyType::PolynomComparerPolicy);

	ScoreResultsCachePolynom::Init(MTThreadPool::getInstance()->Get_ThreadsNum(), 50000);
	Static_CurrentTimeLineRep::Init(Configrations::getInstance()->Get_Policy<ScoringLayerPolicy>(PolicyType::ScoringLayerPolicy)->Pattern_CurrentTimeLineLen
		, Configrations::getInstance()->Get_Policy<OrderTracerPolicy>(PolicyType::OrderTracerPolicy)->TraceCurrTimeLineLen);
	PolynomTimeLineCache::Init(polyPolicy->EnablePolynomTimeLineCaching, polyPolicy->EnablePolynomScan, polyPolicy->PolynomLen_ForScan);

	std::string connStr = Configrations::getInstance()->Get_ConnectionString();
	DataBaseConnection::Set_ConnectionString(connStr);
}


void DefualtProgramStart()
{
	Init();
	PatternGodScanner scanner;
	scanner.Run();
}



void LoadJson_Data_ToDBFile()
{
	std::string dbFilePath = Configrations::getInstance()->Get_DB_FileDir() + Configrations::getInstance()->Get_Config_Main(DBFILE_XAUUSD_M1_DATA_JSON);
	ExchangeData exChData;
	std::ofstream file(dbFilePath);
	Logger& logger = Logger::getInstance();
	long counter = 1;
	logger.WriteLog("Start reciveing data!", true);
	while (true)
	{
		std::string body = exChData.Get_NextCandleStick_M1_JSON_DB();
		if (body.length())
		{
			file << body << "\n";
			logger.WriteLog("Recieved a New Bar to Json File, Bar number: " + std::to_string(counter));
			counter++;
		}
	}
}


