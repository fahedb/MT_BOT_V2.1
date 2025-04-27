#pragma once
#include "ChangeData.h"
#include "Configrations.h"
#include <cmath>

class RsiScoreCalcs;
class EmaScoreCalcs;
class ZlsmaScoreCalcs;
class RelativeScoreCalcs;



class RelativeScoreCalcs
{
public:
	static inline int CalcScore(double hist, double curr, double weight = 10) // This code runs billions of times, so i tried to make it so fast, even if it's hard to understand
	{
		if (hist == 0 && curr == 0)
			return weight;

		const double absHist = std::abs(hist);
		const double absCurr = std::abs(curr);
		const double diff = std::abs(hist - curr);
		const double maxV = absHist > absCurr ? absHist : absCurr;

		const double matching = std::fmax(0.0, 1.0 - (diff / maxV));
		return static_cast<int>(matching * weight);
	}
};


/// <RSIScoreCalcs>
/// RSI Candle Score Calcs for RSI Score calcs
/// <RSIScoreCalcs>

class RsiScoreCalcs
{
public:

	static inline IndicatorScore Calc_AllLensScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr)
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

	inline static IndicatorScore CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr, IndicatorLen len, double weight)
	{
		// Max Score 18
		IndicatorScore score;
		const IndicatorValues& hist = ChNHistory->Get_Indicator_Pointer(IndicatorType::RSI, len)->Get_IndicatorValues();
		const IndicatorValues& curr = ChNcurr->Get_Indicator_Pointer(IndicatorType::RSI, len)->Get_IndicatorValues();

		score.M1 = ((RelativeScoreCalcs::CalcScore(hist.m_Slope_M1, curr.m_Slope_M1, weight)));
		score.M5 = ((RelativeScoreCalcs::CalcScore(hist.m_Slope_M5, curr.m_Slope_M5, weight)));
		score.M15 = ((RelativeScoreCalcs::CalcScore(hist.m_Slope_M15, curr.m_Slope_M15, weight))); 
		score.M30 = ((RelativeScoreCalcs::CalcScore(hist.m_Slope_M30, curr.m_Slope_M30, weight))); 
		score.H4 = ((RelativeScoreCalcs::CalcScore(hist.m_Slope_H4, curr.m_Slope_H4, weight)));	
		score.H1 = ((RelativeScoreCalcs::CalcScore(hist.m_Slope_H1, curr.m_Slope_H1, weight)));	

		score.M1 += (RelativeScoreCalcs::CalcScore(hist.M1, curr.M1, weight));
		score.M5 += (RelativeScoreCalcs::CalcScore(hist.M5, curr.M5, weight));
		score.M15 +=(RelativeScoreCalcs::CalcScore(hist.M15, curr.M15, weight));
		score.M30 +=(RelativeScoreCalcs::CalcScore(hist.M30, curr.M30, weight));
		score.H1 += (RelativeScoreCalcs::CalcScore(hist.H1, curr.H1, weight));
		score.H4 += (RelativeScoreCalcs::CalcScore(hist.H4, curr.H4, weight));

		score.calcTotal();
		return score;
	}
};

