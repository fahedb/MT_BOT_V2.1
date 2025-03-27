#pragma once
#include "BTables.h"
#include "SyncJobs_Table.h"

class DataFeeder
{
	DataFeeder(SQLHENV henv, SQLHDBC hdbc, BasePricesTable* cTB, SyncJobs_Table* syncJ)
		: m_currentTable(cTB), m_syncJobs(syncJ)
	{

	}
	BasePricesTable* m_currentTable;
	SyncJobs_Table* m_syncJobs;
public:
	void FeedDataToCurrentTables(std::vector<std::string>& record,std::string symbol)
	{
		SyncJobStructure job;
		job.priceDateTime = record.at(0);
		job.OpenPrice = record.at(1);
		job.ClosePrice = record.at(4);
		job.symbol = symbol;
		job.tableName = m_currentTable->Get_TableName();
		m_syncJobs->SetSyncJob(job);
	}


};

