
#include "BTables.h"
#include "BasePricesTable.h"
#include <iostream>
#include <string>
#include "ChangeData.h"
#include "Configrations.h"

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
	XAUUSD_CurrentTable(std::string& dir) : BasePricesTable(dir, DEF_TABLE_XAUUSD_CurrentPrices)
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
	virtual std::string GetDB_PhysicalFileName() override { return	Configrations::getInstance()->Get_ConfigValue(DBFILE_XAUUSD_CURR_DATA); }
	virtual int tableCols() {
		return XAUUSDCurrPricesTablesCols;
	}
private:
	virtual int GetColsNum() override { return DEF_COLUMN_NUM_XAUUSD_CURR; }
};

