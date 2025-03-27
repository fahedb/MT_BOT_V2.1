#pragma once
#include "BTables.h"
#include "DefinedTables.h"
#include <string.h>
struct NewOrderStructure
{
	int Long_Short;
	std::string symbol;
	std::string priceDateTime;

};

class OrdersToExecute : BTables
{
	SQLHENV* m_henv;
	SQLHDBC* m_hdbc;
	std::string m_tableName = DEF_TABLE_OrdersToExecute;
	int m_rowsNumber = 4;
public:
	OrdersToExecute(SQLHENV henv, SQLHDBC hdbc) : BTables(henv, hdbc)
	{
		m_henv = &henv;
		m_hdbc = &hdbc;
	}

	NewOrderStructure GetNextOrder()
	{
		if (!FindOrder_RunQuery())
			return NewOrderStructure();
		UpdateAllOrders();
		return BuildOrderStructure(GetCurrent_Executable_Order());

	}
	bool InsertNewOrder_ForExecution(std::string priceDateTime,std::string symbol,std::string long_short)
	{
		std::string InsertCommand = "insert into " + m_tableName + " Values(\'" +
			priceDateTime + "\',\'" + symbol + "\'," + long_short + ",0);";
		return InsertIntoDB(InsertCommand);
	}

private:
	void UpdateAllOrders()
	{
		std::string updateCommand = "update " + m_tableName + " set "
			+ DEF_COLUMN_ActionSubmitted + " = " + "1 where " + DEF_COLUMN_ActionSubmitted + " = 0;";
		UpdateDB(updateCommand);
	}
	SQLSelectionNode* GetCurrent_Executable_Order()
	{
		if (GetSelectedRows()->size() == 0)
		{
			return nullptr;
		}
		return GetSelectedRows()->at(0);

	}
	NewOrderStructure BuildOrderStructure(SQLSelectionNode* selectedRow)
	{
		if (!selectedRow)
			return NewOrderStructure();;
		NewOrderStructure orderStruct;
		ConvertSQLCHARToString(selectedRow->GetRow()->at(0).data(), orderStruct.priceDateTime);
		ConvertSQLCHARToString(selectedRow->GetRow()->at(1).data(), orderStruct.symbol);
		std::string long_short;
		ConvertSQLCHARToString(selectedRow->GetRow()->at(2).data(), long_short);
		orderStruct.Long_Short = std::stoi(long_short);

		return orderStruct;
	}
	void ConvertSQLCHARToString(const SQLCHAR* sqlCharData, std::string& outputString) {
		// Convert SQLCHAR* to std::string
		outputString = reinterpret_cast<const char*>(sqlCharData);
	}
	bool FindOrder_RunQuery()
	{
		std::string query = "select top 1 * from " + m_tableName + " (NOLOCK) where " +
			DEF_COLUMN_ActionSubmitted + " = 0 order by " + DEF_COLUMN_PriceDateTime + " desc;";

		return FreeStyleSelectFromDB(query, 4);
	}
};

