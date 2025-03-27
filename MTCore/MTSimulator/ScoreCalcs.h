#pragma once
#include "ChangeData.h"
#include "Configrations.h"



class RelativeScoreCalcs
{
public:
	static inline int CalcScore(double hist, double curr, double weight = 10) // This code runs billions of times, so i tried to make it so fast, even if it's hard to understand
	{
		if (hist == 0.0 || curr == 0.0)
			return (hist == 0.0 && curr == 0.0) ? weight : 0;

		double absHist = std::fabs(hist);
		double absCurr = std::fabs(curr);
		double maxV = absHist > absCurr ? absHist : absCurr;

		double diff = std::fabs(hist - curr);
		double matching = 1.0 - (diff / maxV);
		return static_cast<int>(matching * weight);
	}
};


/// <BasicCandleScoreCalcs>
/// Basic Candle Score Calcs for basic score calcs
/// <BasicCandleScoreCalcs>
class BasincCandleScoreCalcs 
{
public:
	static int CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr);
};


/// <RSIScoreCalcs>
/// RSI Candle Score Calcs for RSI Score calcs
/// <RSIScoreCalcs>

class RsiScoreCalcs
{
public:
	static IndicatorScore Calc_AllLensScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr);
	static IndicatorScore CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr,IndicatorLen len = IndicatorLen::L30,double weight = 30.0);
private:
	static bool CheckSlopeDirectior(double hist, double curr);

	static IndicatorScore CalcSlopScore_Logic(IndicatorValues hist, IndicatorValues curr,double rsiWeight = 30.0);
};


/// <EmaScoreCalcs>
/// EMA Candle Score Calcs for EMA Score calcs
/// <EmaScoreCalcs>


class EmaScoreCalcs {
public:
	static IndicatorScore Calc_AllLensScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr);
	static IndicatorScore CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr, IndicatorLen len = IndicatorLen::L30,double weight = 30.0);
private:
	static IndicatorScore CalcSlopScore_Logic(IndicatorValues hist, IndicatorValues curr,double weight);
	static IndicatorScore CalcPlaceScore_Logic(IndicatorValues hist, double histRange, double histLow, IndicatorValues curr, double currRange, double currLow,double weight);
};

/// <GravityScoreCalcs >
/// Gravity Candle Score Calcs for Gravity Score calcs
/// <RSIScoreCalcs>

class GravityScoreCalcs 
{
public:
	static int CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr);
};


/// <ZlsmaScoreCalcs>
/// ZLSMA Candle Score Calcs for ZLSMA Score calcs
/// <ZlsmaScoreCalcs>

// Not Finished class -- TODO: Need to finish it and integrate it in the program
class ZlsmaScoreCalcs {
public:
	static IndicatorScore Calc_AllLensScore(ChangeNode* ChNHistory,ChangeNode* ChNcurr)
	{
		double weight = Configrations::getInstance()->Get_Policy()->ZLSMA_Wieght;
		IndicatorScore score = ZlsmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L10, weight);
		score = score + ZlsmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L15, weight);
		score = score + ZlsmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L20, weight);
		score = score + ZlsmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L25, weight);
		score = score + ZlsmaScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L30, weight);

		return score;
	}

	static inline IndicatorScore CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr, IndicatorLen len /*IndicatorLen::L30*/, double weight)
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
private:

	static inline IndicatorScore CalcPlaceScore_Logic(IndicatorValues hist, double histRange, double histLow, IndicatorValues curr, double currRange, double currLow, double weight)
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

	static inline IndicatorScore CalcSlopScore_Logic(IndicatorValues hist, IndicatorValues curr, double weight)
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

};


