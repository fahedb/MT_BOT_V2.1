
#include "BTables.h"
#include "BasePricesTable.h"
#include <iostream>
#include <string>
#include "ChangeData.h"

#define CURR_XAUUSD_DB_FILE "XAUUSD_M1_DATA.json"

#define XAUUSDCurrPricesTablesCols 44
#pragma once
class XAUUSD_CurrentTable : public BasePricesTable
{

private:
	int index = 0;
public:
	XAUUSD_CurrentTable(SQLHENV henv, SQLHDBC hdbc) :  BasePricesTable(henv, hdbc, DEF_TABLE_XAUUSD_CurrentPrices)
	{
		ClearAll();
		index = 0;
	}
	XAUUSD_CurrentTable(std::string& dir, std::vector<ChangeNode>* m_changeNodesPool, IndicatorStorage* indStorage) : BasePricesTable(dir, DEF_TABLE_XAUUSD_CurrentPrices, m_changeNodesPool, indStorage)
	{
		ClearAll();
		index = 0;
		std::string fileName = dir + GetDB_PhysicalFileName();
		Set_DATA_DB_FileName(fileName);
	}
	XAUUSD_CurrentTable() : BasePricesTable(DEF_TABLE_XAUUSD_CurrentPrices)
	{
		ClearAll();
		index = 0;
	}
	_Points GetPointPlace() { return _Points::Points_XAUUSD; }
	std::string Get_Symbol() override { return "XAUUSD"; }
	virtual std::string GetDB_PhysicalFileName() override { return	CURR_XAUUSD_DB_FILE; }
	virtual int tableCols() {
		return XAUUSDCurrPricesTablesCols;
	}
private:
	virtual int GetColsNum() override { return DEF_COLUMN_NUM_XAUUSD_CURR; }
};

