#include "StatisticsDecession.h"
#include "JasonHandler.h"

void StatisticsDecession::Send_For_PNGExport(std::vector<ChangeNode*>* potentioalRes)
{
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
	if (!m_currentTL)
	{
		Logger::getInstance().WriteLog("Error: No Current TimeLine!!\nExport To PNG canceled");
		return;
	}
	std::vector<std::vector<ChangeNode*>*> similarPatterns;
	int timeLineLen = static_cast<int>(m_currentTL->size());
	for (int i = 0; i < potentioalRes->size(); i++)
	{
		int indexStart = m_historyDB_Map->GetIdx_Of_TimePoint(potentioalRes->at(i)->point_DateTime);
		std::vector<ChangeNode*>* currVector = new std::vector<ChangeNode*>(m_historyTL->begin() + (static_cast<size_t>(indexStart) - static_cast<size_t>(timeLineLen)), m_historyTL->begin() + (static_cast<size_t>(indexStart) + static_cast<size_t>(policy->Score_Future_Vision_Candles)));
		similarPatterns.push_back(currVector);
	}
	PatternExporter::Export_All_Pattens_ToPNG(m_currentTL, &similarPatterns);

	for (int i = 0; i < similarPatterns.size(); i++)
	{
		similarPatterns.at(i)->clear();
	}
	similarPatterns.clear();
}



bool StatisticsDecession::Trade_Decession_Logic(DirectionStats& stats)
{
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();

	int generalCount = stats.Get_TotalCount(StatsType::General);
	double upPercent = (double(stats.Get_UpCount(StatsType::General)) / generalCount) * 100.0;
	double downPercent = (double(stats.Get_DownCount(StatsType::General)) / generalCount) * 100.0;
	double blurPercent = (double(stats.Get_BlurCount(StatsType::General)) / generalCount) * 100.0;

	double meanPips = stats.mean(StatsType::General);

	if (generalCount >= policy->Patter_Min_Patterns_ToTake_Action)
	{
		bool isStrongUp = upPercent > policy->Pattern_Min_Percent_To_ExecuteOrder
			&& (meanPips) >= policy->Min_MeanPips_ToExecute_Order;

		bool isStrongDown = downPercent > policy->Pattern_Min_Percent_To_ExecuteOrder
			&& (meanPips) <= -policy->Min_MeanPips_ToExecute_Order;

		if (isStrongUp)
		{
			Send_TraddeSignal("XAUUSD", 0);
			return true;
		}
		else if (isStrongDown)
		{
			Send_TraddeSignal("XAUUSD", 1);
			return true;
		}
		else if (policy->Pattern_Activate_Alternative_ExecuteOrder)
		{
			bool isAlternativeAllowed = blurPercent <= policy->Pattern_Max_Noise_To_ExecuteOrder_Alternative
				&& std::fabs(downPercent - upPercent) >= policy->Pattern_Diff_Percent_To_ExecuteOrder_Alternative;

			if (isAlternativeAllowed)
			{
				bool isAlternativeUp = upPercent > downPercent && (meanPips) >= (policy->Min_MeanPips_ToExecute_Order + 10);
				bool isAlternativeDown = downPercent > upPercent && (meanPips) <= -(policy->Min_MeanPips_ToExecute_Order + 10);

				if (isAlternativeUp)
				{
					Send_TraddeSignal("XAUUSD", 0);
					return true;
				}
				else if (isAlternativeDown)
				{
					Send_TraddeSignal("XAUUSD", 1);
					return true;
				}
			}
		}
	}

	Cur_Orders_InRow = 0;
	return false;

}

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
