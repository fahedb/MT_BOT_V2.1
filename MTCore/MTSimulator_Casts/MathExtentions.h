#pragma once
#include <vector>

class MathExtentions
{
public:
	//TODO: Need to use template here 
	static double GetMinValue(const std::vector<double>& vec)
	{
		if (vec.size() == 0)
			return 0;

		double minValue = vec.at(0);

		for (int i = 0; i < vec.size(); i++)	if (minValue > vec.at(i)) minValue = vec.at(i);

		return minValue;

	}
};

