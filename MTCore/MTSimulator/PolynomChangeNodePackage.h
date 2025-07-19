#include <vector>
#include "ChangeData.h"
#include "PolynomChangeNodePackageBuilder.h"
#include "CONFIGRATIONS.H"
#include "PolynomKey.h"


#pragma once
class PolynomChangeNodePackage
{
public:
	PolynomChangeNodePackage(std::vector<ChangeNode*>& nodes)
	{
		m_map = PolynomChangeNodePackageBuilder::BuildPolynomForChangeNodes(nodes);
	}
	PolynomMap m_map;
private:
	
};

