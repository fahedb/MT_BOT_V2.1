#pragma once
#include "BaseVote.h"

void InitializeGlobals(); // Call this function once at the start of the application to initialize global variables


class WeightedVote : public BaseVote
{
public:
	WeightedVote() : BaseVote()
	{
		InitializeGlobals(); // Ensure globals are initialized
	}
private:

	virtual double CalculateWeight(double normalizedScore) override;
};

