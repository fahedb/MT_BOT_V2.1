#pragma once
#include <vector>
#include "robin_hood.h"
#include "Policies.h"

enum class PatternGodComparerType
{
	NONE = -1,
	PolynomComparer,
	ScoreComparer,
	CudaScoreComparer,
	COUNT = 3 // Total number of comparer types -- this should be updated if more comparer types are added

	// Add more comparer types as needed
};
struct PatternGodComparerTypeList { std::vector<PatternGodComparerType> types = { PatternGodComparerType::PolynomComparer , PatternGodComparerType::ScoreComparer }; };
