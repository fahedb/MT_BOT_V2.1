#include "ScoreCalcs.h"


class RsiScoreCalcs;
class EmaScoreCalcs;
class ZlsmaScoreCalcs;
class RelativeScoreCalcs;

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


IndicatorScore ZlsmaScoreCalcs::Calc_AllLensScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr)
{
	double weight = Configrations::getInstance()->Get_Policy()->ZLSMA_Wieght;
	IndicatorScore score = ZlsmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L10, weight);
	score = score + ZlsmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L15, weight);
	score = score + ZlsmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L20, weight);
	score = score + ZlsmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L25, weight);
	score = score + ZlsmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L30, weight);

	return score;
}


