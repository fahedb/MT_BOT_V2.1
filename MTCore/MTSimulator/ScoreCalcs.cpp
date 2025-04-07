#include "ScoreCalcs.h"

///////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// RSI Calcs Functions////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

IndicatorScore RsiScoreCalcs::Calc_AllLensScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr)
{
	double weight = Configrations::getInstance()->Get_Policy()->RSI_Wieght;
	IndicatorScore score;
	score += RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L10, weight);
	score += RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L15, weight);
	score += RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L20, weight);
	score += RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L25, weight);
	score += RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L30, weight);
	return score;

}


IndicatorScore EmaScoreCalcs::CalcSlopScore_Logic(IndicatorValues hist, IndicatorValues curr, double weight)
{
	IndicatorScore score;
	score.M1 = RelativeScoreCalcs::CalcScore(hist.m_Slope_M1, curr.m_Slope_M1, weight);
	score.M5 = RelativeScoreCalcs::CalcScore(hist.m_Slope_M5, curr.m_Slope_M5, weight);
	score.M15 = RelativeScoreCalcs::CalcScore(hist.m_Slope_M15, curr.m_Slope_M15, weight);
	score.M30 = RelativeScoreCalcs::CalcScore(hist.m_Slope_M30, curr.m_Slope_M30, weight);
	score.H1 = RelativeScoreCalcs::CalcScore(hist.m_Slope_H1, curr.m_Slope_H1, weight);
	score.H4 = RelativeScoreCalcs::CalcScore(hist.m_Slope_H4, curr.m_Slope_H4, weight);

	return score;
}







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


IndicatorScore RsiScoreCalcs::CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr, IndicatorLen len, double weight)
{
	// Max Score 18
	IndicatorScore score;
	auto* histPtr = ChNHistory->Get_Indicator_Pointer(IndicatorType::RSI, len);
	auto* currPtr = ChNcurr->Get_Indicator_Pointer(IndicatorType::RSI, len);

	const IndicatorValues& hist = histPtr->Get_IndicatorValues();
	const IndicatorValues& curr = currPtr->Get_IndicatorValues();

	score = RsiScoreCalcs::CalcSlopScore_Logic(hist, curr, weight);

	score.M1 += static_cast<int>(RelativeScoreCalcs::CalcScore(hist.M1, curr.M1, weight));
	score.M5 += static_cast<int>(RelativeScoreCalcs::CalcScore(hist.M5, curr.M5, weight));
	score.M15 += static_cast<int>(RelativeScoreCalcs::CalcScore(hist.M15, curr.M15, weight));
	score.M30 += static_cast<int>(RelativeScoreCalcs::CalcScore(hist.M30, curr.M30, weight));
	score.H1 += static_cast<int>(RelativeScoreCalcs::CalcScore(hist.H1, curr.H1, weight));
	score.H4 += static_cast<int>(RelativeScoreCalcs::CalcScore(hist.H4, curr.H4, weight));

	score.calcTotal();
	return score;
}




IndicatorScore EmaScoreCalcs::CalcPlaceScore_Logic(IndicatorValues hist, double histRange, double histLow, IndicatorValues curr, double currRange,double currLow,double weight)
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



 IndicatorScore ZlsmaScoreCalcs::CalcSlopScore_Logic(IndicatorValues hist, IndicatorValues curr, double weight)
 {
	 IndicatorScore score;
	 score.M1 = RelativeScoreCalcs::CalcScore(hist.m_Slope_M1, curr.m_Slope_M1, weight);
	 score.M5 = RelativeScoreCalcs::CalcScore(hist.m_Slope_M5, curr.m_Slope_M5, weight);
	 score.M15 = RelativeScoreCalcs::CalcScore(hist.m_Slope_M15, curr.m_Slope_M15, weight);
	 score.M30 = RelativeScoreCalcs::CalcScore(hist.m_Slope_M30, curr.m_Slope_M30, weight);
	 score.H1 = RelativeScoreCalcs::CalcScore(hist.m_Slope_H1, curr.m_Slope_H1, weight);
	 score.H4 = RelativeScoreCalcs::CalcScore(hist.m_Slope_H4, curr.m_Slope_H4, weight);

	 return score;
 }

 IndicatorScore ZlsmaScoreCalcs::CalcPlaceScore_Logic(IndicatorValues hist, double histRange, double histLow, IndicatorValues curr, double currRange, double currLow, double weight)
 {

	 double normHist = (hist.M1 - histLow) / histRange;
	 double normCurr = (curr.M1 - currLow) / currRange;

	 IndicatorScore score;
	 score.M1 = RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);
	 score.M5 = RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);
	 score.M15 = RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);
	 score.M30 = RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);
	 score.H1 = RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);
	 score.H4 = RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);

	 return score;

 }

 IndicatorScore ZlsmaScoreCalcs::CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr, IndicatorLen len /*IndicatorLen::L30*/, double weight)
 {
	 IndicatorScore score;
	 IndicatorValues hist = ChNHistory->Get_Indicator_Pointer(IndicatorType::ZLSMA, len)->Get_IndicatorValues();
	 IndicatorValues curr = ChNcurr->Get_Indicator_Pointer(IndicatorType::ZLSMA, len)->Get_IndicatorValues();

	 score = ZlsmaScoreCalcs::CalcSlopScore_Logic(hist, curr, weight);

	 double histLow = ChNHistory->Get_Entity(HTTP_BODY_MEMBER_LowPrice);
	 double currLow = ChNcurr->Get_Entity(HTTP_BODY_MEMBER_LowPrice);

	 double histRange = ChNHistory->Get_Entity(HTTP_BODY_MEMBER_HighPrice) - histLow;
	 double currRange = ChNcurr->Get_Entity(HTTP_BODY_MEMBER_HighPrice) - currLow;

	 score += ZlsmaScoreCalcs::CalcPlaceScore_Logic(hist, histRange, histLow, curr, currRange, currLow, weight);

	 score.calcTotal();
	 return score;
 }

 IndicatorScore RsiScoreCalcs::CalcSlopScore_Logic(IndicatorValues hist, IndicatorValues curr, double rsiWeight)
 {
	 IndicatorScore score;
	 score.M1 = ((RelativeScoreCalcs::CalcScore(hist.m_Slope_M1, curr.m_Slope_M1, rsiWeight)));	// max 6
	 score.M5 = ((RelativeScoreCalcs::CalcScore(hist.m_Slope_M5, curr.m_Slope_M5, rsiWeight)));// max 4
	 score.M15 = ((RelativeScoreCalcs::CalcScore(hist.m_Slope_M15, curr.m_Slope_M15, rsiWeight))); // defualt max 2 * 4 = 8
	 score.M30 = ((RelativeScoreCalcs::CalcScore(hist.m_Slope_M30, curr.m_Slope_M30, rsiWeight))); // max 2
	 score.H1 = ((RelativeScoreCalcs::CalcScore(hist.m_Slope_H1, curr.m_Slope_H1, rsiWeight)));	// max 2
	 score.H4 = ((RelativeScoreCalcs::CalcScore(hist.m_Slope_H4, curr.m_Slope_H4, rsiWeight)));	// max 2

	 return score;
 }

 IndicatorScore EmaScoreCalcs::CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr, IndicatorLen len /*IndicatorLen::L30*/, double weight)
 {
	 IndicatorScore score;
	 IndicatorValues hist = ChNHistory->Get_Indicator_Pointer(IndicatorType::EMA, len)->Get_IndicatorValues();
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

