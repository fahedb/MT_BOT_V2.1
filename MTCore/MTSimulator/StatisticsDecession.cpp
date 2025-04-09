#include "StatisticsDecession.h"
#include "JasonHandler.h"


CommonDirection StatisticsDecession::CalcFirstPipsChange(ChangeNode* start, DirectionStats* stats, int pipsChangeToCount)
{
	int indexStart = m_historyDB_Map->GetIdx_Of_TimePoint(start->point_DateTime);
	int futureLen = Configrations::getInstance()->Get_Policy()->Score_Future_Vision_Candles;
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



CommonDirection StatisticsDecession::CalcGeneralPipsChange(ChangeNode* start, DirectionStats* stats, int pipsChangeToCount)
{
	int indexStart = m_historyDB_Map->GetIdx_Of_TimePoint(start->point_DateTime);
	int futureLen = Configrations::getInstance()->Get_Policy()->Score_Future_Vision_Candles;
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
	if (upD) dire = CommonDirection::UP;
	else if (downD) dire = CommonDirection::DOWN;
	else dire = CommonDirection::BLUR;

	stats->add(pipChange, dire, candlesCounter, StatsType::General);
	return dire;
}
