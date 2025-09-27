#include "PolynomBuilder.h"
#include "ChangeData.h"
#include "PolynomKey.h"

#pragma once
typedef robin_hood::unordered_map<PolynomKey, Polynom> PolynomMap;
class PolynomChangeNodePackageBuilder
{
public:
	static PolynomMap BuildPolynomForChangeNodes(std::vector<ChangeNode*> nodes);
private:
		static Polynom BuildPolynomForIndicator(std::vector<ChangeNode*> nodes,IndicatorType indT,IndicatorLen indL,IndicatorValueType indVT,BarPeriod indP = BarPeriod::M1);
};

