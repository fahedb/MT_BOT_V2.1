#include <vector>
#include "GraphStructres.h"
#include "ChangeData.h"
#include "MathExtentions.h"

#pragma once
class GaphUtils
{
private:
	static GraphStructure Build_Points(const std::vector<double>& points, int splitter = -1); // Function to add X values to Y values into vector - for private usage
	static std::vector<double> GetNormalizedValues(std::vector<ChangeNode*>& vec, IndicatorType indType = IndicatorType::NONE, IndicatorLen indLen = IndicatorLen::NONE, BarPeriod barP = BarPeriod::NONE);

	// TODO - Need to use tamplate here
	static void NormalizeVectorValues(std::vector<double>& vec)
	{
		double min = MathExtentions::GetMinValue(vec);

		for (int i = 0; i < vec.size(); i++)
			vec.at(i) = vec.at(i) - min;
	}

public:
	static GraphStructure Build_NormalizedGraph(std::vector<ChangeNode*> vec,IndicatorType indType = IndicatorType::NONE, IndicatorLen indLen = IndicatorLen::NONE, BarPeriod barP = BarPeriod::NONE)
	{
		GraphStructure graph;
		std::vector<double> yValues = GaphUtils::GetNormalizedValues(vec,indType,indLen,barP);
		graph.points.reserve(yValues.size());

		for (int i = 0; i < yValues.size(); i++)
		{
			graph.points.emplace_back(i, yValues.at(i));
		}
		return graph;
	}

};

