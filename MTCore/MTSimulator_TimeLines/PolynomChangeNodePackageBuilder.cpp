#include "PolynomChangeNodePackageBuilder.h"
#include "PolynomTimeLine.h"
#include "robin_hood.h"
#include "GraphUtils.h"

IndicatorTypeList m_glob_indTypes; // List of indicator types
IndicatorLenList m_glob_indLengths; // List of indicator lengths
IndicatorValueTypeList m_glob_indValueTypes; // List of indicator value types (Value or Slope)


PolynomMap PolynomChangeNodePackageBuilder::BuildPolynomForChangeNodes(std::vector<ChangeNode*> nodes)
{
	PolynomMap cacledPolynomMap;

	// Build polynomials for different indicators and lengths
	for (const auto& type : m_glob_indTypes.types) // Iterate through each indicator type
	{
		for(const auto& len : m_glob_indLengths.lengths) // Iterate through each indicator length
		{
			for(const auto& valueType : m_glob_indValueTypes.valueTypes) // Iterate through each value type (Value or Slore)
			{
				cacledPolynomMap[PolynomKey(type, len, valueType)] = BuildPolynomForIndicator(nodes, type, len, valueType); // Build polynomial for the given type, length, and value type
			}
		}
	}
	
	return cacledPolynomMap;
}

Polynom PolynomChangeNodePackageBuilder::BuildPolynomForIndicator(std::vector<ChangeNode*> nodes, IndicatorType indT, IndicatorLen indL, IndicatorValueType indVT, BarPeriod indP)
{
	GraphStructure graph = GraphUtils::Build_NormalizedGraph(nodes, indT, indL, indP, indVT);
	Polynom polynom = PolynomBuilder::BuildAPolynom(graph);
	return polynom;
}

