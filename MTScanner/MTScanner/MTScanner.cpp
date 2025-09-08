#include "StrategiesRunner.h"
#include "ProccessFlags.h"

void DefualtProgramStart();
void LoadJson_Data_ToDBFile();

void Test();

int main(int argc, char* argv[])
{
	IndicatorScore testScore;
	ProccessFlags::InitProccessFlags(argc, argv);

	std::thread http_thread(ExchangeData::Start_DataExchange);
	if (ProccessFlags::FindSwitch("-dataLoad"))
	{
		LoadJson_Data_ToDBFile();
	}
	else {
		//Test();
		DefualtProgramStart();
	}
	http_thread.join();
}
void Test()
{
	TradesManager* tradeMngr = TradesManager::getInstance();
	OrdersPolicy* m_ordersPolicy = Configrations::getInstance()->Get_Policy<OrdersPolicy>(PolicyType::OrdersPolicy);

	OrderEntity* order = new OrderEntity(OrderType::LONG, "202508291200000", "202508291200000", CoinsPair::XAUUSD);
	order->Set_TakeProfit(m_ordersPolicy->TP_start);
	order->Set_StopLoss( m_ordersPolicy->SL_start);

	if (order)
	{
		OrderEntityCache::getInstance()->Add_NewOrder(order); // Add the order to the cache
		tradeMngr->Send_TraddeSignal(order->Get_PairType_str(), int(order->Get_OrderType()), order->Get_OrderSeqNum(),
		order->Get_StopLoss(), order->Get_TakeProfit()); // Send the trade signal to the exchange
	}
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
