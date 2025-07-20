#include "PolynomScoreCalcs.h"

inline PolynomScore PolynomScoreCalcs::CalcScore(PolynomChangeNodePackage* hist, PolynomChangeNodePackage* curr)
{
	PolynomScore score;
	if (hist == nullptr || curr == nullptr) {
		return score; // Return empty score if either package is null
	}
	// Calculate EMA scores for each polynomial
	score.L10 = CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L10, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L10, IndicatorValueType::Value)));
	score.L15 = CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L15, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L15, IndicatorValueType::Value)));
	score.L20 = CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L20, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L20, IndicatorValueType::Value)));
	score.L25 = CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L25, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L25, IndicatorValueType::Value)));
	score.L30 = CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L30, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L30, IndicatorValueType::Value)));

	// Calculate EMA slopes for each polynomial
	score.L10_slope = CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L10, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L10, IndicatorValueType::Slope)));
	score.L15_slope = CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L15, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L15, IndicatorValueType::Slope)));
	score.L20_slope = CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L20, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L20, IndicatorValueType::Slope)));
	score.L25_slope = CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L25, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L25, IndicatorValueType::Slope)));
	score.L30_slope = CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L30, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::EMA, IndicatorLen::L30, IndicatorValueType::Slope)));

	// calculate ZLSMA scores for each polynomial
	score.L10 += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L10, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L10, IndicatorValueType::Value)));
	score.L15 += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L15, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L15, IndicatorValueType::Value)));
	score.L20 += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L20, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L20, IndicatorValueType::Value)));
	score.L25 += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L25, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L25, IndicatorValueType::Value)));
	score.L30 += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L30, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L30, IndicatorValueType::Value)));

	// Calculate ZLSMA slopes for each polynomial
	score.L10_slope += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L10, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L10, IndicatorValueType::Slope)));
	score.L15_slope += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L15, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L15, IndicatorValueType::Slope)));
	score.L20_slope += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L20, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L20, IndicatorValueType::Slope)));
	score.L25_slope += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L25, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L25, IndicatorValueType::Slope)));
	score.L30_slope += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L30, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L30, IndicatorValueType::Slope)));

	// calculate RSI scores for each polynomial
	score.L10 += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L10, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L10, IndicatorValueType::Value)));
	score.L15 += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Value)));
	score.L20 += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L20, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L20, IndicatorValueType::Value)));
	score.L25 += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L25, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L25, IndicatorValueType::Value)));
	score.L30 += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L30, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L30, IndicatorValueType::Value)));

	// Calculate RSI slopes for each polynomial
	score.L10_slope += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L10, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L10, IndicatorValueType::Slope)));
	score.L15_slope += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Slope)));
	score.L20_slope += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L20, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L20, IndicatorValueType::Slope)));
	score.L25_slope += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L25, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L25, IndicatorValueType::Slope)));
	score.L30_slope += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L30, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L30, IndicatorValueType::Slope)));

	// calculate the Volume scores for each polynomial
	score.L10 += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::VOLUME, IndicatorLen::NONE, IndicatorValueType::Value)), curr->GetPolynom(PolynomKey(IndicatorType::VOLUME, IndicatorLen::NONE, IndicatorValueType::Value)));
	//calculate the Volume slopes for each polynomial
	score.L10_slope += CalcSinglePolynomScore(hist->GetPolynom(PolynomKey(IndicatorType::VOLUME, IndicatorLen::NONE, IndicatorValueType::Slope)), curr->GetPolynom(PolynomKey(IndicatorType::VOLUME, IndicatorLen::NONE, IndicatorValueType::Slope)));

	// Calculate the total score
	score.CalculateTotalScore();
	return score;
}
