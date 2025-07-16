#include "PolynomBuilder.h"
#include "ChangeData.h"
#include "robin_hood.h"
#include "GraphUtils.h"
#include "PolynomKey.h"	


#pragma once

class PolynomChangeNodePackageBuilder
{
	static robin_hood::unordered_map<PolynomrKey, Polynom> BuildPolynomForChangeNodes(std::vector<ChangeNode*> nodes);

private:
	static Polynom BuildPolynomForIndicator(std::vector<ChangeNode*> nodes,IndicatorType indT,IndicatorLen indL,IndicatorValueType indVT,BarPeriod indP = BarPeriod::M1)
	{
		GraphStructure graph = GraphUtils::Build_NormalizedGraph(nodes, indT, indL, indP,indVT);
		Polynom polynom = PolynomBuilder::BuildAPolynom(graph);
		return polynom;
	}
};

