#include <vector>
#include "ChangeData.h"
#include "PolynomChangeNodePackageBuilder.h"
#include "CONFIGRATIONS.H"

#pragma once
class PolynomChangeNodePackage
{
public:
	PolynomChangeNodePackage(std::vector<ChangeNode*>& nodes)
	{
		

	}

private:
	std::vector<Polynom> m_pols; // Map to store polynomials by symbol

	void BuildPolynomForChangeNodes(std::vector<ChangeNode*> chNs)
	{
		int polynomLen = Configrations::getInstance()->Get_Policy()->PolynomLen_ForScan;

		for()
	}

};

