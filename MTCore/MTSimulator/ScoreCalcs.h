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

	static IndicatorScore CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr, IndicatorLen len /*IndicatorLen::L30*/, double weight);
private:

	static IndicatorScore CalcPlaceScore_Logic(IndicatorValues hist, double histRange, double histLow, IndicatorValues curr, double currRange, double currLow, double weight);
	static IndicatorScore CalcSlopScore_Logic(IndicatorValues hist, IndicatorValues curr, double weight);
};


