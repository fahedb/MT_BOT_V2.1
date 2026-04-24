#include "ProccessFlags.h"
#include "ExchangeData.h"
#include "BackTestDataLoader.h"	
#include "BackTestingTimer.h"
#include "AnalyzerBackTestDataImporter.h"
#include "ScoreResultsCachePolynom.h"
#include "ThreadPool.h"
#include "PolynomTimeLineCache.h"

void DefualtProgramStart();

int main(int argc, char* argv[])
{
	ProccessFlags::InitProccessFlags(argc, argv);
	DefualtProgramStart();
}
void Init()
{
	PolynomComparerPolicy* polyPolicy = Configrations::getInstance()->Get_Policy<PolynomComparerPolicy>(PolicyType::PolynomComparerPolicy);

	ScoreResultsCachePolynom::Init(MTThreadPool::getInstance()->Get_ThreadsNum(), 50000);
	Static_CurrentTimeLineRep::Init(Configrations::getInstance()->Get_Policy<ScoringLayerPolicy>(PolicyType::ScoringLayerPolicy)->Pattern_CurrentTimeLineLen
		, Configrations::getInstance()->Get_Policy<OrderTracerPolicy>(PolicyType::OrderTracerPolicy)->TraceCurrTimeLineLen, false);
	PolynomTimeLineCache::Init(polyPolicy->EnablePolynomTimeLineCaching, polyPolicy->EnablePolynomScan, polyPolicy->PolynomLen_ForScan);

	std::string connStr = Configrations::getInstance()->Get_ConnectionString();
	DataBaseConnection::Set_ConnectionString(connStr);
}

void DefualtProgramStart() 
{
	Init();
	// TODO: Please check where else i build DBMap and just pass it from here to other modules -- No need to build it multiple times
	BackTestDataLoader currDataLoader;
	std::vector<ChangeNode> historyMemoryPool;
	currDataLoader.Load_BackTestData("C:\\MTDB\\BackTestCurrData\\XAUUSD_M1_DATA.json", &historyMemoryPool);
	auto* currData = currDataLoader.Get_BackTestDataPointer();
	auto* dateTimeMap = currDataLoader.Get_DateTimeToIdxMap();
	BackTestingTimer backTestTimer(currData, dateTimeMap);
	backTestTimer.Run();
}