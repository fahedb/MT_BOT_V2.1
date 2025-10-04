#include "BaseVote.h"

#pragma once
class MajorityVote : public BaseVote
{
	virtual double CalculateWeight(double normalizedScore) override
	{
		return 1; // all votes have equal weights
	}
};

