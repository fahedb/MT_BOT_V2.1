#include "GraphUtils.h"

GraphStructure GraphUtils::Build_Points(const std::vector<double>& points, int splitter) // Function to add X values to Y values into vector
{
	GraphStructure graphPoints;
	graphPoints.points.reserve(points.size());

	for (int i = 0; i < points.size(); i++)
		graphPoints.points.emplace_back(i, points.at(i));

	return graphPoints;
}

 std::vector<double> GraphUtils::GetNormalizedValues(std::vector<ChangeNode*>& vec, IndicatorType indType, IndicatorLen indLen, BarPeriod barP, IndicatorValueType indVT)
{
	bool changePipGraph = false;
	if (indType == IndicatorType::NONE && indType != IndicatorType::VOLUME)
		changePipGraph = true;

	std::vector<double> normalizedValue;
	normalizedValue.reserve(vec.size());

	for (int i = 0; i < vec.size(); i++)
	{
		if (changePipGraph) normalizedValue.push_back(static_cast<double>(vec.at(i)->Pip_Change));
		else normalizedValue.push_back(vec.at(i)->Get_Indicator_Pointer(indType, indLen)->Get_IndicatorValues_Pointer()->Get_Value(indVT, barP));
	}

	GraphUtils::NormalizeVectorValues(normalizedValue);
	return normalizedValue;

}
