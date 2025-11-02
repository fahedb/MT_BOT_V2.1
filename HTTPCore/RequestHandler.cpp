#include "RequestHandler.h"
#include "OrdersStructCache.h"
#include "Converts.h"
#include "Logger.h"

void RequestHandler::NewBar_M1_Wait() {
	std::unique_lock<std::mutex> lock(newBar_M1_MTX);
	newBar_M1_CV.wait(lock, [this] { return newBar_M1_Flag; }); // Wait till flag is true
}

void RequestHandler::NewBar_M1_NotifyAll() {
	{
		std::lock_guard<std::mutex> lock(newBar_M1_MTX);
		newBar_M1_Flag = true;
	}
	newBar_M1_CV.notify_all();
}

void RequestHandler::NewBar_M1_NotifyOne() {
	{
		std::lock_guard<std::mutex> lock(newBar_M1_MTX);
		newBar_M1_Flag = true;
	}
	newBar_M1_CV.notify_one();
}

void RequestHandler::NewBar_M1_ResetFlag() {
	std::lock_guard<std::mutex> lock(newBar_M1_MTX);
	newBar_M1_Flag = false;
}

void RequestHandler::Handle_FutureVisionScan(const httplib::Request& req, httplib::Response& res)
{
	FutureVisionScan_ResetFlag(); // Reset the flag to false before waiting
	Handle_NewBar_M1(req.body); // Push new bar to queue and notify the scanner
	FutureVisionScanWait(); // Wait till notified that scan is done
	Handle_IsThereAnTrade(res); // Check if there is an trade to execute after the scan
}

void RequestHandler::FutureVisionScan_ResetFlag() {
	std::lock_guard<std::mutex> lock(futureVisionScanMTX);
	futureVisionScanFlag = false;
}


void RequestHandler::FutureVisionScan_NotifyAll() {

	{
		std::lock_guard<std::mutex> lock(futureVisionScanMTX);
		futureVisionScanFlag = true;
	}
	futureVisionScanCV.notify_all();
}

void RequestHandler::FutureVisionScan_NotifyOne() {

	{
		std::lock_guard<std::mutex> lock(futureVisionScanMTX);
		futureVisionScanFlag = true;
	}
	futureVisionScanCV.notify_one();
}

void RequestHandler::FutureVisionScanWait() {
	std::unique_lock<std::mutex> lock(futureVisionScanMTX);
	futureVisionScanCV.wait(lock, [this] { return futureVisionScanFlag; }); // Wait till flag is true
}

int RequestHandler::Get_ConfigIdx(std::string& config)
{
	for (int i = 0; i < m_confVec.size(); i++)
	{
		if (m_confVec.at(i).label == config)
			return i;
	}
	return -1;
}

ConfigStruct RequestHandler::Get_Config(std::string& config)
{
	int idx = Get_ConfigIdx(config);
	if (idx < 0)
		return ConfigStruct();
	return m_confVec.at(idx);
}

bool RequestHandler::UpdateConfig(std::string& config, std::string& value)
{
	int idx = Get_ConfigIdx(config);
	if (idx >= 0)
	{
		m_confVec.at(idx).value = value;
		m_confVec.at(idx).need_To_Update = true;
		return true;
	}
	return false;
}

std::pair<std::string, std::string> RequestHandler::GetNextConfigToUpdate()
{
	for (int i = 0; i < m_confVec.size(); i++)
	{
		if (m_confVec.at(i).need_To_Update)
		{
			m_confVec.at(i).need_To_Update = false;
			return std::make_pair(m_confVec.at(i).label, m_confVec.at(i).value);
		}
	}
	return std::make_pair("", "");
}

void RequestHandler::PushNewConfig(std::vector<std::string>& conf)
{
	if (conf.size() < 3)
		return;

	ConfigStruct configStruct;
	configStruct.label = conf.at(0);
	configStruct.value = conf.at(1);
	configStruct.explain = conf.at(2);
	configStruct.m_permission = Converts::ToInt(conf.at(3));
	configStruct.m_catagory = Converts::ToInt(conf.at(4));
	configStruct.policyCategory = conf.at(5);
	configStruct.need_To_Update = false;

	m_confVec.push_back(configStruct);
}

void RequestHandler::Handle_NewBar_M1(const std::string& body)
{
	if (!body.length())
		return;
	std::lock_guard<std::mutex> lock(m_CurrQueue_locker);
	m_candlesBodies.push(body); // Push new candle body to queue
	NewBar_M1_NotifyOne(); // Notify one waiting thread
}

void RequestHandler::Handle_JSON_DB_NewBar_M1(const std::string& body)
{
	if (!body.length())
		return;
	std::lock_guard<std::mutex> lock(m_HistQueue_locker);
	m_candlesBodies_history.push(body);
}

