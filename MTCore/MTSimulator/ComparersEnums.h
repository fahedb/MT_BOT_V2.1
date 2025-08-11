#pragma once
#include <vector>
#include "robin_hood.h"
#include "Policies.h"

enum class PatternGodComparerType
{
	NONE = -1,
	PolynomComparer,
	ScoreComparer,
	COUNT = 2 // Total number of comparer types -- this should be updated if more comparer types are added

	// Add more comparer types as needed
};
struct PatternGodComparerTypeList { std::vector<PatternGodComparerType> types = { PatternGodComparerType::PolynomComparer , PatternGodComparerType::ScoreComparer }; };

struct PatternGodComparerTypeWeights {
	explicit PatternGodComparerTypeWeights(const BaseComparerPolicy& p) {
		weights.reserve(2);
		weights[PatternGodComparerType::PolynomComparer] = p.PolynomComparerWeight;
		weights[PatternGodComparerType::ScoreComparer] = p.ScoreComparerWeight;
	}
	robin_hood::unordered_map<PatternGodComparerType, double> weights;
};