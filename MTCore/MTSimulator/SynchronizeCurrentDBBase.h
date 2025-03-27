#pragma once
#include "SyncJobs_table.h"
#include "BasePricesTable.h"
#include "EURUSD_CurrentTable.h"
#include <sstream>   
#include <iomanip>

#define PrimaryTimePoint "99999999"
class SynchronizeCurrentDBBase
{
public:
	SyncJobs_Table* syncJobsPtr;
	BasePricesTable* m_currentTable;
	SQLHENV* m_henv;
	SQLHDBC* m_hdbc;
	SynchronizeCurrentDBBase(SQLHENV henv, SQLHDBC hdbc)
	{
		m_currentTable = nullptr;
		m_henv = &henv;
		m_hdbc = &hdbc;
		syncJobsPtr = new SyncJobs_Table(henv, hdbc);
	}

	bool SyncCurrentTables(BasePricesTable* tableObj);
	bool SyncCurrentTables(BasePricesTable* tableObj, SyncJobStructure* priceN);
	bool InsertionIntoHistoryTables(BasePricesTable* tableObj);
	bool InsertionIntoTables(BasePricesTable* tableObj, SyncJobStructure* priceN);
	void FeedDataToCurrentTablesJobs(std::vector<std::string>& record,std::string symbol);
	bool FeedDataToCurrentTablesJobs(SyncJobStructure job) { return syncJobsPtr->SetSyncJob(job); }
	bool InsertPrimaryKey() {
		std::string command = "Insert into " + m_currentTable->Get_TableName() + " (" + DEF_COLUMN_PriceDateTime + ") values (\'" + PrimaryTimePoint + "\')";
		return  m_currentTable->FreeStyleCommand(command);
	}
private:
	void RoundJobData(SyncJobStructure* job);
	std::string roundToString(float value, int decimalPlaces);
	std::string Find_Oldest_Row_InCurrentTables(const std::string& tableName);
	void ConvertSQLCHARToString(const SQLCHAR* sqlCharData, std::string& outputString) { outputString = reinterpret_cast<const char*>(sqlCharData); } // Convert SQLCHAR* to std::string
	std::string build_CurrenTables_SqlUpdateCommand(SyncJobStructure* priceN, std::string oldPoint);
	bool SetTableObj(std::string tableName);

	
};

