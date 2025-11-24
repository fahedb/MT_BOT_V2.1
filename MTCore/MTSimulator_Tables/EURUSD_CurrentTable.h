#include "BasePricesTable.h"
#include <iostream>
#include <string>

#pragma once
class EURUSD_CurrentTable :public BasePricesTable
{
private:
	int index = 0;
public:
	EURUSD_CurrentTable(SQLHENV henv, SQLHDBC hdbc) : BasePricesTable(henv, hdbc, "EURUSD_CurrentPrices")
	{
		ClearAll();
		index = 0;
	}
	_Points GetPointPlace() { return _Points::Points_EURUSD; }
	std::string Get_Symbol() override { return "EURUSD"; }

};