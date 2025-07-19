#include "StrategiesRunner.h"
#include "ProccessFlags.h"

void DefualtProgramStart();
void LoadJson_Data_ToDBFile();

int main(int argc, char* argv[])
{
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

void DefualtProgramStart()
{

	//OrderEntity* order = new OrderEntity(OrderType::SHORT, "2025062519350000", "2025062519350000", CoinsPair::XAUUSD);
	//order->Set_TakeProfit(static_cast<int>(-244));
	//order->Set_StopLoss(static_cast<int>(234));

	//ExchangeData().Set_NewOrder("XAUUSD", 1, order->Get_OrderSeqNum(), order->Get_StopLoss(), order->Get_TakeProfit());

	StrategiesRunner runner;
	runner.Run();
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
		std::string body = exChData.Get_NextCandleStick_M1_JSON_DB();
		if (body.length())
		{
			file << body << "\n";
			logger.WriteLog("Recieved a New Bar to Json File, Bar number: " + std::to_string(counter));
			counter++;
		}
	}
}
