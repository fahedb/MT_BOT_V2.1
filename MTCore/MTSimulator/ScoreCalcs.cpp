#include "ScoreCalcs.h"

IndicatorScore EmaScoreCalcs::Calc_AllLensScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr)
{
	double weight = Configrations::getInstance()->Get_Policy()->EMA_Wieght;
	IndicatorScore score = EmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L10,weight);
	score = score + EmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L15,weight);
	score = score + EmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L20,weight);
	score = score + EmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L25,weight);
	score = score + EmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L30,weight);

	return score;
}

IndicatorScore EmaScoreCalcs::CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr,IndicatorLen len /*IndicatorLen::L30*/,double weight)
{
	IndicatorScore score;
	IndicatorValues hist = ChNHistory->Get_Indicator_Pointer(IndicatorType::EMA,len)->Get_IndicatorValues();
	IndicatorValues curr = ChNcurr->Get_Indicator_Pointer(IndicatorType::EMA, len)->Get_IndicatorValues();

	score = EmaScoreCalcs::CalcSlopScore_Logic(hist, curr, weight);
	
	double histLow = ChNHistory->Get_Entity(HTTP_BODY_MEMBER_LowPrice);
	double currLow = ChNcurr->Get_Entity(HTTP_BODY_MEMBER_LowPrice);

	double histRange = ChNHistory->Get_Entity(HTTP_BODY_MEMBER_HighPrice) - histLow;
	double currRange = ChNcurr->Get_Entity(HTTP_BODY_MEMBER_HighPrice) - currLow;

	score += EmaScoreCalcs::CalcPlaceScore_Logic(hist, histRange, histLow, curr, currRange, currLow, weight);

	score.calcTotal();
	return score;
}
 
