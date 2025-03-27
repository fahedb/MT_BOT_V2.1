#pragma once
#include "BTables.h"
#include "BasePricesTable.h"
#include <iostream>
#include <string>
#include "ChangeData.h"
#include "Configrations.h"

class XAUUSD_OneMinTable : public BasePricesTable
{

private:
	int index = 0;
public:
	XAUUSD_OneMinTable(SQLHENV henv, SQLHDBC hdbc) : BasePricesTable(henv, hdbc, DEF_TABLE_XAUUSD_OneMinTimeLinePrices)
	{
		ClearAll();
		index = 0;
	}
	XAUUSD_OneMinTable(std::string dir) : BasePricesTable(dir, DEF_TABLE_XAUUSD_OneMinTimeLinePrices)
	{
		ClearAll();
		index = 0;
		Set_DATA_DB_FileName(dir + GetDB_PhysicalFileName());
	}
	_Points GetPointPlace() { return _Points::Points_XAUUSD; }
	virtual std::string GetDB_PhysicalFileName() {return	Configrations::getInstance()->Get_ConfigValue(DBFILE_XAUUSD_M1_DATA);}
	virtual std::string GetDB_PhysicalFileName_Json() {return	Configrations::getInstance()->Get_ConfigValue(DBFILE_XAUUSD_M1_DATA_JSON);}
	
private:
	virtual int GetColsNum() override { return DEF_COLUMN_NUM_XAUUSD_M1; }
};

