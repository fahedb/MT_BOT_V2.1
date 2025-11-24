#include "ProccessFlags.h"
#include "ExchangeData.h"
#include "BackTestDataLoader.h"	
#include "BackTestingTimer.h"

void DefualtProgramStart();

int main(int argc, char* argv[])
{
	ProccessFlags::InitProccessFlags(argc, argv);
	DefualtProgramStart();
}

void DefualtProgramStart() 
{
	// TODO: Please check where else i build DBMap and just pass it from here to other modules -- No need to build it multiple times
	BackTestDataLoader currDataLoader;
	currDataLoader.Load_BackTestData("C:\\MTDB\\BackTestCurrData\\XAUUSD_M1_DATA.json");
	auto* currData = currDataLoader.Get_BackTestDataPointer();
	auto* dateTimeMap = currDataLoader.Get_DateTimeToIdxMap();
	BackTestingTimer backTestTimer(currData, dateTimeMap);
	backTestTimer.Run();
}