inline IndicatorScore EmaScoreCalcs::CalcPlaceScore_Logic(IndicatorValues hist, double histRange, double histLow, IndicatorValues curr, double currRange,double currLow,double weight)
{
	
	double normHist = (hist.M1 - histLow) / histRange;
	double normCurr = (curr.M1 - currLow) / currRange;

	IndicatorScore score;
	score.M1 = RelativeScoreCalcs::CalcScore(normHist, normCurr,weight); // Need to add weight parameter !!!!
	score.M5 = RelativeScoreCalcs::CalcScore(normHist, normCurr,weight);
	score.M15 = RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);
	score.M30 = RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);
	score.H1 = RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);
	score.H4 = RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);

	return score;

}

 inline IndicatorScore EmaScoreCalcs::CalcSlopScore_Logic(IndicatorValues hist, IndicatorValues curr,double weight)
 {
	 IndicatorScore score;
	 score.M1	= RelativeScoreCalcs::CalcScore(hist.m_Slope_M1, curr.m_Slope_M1,weight);
	 score.M5	= RelativeScoreCalcs::CalcScore(hist.m_Slope_M5, curr.m_Slope_M5,weight);
	 score.M15	= RelativeScoreCalcs::CalcScore(hist.m_Slope_M15, curr.m_Slope_M15,weight);
	 score.M30	= RelativeScoreCalcs::CalcScore(hist.m_Slope_M30, curr.m_Slope_M30,weight);
	 score.H1	= RelativeScoreCalcs::CalcScore(hist.m_Slope_H1, curr.m_Slope_H1,weight);
	 score.H4	= RelativeScoreCalcs::CalcScore(hist.m_Slope_H4, curr.m_Slope_H4,weight);

	 return score;
 }



 int BasincCandleScoreCalcs::CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr)
 {
	 // Max Score 2
	 int score = 0;
	 //if (ChNcurr-Pip_Change > 0 && ChNHistory->Pip_Change > 0) score++;
	 //else if (ChNcurr->Pip_Change < 0 && ChNHistory->Pip_Change < 0) score++;

	 if (ChNcurr->Increase_Decrease && ChNHistory->Increase_Decrease)
	 {
		 if (ChNcurr->Get_Entity(HTTP_BODY_MEMBER_HighPrice) == ChNcurr->Get_Entity(HTTP_BODY_MEMBER_ClosePrice) 
			 && ChNHistory->Get_Entity(HTTP_BODY_MEMBER_HighPrice) == ChNHistory->Get_Entity(HTTP_BODY_MEMBER_ClosePrice)) score++;
		 else if (ChNcurr->Get_Entity(HTTP_BODY_MEMBER_LowPrice) == ChNcurr->Get_Entity(HTTP_BODY_MEMBER_OpenPrice) && ChNHistory->Get_Entity(HTTP_BODY_MEMBER_LowPrice) == ChNHistory->Get_Entity(HTTP_BODY_MEMBER_OpenPrice)) score++;
	 }
	 else if (!ChNcurr->Increase_Decrease && !ChNHistory->Increase_Decrease)
	 {
		 if (ChNcurr->Get_Entity(HTTP_BODY_MEMBER_HighPrice) == ChNcurr->Get_Entity(HTTP_BODY_MEMBER_ClosePrice)
			 && ChNHistory->Get_Entity(HTTP_BODY_MEMBER_HighPrice) == ChNHistory->Get_Entity(HTTP_BODY_MEMBER_ClosePrice)) score++;
		 else if (ChNcurr->Get_Entity(HTTP_BODY_MEMBER_LowPrice) == ChNcurr->Get_Entity(HTTP_BODY_MEMBER_OpenPrice) && ChNHistory->Get_Entity(HTTP_BODY_MEMBER_LowPrice) == ChNHistory->Get_Entity(HTTP_BODY_MEMBER_OpenPrice)) score++;
	 }
	 return score;
 }
 IndicatorScore RsiScoreCalcs::Calc_AllLensScores(ChangeNode* ChNHistory, ChangeNode* ChNcurr)
 {
	 double weight = Configrations::getInstance()->Get_Policy()->RSI_Wieght;
	 IndicatorScore score;
	 score +=RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L10,weight);
	 score +=RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L15,weight);
	 score +=RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L20,weight);
	 score +=RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L25,weight);
	 score +=RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L30,weight);
	 return score;

 }
 IndicatorScore RsiScoreCalcs::CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr,IndicatorLen len, double weight)
 {
	 // Max Score 18
	 IndicatorScore score;
	 auto* histPtr = ChNHistory->Get_Indicator_Pointer(IndicatorType::RSI, len);
	 auto* currPtr = ChNcurr->Get_Indicator_Pointer(IndicatorType::RSI, len);

	 const IndicatorValues& hist = histPtr->Get_IndicatorValues();
	 const IndicatorValues& curr = currPtr->Get_IndicatorValues();

	 score = RsiScoreCalcs::CalcSlopScore_Logic(hist, curr, weight);

	 score.M1 += static_cast<int>(RelativeScoreCalcs::CalcScore(hist.M1, curr.M1,weight));
	 score.M5 += static_cast<int>(RelativeScoreCalcs::CalcScore(hist.M5, curr.M5,weight));
	 score.M15 +=static_cast<int>(RelativeScoreCalcs::CalcScore(hist.M15, curr.M15,weight));
	 score.M30 +=static_cast<int>(RelativeScoreCalcs::CalcScore(hist.M30, curr.M30,weight));
	 score.H1 += static_cast<int>(RelativeScoreCalcs::CalcScore(hist.H1, curr.H1,weight));
	 score.H4 += static_cast<int>(RelativeScoreCalcs::CalcScore(hist.H4, curr.H4,weight));

	 score.calcTotal();
	 return score;
 }


 bool RsiScoreCalcs::CheckSlopeDirectior(double hist, double curr)
 {
	 if (curr > 0 && hist > 0)
		 return true;
	 if (curr < 0 && hist < 0)
		 return true;
	 return false;
 }

 inline IndicatorScore RsiScoreCalcs::CalcSlopScore_Logic(IndicatorValues hist, IndicatorValues curr,double rsiWeight)
 {
	 IndicatorScore score;
	 score.M1 =		((RelativeScoreCalcs::CalcScore(hist.m_Slope_M1, curr.m_Slope_M1		, rsiWeight)));	// max 6
	 score.M5 =		((RelativeScoreCalcs::CalcScore(hist.m_Slope_M5, curr.m_Slope_M5		,rsiWeight)));// max 4
	 score.M15 =	((RelativeScoreCalcs::CalcScore(hist.m_Slope_M15, curr.m_Slope_M15	,rsiWeight))); // defualt max 2 * 4 = 8
	 score.M30 =	((RelativeScoreCalcs::CalcScore(hist.m_Slope_M30, curr.m_Slope_M30	,rsiWeight))); // max 2
	 score.H1 =		((RelativeScoreCalcs::CalcScore(hist.m_Slope_H1, curr.m_Slope_H1		,rsiWeight)));	// max 2
	 score.H4 =		((RelativeScoreCalcs::CalcScore(hist.m_Slope_H4, curr.m_Slope_H4		,rsiWeight)));	// max 2

	 return score;
 }

 int GravityScoreCalcs::CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr)
 {
	 // Max Score 6
	 // Min Score -5
	 int score = 0;
	 Gravity* histGravity = ChNHistory->GetGravityInd()->GetGravity();
	 Gravity* currGravity = ChNcurr->GetGravityInd()->GetGravity();

	 if (currGravity->m_GravityStren > 0 && histGravity->m_GravityStren < 0) return -3;
	 if (currGravity->m_GravityStren < 0 && histGravity->m_GravityStren > 0) return -3;

	 double upperGravityLine = Configrations::getInstance()->Get_Policy()->Pattern_upperGravity_Check;
	 double lowerGravityLine = Configrations::getInstance()->Get_Policy()->Pattern_lowerGravity_Check;
	 double rangeGravityCheck = Configrations::getInstance()->Get_Policy()->Pattern_Gravity_Range_Check;

	 if (histGravity->m_GravityStren > 0 && currGravity->m_GravityStren > 0) score++;
	 if (histGravity->m_GravityStren < 0 && currGravity->m_GravityStren < 0) score++;

	 if (histGravity->m_GravityStren > upperGravityLine && currGravity->m_GravityStren > upperGravityLine) score++;
	 else if (histGravity->m_GravityStren < lowerGravityLine && currGravity->m_GravityStren < lowerGravityLine) score++;

	 if (std::abs(histGravity->m_GravityStren - currGravity->m_GravityStren) < rangeGravityCheck) score++;

	 if (histGravity->m_type == currGravity->m_type) score++;

	 return score;
 }