class General_MA_ScoreCalcs
{
public:
	static inline IndicatorScore CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr, double histLow, double histHigh, double currLow, double currHigh, IndicatorType type, IndicatorLen len /*IndicatorLen::L30*/, double weight)
	{
		IndicatorScore score;
		auto histIndPointer = ChNHistory->Get_Indicator_Pointer(type, len);
		auto currIndPointer = ChNcurr->Get_Indicator_Pointer(type, len);

		const IndicatorValues& hist = histIndPointer->Get_IndicatorValues();
		const IndicatorValues& curr = currIndPointer->Get_IndicatorValues();

		/// Slope of ema score
		score.M1 = RelativeScoreCalcs::CalcScore(hist.m_Slope_M1, curr.m_Slope_M1, weight);
		score.M5 = RelativeScoreCalcs::CalcScore(hist.m_Slope_M5, curr.m_Slope_M5, weight);
		score.M15 = RelativeScoreCalcs::CalcScore(hist.m_Slope_M15, curr.m_Slope_M15, weight);
		score.M30 = RelativeScoreCalcs::CalcScore(hist.m_Slope_M30, curr.m_Slope_M30, weight);
		score.H1 = RelativeScoreCalcs::CalcScore(hist.m_Slope_H1, curr.m_Slope_H1, weight);
		score.H4 = RelativeScoreCalcs::CalcScore(hist.m_Slope_H4, curr.m_Slope_H4, weight);


		double histRange = histHigh - histLow;
		double currRange = currHigh - currLow;


		double normHist =0;
		double normCurr =0;

		normHist = (hist.M1 - histLow) / histRange;
		normCurr = (curr.M1 - currLow) / currRange;
		score.M1 += RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);

		normHist = (hist.M5 - histLow) / histRange;
		normCurr = (curr.M5 - currLow) / currRange;
		score.M5 += RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);

		normHist = (hist.M15 - histLow) / histRange;
		normCurr = (curr.M15 - currLow) / currRange;
		score.M15 += RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);

		normHist = (hist.M30 - histLow) / histRange;
		normCurr = (curr.M30 - currLow) / currRange;
		score.M30 += RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);

		normHist = (hist.H1 - histLow) / histRange;
		normCurr = (curr.H1 - currLow) / currRange;
		score.H1 += RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);

		normHist = (hist.H4 - histLow) / histRange;
		normCurr = (curr.H4 - currLow) / currRange;
		score.H4 += RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);
		score.calcTotal();

		return score;

	}
};


/// <EmaScoreCalcs>
/// EMA Candle Score Calcs for EMA Score calcs
/// <EmaScoreCalcs>


class EmaScoreCalcs {
public:

	static inline IndicatorScore Calc_AllLensScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr)
	{
		double weight = Configrations::getInstance()->Get_Policy()->EMA_Wieght;
		double histLow = ChNHistory->Get_Entity(HTTP_BODY_MEMBER_LowPrice);
		double histHigh = ChNHistory->Get_Entity(HTTP_BODY_MEMBER_HighPrice);

		double currLow = ChNcurr->Get_Entity(HTTP_BODY_MEMBER_LowPrice);
		double currHigh = ChNcurr->Get_Entity(HTTP_BODY_MEMBER_HighPrice);

		IndicatorScore score = General_MA_ScoreCalcs::CalcScore(ChNHistory, ChNcurr, histLow, histHigh, currLow, currHigh, IndicatorType::EMA, IndicatorLen::L10, weight);
		score = score + General_MA_ScoreCalcs::CalcScore(ChNHistory, ChNcurr, histLow, histHigh, currLow, currHigh, IndicatorType::EMA, IndicatorLen::L15, weight);
		score = score + General_MA_ScoreCalcs::CalcScore(ChNHistory, ChNcurr, histLow, histHigh, currLow, currHigh, IndicatorType::EMA, IndicatorLen::L20, weight);
		score = score + General_MA_ScoreCalcs::CalcScore(ChNHistory, ChNcurr, histLow, histHigh, currLow, currHigh, IndicatorType::EMA, IndicatorLen::L25, weight);
		score = score + General_MA_ScoreCalcs::CalcScore(ChNHistory, ChNcurr, histLow, histHigh, currLow, currHigh, IndicatorType::EMA, IndicatorLen::L30, weight);
		return score;
	}
};

/// <VolumeScoreCalcs>
/// Volume Candle Score Calcs for Volume Score calcs
/// <VolumeScoreCalcs >

