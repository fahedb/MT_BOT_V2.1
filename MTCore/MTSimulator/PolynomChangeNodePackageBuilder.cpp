#include "PolynomChangeNodePackageBuilder.h"
#include "PolynomTimeLine.h"

PolynomMap PolynomChangeNodePackageBuilder::BuildPolynomForChangeNodes(std::vector<ChangeNode*> nodes)
{
	PolynomMap polynomMap; // Map to hold polynomials with keys as PolynomKey (IndicatorType, IndicatorLen, IndicatorValueType)

	IndicatorTypeList indTypes; // List of indicator types
	IndicatorLenList indLengths; // List of indicator lengths
	IndicatorValueTypeList indValueTypes; // List of indicator value types (Value or Slope)

	// Build polynomials for different indicators and lengths
	for (const auto& type : indTypes.types) // Iterate through each indicator type
	{
		for(const auto& len : indLengths.lengths) // Iterate through each indicator length
		{
			for(const auto& valueType : indValueTypes.valueTypes) // Iterate through each value type (Value or Slore)
			{
				polynomMap[PolynomKey(type, len, valueType)] = BuildPolynomForIndicator(nodes, type, len, valueType); // Build polynomial for the given type, length, and value type
			}
		}
	}

	return polynomMap;
}
