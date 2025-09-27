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
	static int CalcScore(double hist, double curr, double weight = 10);
};


/// <RSIScoreCalcs>
/// RSI Candle Score Calcs for RSI Score calcs
/// <RSIScoreCalcs>

class RsiScoreCalcs
{
public:

	static IndicatorScore Calc_AllLensScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr);

	static IndicatorScore CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr, IndicatorLen len, double weight);
};

class General_MA_ScoreCalcs
{
public:
	static IndicatorScore CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr, double histLow, double histHigh, double currLow, double currHigh, IndicatorType type, IndicatorLen len /*IndicatorLen::L30*/, double weight);
};


/// <EmaScoreCalcs>
/// EMA Candle Score Calcs for EMA Score calcs
/// <EmaScoreCalcs>


class EmaScoreCalcs {
public:

	static inline IndicatorScore Calc_AllLensScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr);
};

/// <VolumeScoreCalcs>
/// Volume Candle Score Calcs for Volume Score calcs
/// <VolumeScoreCalcs >

class VolumeScoreCalcs 
{
public:
	static IndicatorScore CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr);
};


/// <ZlsmaScoreCalcs>
/// ZLSMA Candle Score Calcs for ZLSMA Score calcs
/// <ZlsmaScoreCalcs>
class ZlsmaScoreCalcs {
public:
	static IndicatorScore Calc_AllLensScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr);
};



class ProxyScoreCalcs
{
public:
	static ScoresResult CalcMaxScorePossible(ChangeNode* chN) { return ProxyScoreCalcs::CompareChangeNodes(chN, chN); }

	static ScoresResult CompareChangeNodes(ChangeNode* historyNode, ChangeNode* currentNode);

	static ScoresResult CalcAverageScore(ScoresResult score, int timeLineLen);

};