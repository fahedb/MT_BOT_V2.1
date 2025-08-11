#include "StatisticsDecession.h"
#include "JasonHandler.h"


CommonDirection StatisticsDecession::CalcHighestPipsChange(ChangeNode* start, DirectionStats* stats,int futureLen)
{
	int pipsChangeToCount = m_tradeMngrPolicy->Pattern_Min_PipsChange_ToCount;

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


CommonDirection StatisticsDecession::CalcFirstPipsChange(ChangeNode* start, DirectionStats* stats,int futureLen)
{
	int indexStart = m_historyDB_Map->GetIdx_Of_TimePoint(start->point_DateTimeLong);
	int pipsChangeToCount = m_tradeMngrPolicy->Pattern_Min_PipsChange_ToCount;

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
	int pipsChangeToCount = m_tradeMngrPolicy->Pattern_Min_PipsChange_ToCount;

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
