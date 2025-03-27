#include "BTables.h"
#include "DefinedTables.h"
#include "ChangeData.h"
#include "CandleData.h"
#pragma once

#define SYNCJOBS_TABLES_COLS 6
// Table Name: SyncJobs

typedef struct SyncJobStructure
{
	std::string tableName;
	std::string symbol;
	std::string priceDateTime;
	std::string OpenPrice;
	std::string ClosePrice;
	CandleData data;
};

class SyncJobs_Table:BTables
{

public:
	SyncJobs_Table(SQLHENV henv, SQLHDBC hdbc) : BTables(henv, hdbc)
	{
	}

	SyncJobStructure* GetSyncJob()
	{
		if (!IsThereAnUpdate())
			return nullptr;
		return new SyncJobStructure(Build_PriceNode(GetFirstSelection()));

	}
	void RemoveSelectedRows() { RemoveSelection(); }
	bool SetSyncJob(SyncJobStructure& priceN)
	{
		if (!priceN.priceDateTime.length())
			return false;

		std::string sqlInsert = "Insert into " + TableName + 
			" ("+ DEF_COLUMN_Symbol +"," + DEF_COLUMN_PriceDateTime + "," + DEF_COLUMN_OpenPrice + "," + DEF_COLUMN_ClosePrice + "," + DEF_COLUMN_SyncStatus + "," + DEF_COLUMN_SyncToTable + ")"
			+"Values(\'" + priceN.symbol + "\',\'" + priceN.priceDateTime +"\',"+ priceN.OpenPrice + "," + priceN.ClosePrice + ",0,\'" + priceN.tableName + "\')";
			return InsertIntoDB(sqlInsert);
	}
	bool UpdateJob(SyncJobStructure& priceN,int status)
	{
		std::string sqlUpdate = "update " + TableName + " set " + DEF_COLUMN_SyncStatus + "=" + std::to_string(status) 
			+" where " + DEF_COLUMN_PriceDateTime +"=\'" + priceN.priceDateTime+"\' and " + DEF_COLUMN_SyncToTable + "=\'" + priceN.tableName+"\'";
		return UpdateDB(sqlUpdate);
	}
private:
	const std::string TableName = DEF_TABLE_SyncJobs;
	int roundToDecimalPlaces(double value, int decimalPlaces);
	bool IsThereAnUpdate();
	SQLSelectionNode* GetFirstSelection();
	SyncJobStructure Build_PriceNode(SQLSelectionNode* col);
	void ConvertSQLCHARToString(const SQLCHAR* sqlCharData, std::string& outputString);





};

