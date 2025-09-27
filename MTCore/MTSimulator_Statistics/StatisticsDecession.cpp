#include "StatisticsDecession.h"
#include "CONFIGRATIONS.H"

TradeManagerPolicy* m_glob_tradeMngrPolicy = nullptr; // Trade Manager Policy to get the configs from

CommonDirection StatisticsDecession::CalcHighestPipsChange(ChangeNode* start, DirectionStats* stats,int futureLen)
{
	int pipsChangeToCount = m_glob_tradeMngrPolicy->Pattern_Min_PipsChange_ToCount;

	int indexStart = m_historyDB_Map->GetIdx_Of_TimePoint(start->point_DateTimeLong);
	int pipChange = 0;
	int candlesCounter = 0;

	int maxChange = 1;
	int maxChangeCanleIdx = 0;
	int minChange = -1;
	int minChangeCanleIdx = 0;

	bool upD = false, downD = false;
	for (int i = indexStart; i < indexStart + futureLen && i < m_historyTL->size(); i++)
	{
		pipChange += m_historyTL->at(i)->Pip_Change;
		candlesCounter++;

		if (maxChange < pipChange)
		{
			maxChange = pipChange;
			maxChangeCanleIdx = candlesCounter;
		}

		if (minChange > pipChange)
		{
			minChange = pipChange;
			minChangeCanleIdx = candlesCounter;
		}
	}

	CommonDirection dire;
	
	if (std::abs(maxChange) > std::abs(minChange))
	{
		pipChange = maxChange;
		candlesCounter = maxChangeCanleIdx;
		dire = CommonDirection::UP;
	}
	else
	{

		pipChange = minChange;
		candlesCounter = minChangeCanleIdx;
		dire = CommonDirection::DOWN;
	}

	stats->add(pipChange, dire, candlesCounter, StatsType::Highest);
	return dire;
}


StatisticsDecession::StatisticsDecession(std::vector<ChangeNode*>* historyTL, DBMap* historyDB_Map)
{
	if (!m_glob_tradeMngrPolicy)
		m_glob_tradeMngrPolicy = Configrations::getInstance()->Get_Policy<TradeManagerPolicy>(PolicyType::TradeManagerPolicy);
	m_currentTL = nullptr;
	m_historyTL = historyTL;
	m_historyDB_Map = historyDB_Map;
	Cur_Orders_InRow = 0;
	m_orderCode = 0; // Need an logic to implement from DB
}

StatisticsDecession::StatisticsDecession(TimeLine* history)
{
	if (!m_glob_tradeMngrPolicy)
		m_glob_tradeMngrPolicy = Configrations::getInstance()->Get_Policy<TradeManagerPolicy>(PolicyType::TradeManagerPolicy);
	m_historyDB_Map = history->Get_DBMap();
	m_historyTL = history->Get_TimeLine();
	m_currentTL = nullptr;
	Cur_Orders_InRow = 0;
	m_orderCode = 0; // Need an logic to implement from DB
}

DirectionStats StatisticsDecession::DetectCommonDirection(std::vector<ChangeNode*>* potentioalRes, int futureLen) // TODO: Need to change the logic here to TimeLine
{
	DirectionStats stats;

	if (!potentioalRes || potentioalRes->empty())
		return stats;

	PatternGodStatisticsPolicy* policy = Configrations::getInstance()->Get_Policy<PatternGodStatisticsPolicy>(PolicyType::PatternGodStatisticsPolicy);
	stats.Set_TimePoint(m_currentTL->at(m_currentTL->size() - 1)->point_DateTime);
	stats.Activate();

	if (!futureLen)
		futureLen = policy->Score_Future_Vision_Candles;

	for (ChangeNode* node : *potentioalRes)
	{
		int generalPipChange = 0;
		int firstPipChange = 0;

		CommonDirection generalDir = CalcGeneralPipsChange(node, &stats, futureLen);
		CommonDirection firstDir = CalcFirstPipsChange(node, &stats, futureLen);
		CommonDirection HighestDir = CalcHighestPipsChange(node, &stats, futureLen);

	}

	return stats;
}

CommonDirection StatisticsDecession::CalcFirstPipsChange(ChangeNode* start, DirectionStats* stats,int futureLen)
{
	int indexStart = m_historyDB_Map->GetIdx_Of_TimePoint(start->point_DateTimeLong);
	int pipsChangeToCount = m_glob_tradeMngrPolicy->Pattern_Min_PipsChange_ToCount;

	int pipChange = 0;
	int candlesCounter = 0;
	bool upD = false, downD = false;
	
	for (int i = indexStart; i < indexStart + futureLen && i < m_historyTL->size(); i++)
	{
		pipChange += m_historyTL->at(i)->Pip_Change;
		candlesCounter++;
		if (pipChange >= pipsChangeToCount)
		{
			upD = true;
			break;

		}
		if (pipChange <= -pipsChangeToCount)
		{
			downD = true;
			break;
		}
	}
	CommonDirection dire;
	if (upD) dire = CommonDirection::UP;
	else if (downD) dire = CommonDirection::DOWN;
	else dire = CommonDirection::BLUR;

	stats->add(pipChange, dire, candlesCounter, StatsType::First);
	return dire;
}



CommonDirection StatisticsDecession::CalcGeneralPipsChange(ChangeNode* start, DirectionStats* stats,int futureLen)
{
	int pipsChangeToCount = m_glob_tradeMngrPolicy->Pattern_Min_PipsChange_ToCount;

	int indexStart = m_historyDB_Map->GetIdx_Of_TimePoint(start->point_DateTimeLong);
	int pipChange = 0;
	int candlesCounter = 0;

	bool upD = false, downD = false;

	for (int i = indexStart; i < indexStart + futureLen && i < m_historyTL->size(); i++)
	{
		candlesCounter++;
		pipChange += m_historyTL->at(i)->Pip_Change;
		stats->addStep_ForCandleIdx(pipChange, candlesCounter - 1);
		if (pipChange >= pipsChangeToCount)
		{
			upD = true;
		}

		if (pipChange <= -pipsChangeToCount) {
			downD = true;
		}
	}

	CommonDirection dire;
	if (upD && !downD) dire = CommonDirection::UP;
	else if (downD && !upD) dire = CommonDirection::DOWN;
	else dire = CommonDirection::BLUR;

	stats->add(pipChange, dire, candlesCounter, StatsType::General);
	return dire;
}
