#pragma once
#include "ChangeData.h"
#include "Configrations.h"

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
	static IndicatorScore Calc_AllLensScores(ChangeNode* ChNHistory, ChangeNode* ChNcurr);
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
	static IndicatorScore CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr, IndicatorLen len = IndicatorLen::L30)
	{
		// Max Score 18
		IndicatorScore score;
		IndicatorValues hist = ChNHistory->Get_Indicator_Pointer(IndicatorType::ZLSMA,len)->Get_IndicatorValues();
		IndicatorValues curr = ChNcurr->Get_Indicator_Pointer(IndicatorType::ZLSMA, len)->Get_IndicatorValues();

		//score += BaseScoreCalcs::CalcSlopScore_Logic(hist, curr);

		//if (CheckPointPlace(hist.M1, ChNHistory->HighPrice, ChNHistory->LowPrice, curr.M1, ChNcurr->HighPrice, ChNcurr->LowPrice)) score+=3;
		//if (CheckPointPlace(hist.M5, ChNHistory->HighPrice, ChNHistory->LowPrice, curr.M5, ChNcurr->HighPrice, ChNcurr->LowPrice)) score+=2;
		//if (CheckPointPlace(hist.M15, ChNHistory->HighPrice, ChNHistory->LowPrice, curr.M15, ChNcurr->HighPrice, ChNcurr->LowPrice)) score++;
		//if (CheckPointPlace(hist.M30, ChNHistory->HighPrice, ChNHistory->LowPrice, curr.M30, ChNcurr->HighPrice, ChNcurr->LowPrice)) score++;
		//if (CheckPointPlace(hist.H1, ChNHistory->HighPrice, ChNHistory->LowPrice, curr.H1, ChNcurr->HighPrice, ChNcurr->LowPrice)) score++;
		//if (CheckPointPlace(hist.H4, ChNHistory->HighPrice, ChNHistory->LowPrice, curr.H4, ChNcurr->HighPrice, ChNcurr->LowPrice)) score++;

		return score;
	}
private:
	static bool CheckPointPlace(double histEma, double histHighPrice, double histLowPrice, double currEma, double currHighPrice, double currLowPrice)
	{
		if (histEma > histHighPrice && currEma > currHighPrice)
			return true;
		if (histEma < histLowPrice && currEma < currLowPrice)
			return true;

		if ((histEma > histLowPrice && histEma < histHighPrice) && (currEma > currLowPrice && currEma < currHighPrice))
			return true;

		return false;
	}
};


class RelativeScoreCalcs
{
public:
	static inline int CalcScore(double hist, double curr,double weight = 10) // This code runs billions of times, so i tried to make it so fast, even if it's hard to understand
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
