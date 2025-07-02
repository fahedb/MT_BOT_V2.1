#include "GaphUtils.h"
#include "BarPeriods.h"

GraphStructure GaphUtils::Build_Points(const std::vector<double>& points, int splitter) // Function to add X values to Y values into vector
{
	GraphStructure graphPoints;
	graphPoints.points.reserve(points.size());

	for (int i = 0; i < points.size(); i++)
		graphPoints.points.emplace_back(i, points.at(i));

	return graphPoints;
}

 std::vector<double> GaphUtils::GetNormalizedValues(std::vector<ChangeNode*>& vec, IndicatorType indType, IndicatorLen indLen, BarPeriod barP)
{
	bool changePipGraph = false;
	if (indType == IndicatorType::NONE && indType != IndicatorType::VOLUME)
		changePipGraph = true;

	std::vector<double> normalizedValue;
	normalizedValue.reserve(vec.size());

	for (int i = 0; i < vec.size(); i++)
	{
		if (changePipGraph) normalizedValue.push_back(static_cast<double>(vec.at(i)->Pip_Change));
		else normalizedValue.push_back(BarPeriod_Utils::Extract_Value(vec.at(i)->Get_Indicator_Pointer(indType, indLen), barP));
	}

	GaphUtils::NormalizeVectorValues(normalizedValue);
	return normalizedValue;

}