std::string RequestHandler::Get_NextCandle()
{
	if (!m_candlesBodies.size())
		return "";
	std::lock_guard<std::mutex> lock(m_CurrQueue_locker);

	std::string body = m_candlesBodies.front();
	m_candlesBodies.pop();
	return body;
}

std::string RequestHandler::Get_Next_DBCandle_M1()
{
	if (!m_candlesBodies_history.size())
		return "";

	std::lock_guard<std::mutex> lock(m_HistQueue_locker);

	std::string body = m_candlesBodies_history.front();
	m_candlesBodies_history.pop();
	return body;
}

std::string RequestHandler::GetStatus_Scanning() {
	if (m_loadingDataStatus)
		return HTTP_PARAM_VALUE_Status_LoadingData;
	if (m_runningStatus)
		return HTTP_PARAM_VALUE_Status_Running;
	return HTTP_PARAM_VALUE_Status_Stopped;
}

void RequestHandler::Handle_IsThereAnTrade(httplib::Response& res) // Returns 
{
	std::lock_guard<std::mutex> lock(m_tradeQueueMutex);
	if (!m_ordersToExecute.size())
		return;
	Http_OrderStruct newOrder = m_ordersToExecute.top();

	while (!m_ordersToExecute.empty()) // i'm using queue for futher usage, but currently i need only the front item
		m_ordersToExecute.pop();


	res.set_header(HTTP_PARAM_Symbol, newOrder.symbol);
	res.set_header(HTTP_PARAM_TradeType, newOrder.long_short ? HTTP_PARAM_VALUE_TradeType_Short : HTTP_PARAM_VALUE_TradeType_Long);
	res.set_header(HTTP_PARAM_TradeSeqNum, std::to_string(newOrder.orderSeqNum));
	res.set_header(HTTP_PARAM_OrderStopLoss, std::to_string(newOrder.stopLoss));
	res.set_header(HTTP_PARAM_OrderTakeProfit, std::to_string(newOrder.takeProfit));

}

void RequestHandler::Handle_CloseOrder(const httplib::Request& req, httplib::Response& res)
{
	int seqNum = req.get_header_value(HTTP_PARAM_TradeSeqNum).empty() ? 0 : Converts::ToInt(req.get_header_value(HTTP_PARAM_TradeSeqNum));

	OrderEntity* order = OrderEntityCache::getInstance()->Get_Order(seqNum);
	if (order)
		order->CloseOrder();
	res.set_header(HTTP_PARAM_OrderStatus, HTTP_PARAM_VALUE_OrderStatus_Closed);
}

void RequestHandler::Handle_GetOrderData(const httplib::Request& req, httplib::Response& res)
{

	std::string  seqNum = req.get_header_value(HTTP_PARAM_TradeSeqNum);
	int seqnum = Converts::ToInt(seqNum);

	if (seqnum <= 0)
	{
		Logger::getInstance().WriteLog("Error with seq num,\n recieved seqNum: " + seqNum, false, true, LogType::ERR);
		res.set_content("Invalid Order SeqNum", "text/plain");
		res.set_header(HTTP_PARAM_OrderStatus, HTTP_PARAM_VALUE_OrderStatus_Closed);
		return;
	}

	OrderEntity* order = OrderEntityCache::getInstance()->Get_Order(seqnum);
	if (order)
	{
		res.set_header(HTTP_PARAM_OrderStopLoss, std::to_string(order->Get_StopLoss()));
		res.set_header(HTTP_PARAM_OrderTakeProfit, std::to_string(order->Get_TakeProfit()));
		res.set_header(HTTP_PARAM_TradeType, order->Get_OrderType_str());
		res.set_header(HTTP_PARAM_Symbol, order->Get_PairType_str());
		res.set_header(HTTP_PARAM_OrderStatus, order->Get_OrderStatus_str());
		res.set_header(HTTP_PARAM_TradeSeqNum, seqNum);
	}
	else
	{
		res.set_header(HTTP_PARAM_OrderStatus, HTTP_PARAM_VALUE_OrderStatus_Closed);
		res.set_header(HTTP_PARAM_TradeSeqNum, seqNum);
		return;
	}
}

void RequestHandler::Set_NewOrder(const std::string& symbol, int long_short, int seqNum, int stopLoss, int takeProfit)
{
	std::lock_guard<std::mutex> lock(m_tradeQueueMutex);
	Http_OrderStruct newOrder;
	newOrder.symbol = symbol;
	newOrder.long_short = long_short;
	newOrder.orderSeqNum = seqNum;
	newOrder.stopLoss = stopLoss;
	newOrder.takeProfit = takeProfit;
	m_ordersToExecute.push(newOrder);
}


void RequestHandler::Handle_IsScanOnGoing(httplib::Response& res)
{
	Get_IsScanning() ? res.set_header(HTTP_PARAM_IsScanOnGoing, HTTP_PARAM_VALUE_True) : res.set_header(HTTP_PARAM_IsScanOnGoing, HTTP_PARAM_VALUE_False);
}