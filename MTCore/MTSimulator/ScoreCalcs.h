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
		if (hist == curr)
			return static_cast<int>(weight);

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
		double histLow = ChNHistory->m_lowPrice;
		double histHigh = ChNHistory->m_highPrice;

		double currLow = ChNcurr->m_lowPrice;
		double currHigh = ChNcurr->m_highPrice;

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

		if (!histIndPointer || !currIndPointer)
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

		double histLow =  ChNHistory->m_lowPrice ;
		double histHigh = ChNHistory->m_highPrice;

		double currLow = ChNcurr->m_lowPrice;
		double currHigh = ChNcurr->m_highPrice;

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

	static ScoresResult CalcAverageScore(ScoresResult score, int timeLineLen)
	{
		ScoresResult average;
		average.emaScore.M1 = score.emaScore.M1 / timeLineLen;
		average.emaScore.M5 = score.emaScore.M5 / timeLineLen;
		average.emaScore.M15 = score.emaScore.M15 / timeLineLen;
		average.emaScore.M30 = score.emaScore.M30 / timeLineLen;
		average.emaScore.H1 = score.emaScore.H1 / timeLineLen;
		average.emaScore.H4 = score.emaScore.H4 / timeLineLen;
		average.emaScore.total = score.emaScore.total / timeLineLen;

		average.zlsmaScore.M1 = score.zlsmaScore.M1 / timeLineLen;
		average.zlsmaScore.M5 = score.zlsmaScore.M5 / timeLineLen;
		average.zlsmaScore.M15 = score.zlsmaScore.M15 / timeLineLen;
		average.zlsmaScore.M30 = score.zlsmaScore.M30 / timeLineLen;
		average.zlsmaScore.H1 = score.zlsmaScore.H1 / timeLineLen;
		average.zlsmaScore.H4 = score.zlsmaScore.H4 / timeLineLen;
		average.zlsmaScore.total = score.zlsmaScore.total / timeLineLen;

		average.rsiScore.M1 = score.rsiScore.M1 / timeLineLen;
		average.rsiScore.M5 = score.rsiScore.M5 / timeLineLen;
		average.rsiScore.M15 = score.rsiScore.M15 / timeLineLen;
		average.rsiScore.M30 = score.rsiScore.M30 / timeLineLen;
		average.rsiScore.H1 = score.rsiScore.H1 / timeLineLen;
		average.rsiScore.H4 = score.rsiScore.H4 / timeLineLen;
		average.rsiScore.total = score.rsiScore.total / timeLineLen;

		average.volumeScore.M1 = score.volumeScore.M1 / timeLineLen;
		average.volumeScore.M5 = score.volumeScore.M5 / timeLineLen;
		average.volumeScore.M15 = score.volumeScore.M15 / timeLineLen;
		average.volumeScore.M30 = score.volumeScore.M30 / timeLineLen;
		average.volumeScore.H1 = score.volumeScore.H1 / timeLineLen;
		average.volumeScore.H4 = score.volumeScore.H4 / timeLineLen;
		average.volumeScore.total = score.volumeScore.total / timeLineLen;

		average.totalScore.M1 = score.totalScore.M1 / timeLineLen;
		average.totalScore.M5 = score.totalScore.M5 / timeLineLen;
		average.totalScore.M15 = score.totalScore.M15 / timeLineLen;
		average.totalScore.M30 = score.totalScore.M30 / timeLineLen;
		average.totalScore.H1 = score.totalScore.H1 / timeLineLen;
		average.totalScore.H4 = score.totalScore.H4 / timeLineLen;
		average.totalScore.total = score.totalScore.total / timeLineLen;

		return average;
	}

};