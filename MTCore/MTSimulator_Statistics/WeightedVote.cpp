#include "WeightedVote.h"
#include "VoteWeightMethodEnum.h"
#include "Configrations.h"

double m_glob_powerlaw_exponent_for_Weighting;
double m_glob_softmax_T_param_for_Weighting;
WeightingMethod m_glob_weightingMethod;

void InitializeGlobals()
{
	Configrations* config = Configrations::getInstance();
	PatternGodStatisticsPolicy* statisPolicy = config->Get_Policy<PatternGodStatisticsPolicy>(PolicyType::PatternGodStatisticsPolicy);
	m_glob_powerlaw_exponent_for_Weighting = statisPolicy->powerlaw_exponent_for_Weighting;
	m_glob_softmax_T_param_for_Weighting = statisPolicy->softmax_T_param_for_Weighting;
	m_glob_weightingMethod = static_cast<WeightingMethod>(statisPolicy->Weighting_Method_for_Weighting);

}

double WeightedVote::CalculateWeight(double normalizedScore)
{
	switch (m_glob_weightingMethod)
	{
	case WeightingMethod::Linear:
		return normalizedScore; // Linear weight
	case WeightingMethod::PowerLaw:
		return std::pow(normalizedScore, m_glob_powerlaw_exponent_for_Weighting); // Power-law weight
	case WeightingMethod::SoftMax:
		return std::exp(normalizedScore / m_glob_softmax_T_param_for_Weighting); // Softmax weight
	default:
		return normalizedScore; // Default to linear if method is unknown
	}
}
