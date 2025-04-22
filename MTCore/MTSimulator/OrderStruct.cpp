#include "OrderStruct.h"

void OrderEntity::ActivateOrder(OrderType long_short, std::string timePoint, std::string statisticsTimePoint, CoinsPair pair)
{
	m_orderType = long_short;
	m_timePoint = timePoint;
	m_statisticsTimePoint = statisticsTimePoint;
	m_pair = pair;
	m_active = true;
	m_status = OrderStatus::OPEN;
	// Need to get new seqNum for the order
	DataBaseConnection* dbConn = Static_DBConnection::getInstance()->GetConnection();
	if (!dbConn) {
		Logger::getInstance().WriteLog("DB connection failed during ActivateOrder", false, true, LogType::ERR);
		return;
	}
	SeqNumTable table(dbConn->Get_EnvHandler(), dbConn->Get_DBConnection());
	m_order_seqNum = table.GetNextSeqNum(EntityType::Order);
}
