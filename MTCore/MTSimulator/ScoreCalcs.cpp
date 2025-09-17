#include "ScoreCalcs.h"

const BarPeriodList m_glob_barPeriods;
const IndicatorTypeList m_glob_indTypes;
const IndicatorLenList m_glob_indLengths;


///////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// RSI Calcs Functions////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

IndicatorScore RsiScoreCalcs::Calc_AllLensScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr)
{
	double weight = Configrations::getInstance()->Get_Policy<ScoreComparerPolicy>(PolicyType::ScoreComparerPolicy)->RSI_Wieght;
	IndicatorScore score;
	score += RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L10, weight);
	score += RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L15, weight);
	score += RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L20, weight);
	score += RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L25, weight);
	score += RsiScoreCalcs::CalcScore(ChNHistory, ChNcurr, IndicatorLen::L30, weight);
	return score;

}

IndicatorScore RsiScoreCalcs::CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr, IndicatorLen len, double weight)
{
	// Max Score 18
	IndicatorScore score;
	const IndicatorValues& hist = ChNHistory->Get_Indicator_Pointer(IndicatorType::RSI, len)->Get_IndicatorValues();
	const IndicatorValues& curr = ChNcurr->Get_Indicator_Pointer(IndicatorType::RSI, len)->Get_IndicatorValues();

	for (const auto period : m_glob_barPeriods.periods) { // Iterate through all BarPeriods
		if (period == BarPeriod::NONE) continue; // Skip NONE period
		double localScore = 0;

		localScore = RelativeScoreCalcs::CalcScore(hist.Get_Value(IndicatorValueType::Slope, period), curr.Get_Value(IndicatorValueType::Slope, period), weight); // Calculate the score for each BarPeriod
		localScore += RelativeScoreCalcs::CalcScore(hist.Get_Value(IndicatorValueType::Value, period), curr.Get_Value(IndicatorValueType::Value, period), weight); // Add the value score for each BarPeriod
		score.m_scores[static_cast<int>(period)] = localScore; // Store the score for the BarPeriod
	}

	score.calcTotal();
	return score;
}

IndicatorScore General_MA_ScoreCalcs::CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr, double histLow, double histHigh, double currLow, double currHigh, IndicatorType type, IndicatorLen len, double weight)
{
	IndicatorScore score;
	auto histIndPointer = ChNHistory->Get_Indicator_Pointer(type, len);
	auto currIndPointer = ChNcurr->Get_Indicator_Pointer(type, len);

	const IndicatorValues& hist = histIndPointer->Get_IndicatorValues();
	const IndicatorValues& curr = currIndPointer->Get_IndicatorValues();


	for (auto& period : m_glob_barPeriods.periods) { // Iterate through all BarPeriods for slopes
		if (period == BarPeriod::NONE) continue; // Skip NONE period
		score.m_scores[static_cast<int>(period)] = RelativeScoreCalcs::CalcScore(hist.Get_Value(IndicatorValueType::Slope, period), curr.Get_Value(IndicatorValueType::Slope, period), weight); // Calculate the score for each BarPeriod
	}

	double histRange = histHigh - histLow;
	double currRange = currHigh - currLow;

	double normHist = 0;
	double normCurr = 0;

	for (auto& period : m_glob_barPeriods.periods) { // Iterate through all BarPeriods for values
		if (period == BarPeriod::NONE) continue; // Skip NONE period
		normHist = (hist.Get_Value(IndicatorValueType::Value, period) - histLow) / histRange;
		normCurr = (curr.Get_Value(IndicatorValueType::Value, period) - currLow) / currRange;
		score.m_scores[static_cast<int>(period)] += RelativeScoreCalcs::CalcScore(normHist, normCurr, weight);
	}

	score.calcTotal();

	return score;

}