class VolumeScoreCalcs 
{
public:
	static IndicatorScore CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr) {
		auto histIndPointer = ChNHistory->Get_Indicator_Pointer(IndicatorType::VOLUME, IndicatorLen::NONE);
		auto currIndPointer = ChNcurr->Get_Indicator_Pointer(IndicatorType::VOLUME, IndicatorLen::NONE);

		IndicatorScore score;

		if (histIndPointer || currIndPointer)
			return score;

		int weight = Configrations::getInstance()->Get_Policy()->VOLUME_Wieght;

		IndicatorValues histindV = histIndPointer->Get_IndicatorValues();
		IndicatorValues currindV = currIndPointer->Get_IndicatorValues();

		score.M1 =	RelativeScoreCalcs::CalcScore(histindV.M1, currindV.M1, weight);
		score.M5 =	RelativeScoreCalcs::CalcScore(histindV.M5, currindV.M5, weight);
		score.M15 = RelativeScoreCalcs::CalcScore(histindV.M15, currindV.M15, weight);
		score.M30 = RelativeScoreCalcs::CalcScore(histindV.M30, currindV.M30, weight);
		score.H1 =	RelativeScoreCalcs::CalcScore(histindV.H1, currindV.H1, weight);
		score.H4 =	RelativeScoreCalcs::CalcScore(histindV.H4, currindV.H4, weight);

		score.M1  += RelativeScoreCalcs::CalcScore(histindV.m_Slope_M1,  currindV.m_Slope_M1, weight);
		score.M5  += RelativeScoreCalcs::CalcScore(histindV.m_Slope_M5,  currindV.m_Slope_M5, weight);
		score.M15 += RelativeScoreCalcs::CalcScore(histindV.m_Slope_M15, currindV.m_Slope_M15, weight);
		score.M30 += RelativeScoreCalcs::CalcScore(histindV.m_Slope_M30, currindV.m_Slope_M30, weight);
		score.H1  += RelativeScoreCalcs::CalcScore(histindV.m_Slope_H1,  currindV.m_Slope_H1, weight);
		score.H4  += RelativeScoreCalcs::CalcScore(histindV.m_Slope_H4,  currindV.m_Slope_H4, weight);

		return score;
	}
};


/// <ZlsmaScoreCalcs>
/// ZLSMA Candle Score Calcs for ZLSMA Score calcs
/// <ZlsmaScoreCalcs>
class ZlsmaScoreCalcs {
public:

	static inline IndicatorScore Calc_AllLensScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr)
	{
		double weight = Configrations::getInstance()->Get_Policy()->ZLSMA_Wieght;

		double histLow =  ChNHistory->Get_Entity(HTTP_BODY_MEMBER_LowPrice) ;
		double histHigh = ChNHistory->Get_Entity(HTTP_BODY_MEMBER_HighPrice);

		double currLow  = ChNcurr->Get_Entity(HTTP_BODY_MEMBER_LowPrice);
		double currHigh = ChNcurr->Get_Entity(HTTP_BODY_MEMBER_HighPrice);

		IndicatorScore score = General_MA_ScoreCalcs::CalcScore(ChNHistory, ChNcurr, histLow, histHigh, currLow, currHigh, IndicatorType::ZLSMA, IndicatorLen::L10, weight);
		score += General_MA_ScoreCalcs::CalcScore(ChNHistory, ChNcurr, histLow, histHigh, currLow, currHigh, IndicatorType::ZLSMA, IndicatorLen::L15, weight);
		score += General_MA_ScoreCalcs::CalcScore(ChNHistory, ChNcurr, histLow, histHigh, currLow, currHigh, IndicatorType::ZLSMA, IndicatorLen::L20, weight);
		score += General_MA_ScoreCalcs::CalcScore(ChNHistory, ChNcurr, histLow, histHigh, currLow, currHigh, IndicatorType::ZLSMA, IndicatorLen::L25, weight);
		score += General_MA_ScoreCalcs::CalcScore(ChNHistory, ChNcurr, histLow, histHigh, currLow, currHigh, IndicatorType::ZLSMA, IndicatorLen::L30, weight);

		return score;
	}
};



class ProxyScoreCalcs
{
public:
	static inline ScoresResult CalcMaxScorePossible(ChangeNode* chN) { return ProxyScoreCalcs::CompareChangeNodes(chN, chN); }

	static inline ScoresResult CompareChangeNodes(ChangeNode* historyNode, ChangeNode* currentNode)
	{
		ScoresResult score;
		// Calc all the scores
		score.rsiScore = RsiScoreCalcs::Calc_AllLensScore(historyNode, currentNode);
		score.emaScore = EmaScoreCalcs::Calc_AllLensScore(historyNode, currentNode);
		score.zlsmaScore = ZlsmaScoreCalcs::Calc_AllLensScore(historyNode, currentNode);
		score.volumeScore = VolumeScoreCalcs::CalcScore(historyNode, currentNode);
		// Sum all the scores into one Object
		score.calcTotalResult();
		return score;
	}
};