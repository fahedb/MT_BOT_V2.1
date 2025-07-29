#include "PolynomScoreCalcs.h"

double CalcSinglePolynomScore(Polynom* hist, Polynom* curr, PolynomPolicy* policy, const int weight = 10) {
	if (!hist || !curr)
		return 0.0;

	double wEval = policy->weight_eval;
	double wRMSE = policy->weight_rmse;
	double wCoeff = policy->weight_coeff;


	double scoreEval = PolynomComparer::Compare_ByEval(*hist, *curr);
	double scoreRMSE = PolynomComparer::Compare_ByRMSE(*hist, *curr);
	double scoreCoeff = PolynomComparer::Compare_ByWeightedCoefficients(*hist, *curr);
	double score = wEval * scoreEval + wRMSE * scoreRMSE + wCoeff * scoreCoeff;
	return score * weight;

}

TimeLinePolynomScore PolynomScoreCalcs::CalcScore(PolynomChangeNodePackage* hist, PolynomChangeNodePackage* curr)
{
	TimeLinePolynomScore score;

	if (hist == nullptr || curr == nullptr) {
		return score; // Return empty score if either package is null
	}
	// Calculate EMA scores for each polynomial

	IndicatorTypeList indTypes;
	IndicatorLenList indLengths;
	IndicatorValueTypeList indValueTypes;

	PolynomPolicy* policy = Configrations::getInstance()->Get_PolynomPolicy();

	for(int i =0 ; i < indTypes.types.size(); i++)
	{
		for(int j = 0; j < indLengths.lengths.size(); j++)
		{
			for(int k = 0; k < indValueTypes.valueTypes.size(); k++)
			{
				auto polynomKey = PolynomKey(indTypes.types[i], indLengths.lengths[j], indValueTypes.valueTypes[k]);
				score.GetScoreContainer(indValueTypes.valueTypes[k])->AddScore(indLengths.lengths[j], CalcSinglePolynomScore(hist->GetPolynom(polynomKey), curr->GetPolynom(polynomKey), policy));
			}
		}
	}

	// Calculate the total score
	score.GetScoreContainer(IndicatorValueType::Value)->CalculateTotalScore();
	score.GetScoreContainer(IndicatorValueType::Slope)->CalculateTotalScore();
	return score;
}