ScoresResult ProxyScoreCalcs::CompareChangeNodes(ChangeNode* historyNode, ChangeNode* currentNode)
{
	ScoresResult score;
	score.Reset();

	// Calc all the scores
	score.indicatorScores.at(static_cast<int>(IndicatorType::RSI)) = RsiScoreCalcs::Calc_AllLensScore(historyNode, currentNode);
	score.indicatorScores.at(static_cast<int>(IndicatorType::EMA)) = EmaScoreCalcs::Calc_AllLensScore(historyNode, currentNode);
	score.indicatorScores.at(static_cast<int>(IndicatorType::ZLSMA)) = ZlsmaScoreCalcs::Calc_AllLensScore(historyNode, currentNode);

	//score.volumeScore = VolumeScoreCalcs::CalcScore(historyNode, currentNode);
	// Sum all the scores into one Object
	score.calcTotalResult();
	return score;
}

ScoresResult ProxyScoreCalcs::CalcAverageScore(ScoresResult score, int timeLineLen)
{
	ScoresResult average;
	average.totalScore.Reset();



	for (const auto& type : m_glob_indTypes.types) {
		for (const auto& period : m_glob_barPeriods.periods) {
			if (period == BarPeriod::NONE) continue; // Skip NONE period
			//				auto& scoreContainer = average.indicatorScores[type].scores[period];
			average.indicatorScores[static_cast<int>(type)].m_scores[static_cast<int>(period)] = score.indicatorScores[static_cast<int>(type)].m_scores[static_cast<int>(period)] / timeLineLen; // Calculate the average score for each BarPeriod
		}

	}

	return average;
}


IndicatorScore ZlsmaScoreCalcs::Calc_AllLensScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr)
{
	double weight = Configrations::getInstance()->Get_Policy<ScoreComparerPolicy>(PolicyType::ScoreComparerPolicy)->ZLSMA_Wieght;

	double histLow = ChNHistory->m_lowPrice;
	double histHigh = ChNHistory->m_highPrice;

	double currLow = ChNcurr->m_lowPrice;
	double currHigh = ChNcurr->m_highPrice;
	IndicatorScore score;
	score.Reset();

	for (const auto& len : m_glob_indLengths.lengths) {
		if (len == IndicatorLen::NONE) continue; // Skip NONE length
		score += General_MA_ScoreCalcs::CalcScore(ChNHistory, ChNcurr, histLow, histHigh, currLow, currHigh, IndicatorType::ZLSMA, len, weight);
	}
	return score;
}


IndicatorScore VolumeScoreCalcs::CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr) {
	auto histIndPointer = ChNHistory->Get_Indicator_Pointer(IndicatorType::VOLUME, IndicatorLen::NONE);
	auto currIndPointer = ChNcurr->Get_Indicator_Pointer(IndicatorType::VOLUME, IndicatorLen::NONE);

	IndicatorScore score;

	if (!histIndPointer || !currIndPointer)
		return score;

	int weight = Configrations::getInstance()->Get_Policy<ScoreComparerPolicy>(PolicyType::ScoreComparerPolicy)->VOLUME_Wieght;

	IndicatorValues histindV = histIndPointer->Get_IndicatorValues();
	IndicatorValues currindV = currIndPointer->Get_IndicatorValues();

	for (auto& period : m_glob_barPeriods.periods) { // Iterate through all BarPeriods
		if (period == BarPeriod::NONE) continue; // Skip NONE period
		double localScore = 0;

		localScore = RelativeScoreCalcs::CalcScore(histindV.Get_Value(IndicatorValueType::Slope, period), currindV.Get_Value(IndicatorValueType::Slope, period), weight); // Calculate the score for each BarPeriod
		localScore += RelativeScoreCalcs::CalcScore(histindV.Get_Value(IndicatorValueType::Value, period), currindV.Get_Value(IndicatorValueType::Value, period), weight); // Add the value score for each BarPeriod
		score.m_scores[int(period)] = localScore; // Store the score for the BarPeriod
	}

	return score;
}

IndicatorScore EmaScoreCalcs::Calc_AllLensScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr)
{
	double weight = Configrations::getInstance()->Get_Policy<ScoreComparerPolicy>(PolicyType::ScoreComparerPolicy)->EMA_Wieght;
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

int RelativeScoreCalcs::CalcScore(double hist, double curr, double weight) // This code runs billions of times, so i tried to make it so fast, even if it's hard to understand
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
