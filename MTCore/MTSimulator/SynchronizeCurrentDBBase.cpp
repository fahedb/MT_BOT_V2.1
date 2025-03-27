#include "SynchronizeCurrentDBBase.h"
#include <fstream>
#include "PROCESSDATA.H"
bool SynchronizeCurrentDBBase::SyncCurrentTables(BasePricesTable* tableObj)
{
	SyncJobStructure* priceN = syncJobsPtr->GetSyncJob();
	if (!tableObj)
		return false;
	m_currentTable = tableObj;

	if (priceN)
	{
		RoundJobData(priceN);
		std::string sqlUpdateCommand = build_CurrenTables_SqlUpdateCommand(priceN, Find_Oldest_Row_InCurrentTables(priceN->tableName)); // need to replace test /////
		m_currentTable->FreeStyleUpdateCommand(sqlUpdateCommand);
		syncJobsPtr->UpdateJob(*priceN, 1);
		syncJobsPtr->RemoveSelectedRows();
		m_currentTable = nullptr;
		return true;
	}
	m_currentTable = nullptr;
	return false;
}


bool SynchronizeCurrentDBBase::SyncCurrentTables(BasePricesTable* tableObj, SyncJobStructure* priceN)
{
	if (!tableObj)
		return false;
	m_currentTable = tableObj;
	if (priceN)
	{
		//RoundJobData(priceN);
		std::string sqlUpdateCommand = build_CurrenTables_SqlUpdateCommand(priceN, Find_Oldest_Row_InCurrentTables(tableObj->Get_TableName())); // need to replace test /////
		bool res = m_currentTable->FreeStyleUpdateCommand(sqlUpdateCommand);
		m_currentTable = nullptr;
		return res;
	}
	m_currentTable = nullptr;
	return false;
}


bool SynchronizeCurrentDBBase::InsertionIntoHistoryTables(BasePricesTable* tableObj)
{
	SyncJobStructure* priceN = syncJobsPtr->GetSyncJob();
	if (!tableObj)
		return false;
	m_currentTable = tableObj;

	if (priceN)
	{
		RoundJobData(priceN);
		m_currentTable->StartInsertCommand();
		m_currentTable->InitValue_PriceDateTime(priceN->priceDateTime);
		m_currentTable->InitValue_SellPrice(std::stof(priceN->ClosePrice));
		m_currentTable->InitValue_BuyPrice(std::stof(priceN->OpenPrice));
		m_currentTable->InitValue_ActionSubmitted(1);
		m_currentTable->RunInsert();
		syncJobsPtr->UpdateJob(*priceN, 1);
		syncJobsPtr->RemoveSelectedRows();
		m_currentTable = nullptr;
		return true;
	}
	m_currentTable = nullptr;
	return false;
}

bool SynchronizeCurrentDBBase::InsertionIntoTables(BasePricesTable* tableObj, SyncJobStructure* priceN)
{
	if (!tableObj)
		return false;
	m_currentTable = tableObj;
	std::ofstream file("C:\\MTDB\\log.txt", std::ios::app);
	if (priceN)
	{
		//RoundJobData(priceN);
		if (InsertPrimaryKey()) {
			std::string sqlUpdateCommand = build_CurrenTables_SqlUpdateCommand(priceN, PrimaryTimePoint); // need to replace primary row
			file << "\n" << sqlUpdateCommand;
			bool res = m_currentTable->FreeStyleUpdateCommand(sqlUpdateCommand);
			if (!res)
			{
				//delete PrimaryTimePOint
				m_currentTable->FreeStyleCommand("delete from " + m_currentTable->Get_TableName() + " where " + DEF_COLUMN_PriceDateTime + " = \'" + PrimaryTimePoint + "\'");
				file << "\nres = false, deleting primaryDateTime\n";
			}
			m_currentTable = nullptr;
			return res;
		}
		return false;
	}
	m_currentTable = nullptr;
	return false;
}

void SynchronizeCurrentDBBase::FeedDataToCurrentTablesJobs(std::vector<std::string>& record, std::string symbol)
{

	SyncJobStructure job;
	job.priceDateTime = record.at(0);
	job.OpenPrice = record.at(1);
	job.ClosePrice = record.at(4);
	job.symbol = symbol;
	job.tableName = m_currentTable->Get_TableName();
	syncJobsPtr->SetSyncJob(job);
}

void SynchronizeCurrentDBBase::RoundJobData(SyncJobStructure* job)
{
	float highV = std::stof(job->OpenPrice);
	float lowV = std::stof(job->ClosePrice);

	job->OpenPrice = roundToString(highV, 5);
	job->ClosePrice = roundToString(lowV, 5);
}

  std::string SynchronizeCurrentDBBase::roundToString(float value, int decimalPlaces) {
	 std::ostringstream oss;
	 oss << std::fixed << std::setprecision(decimalPlaces) << value;
	 return oss.str();

 }

   std::string SynchronizeCurrentDBBase::Find_Oldest_Row_InCurrentTables(const std::string& tableName)
  {
	  std::string sqlQuery = "select top 1 * from " + tableName + " (NOLOCK) order by " + DEF_COLUMN_PriceDateTime + " asc";
	  m_currentTable->StartQueryCommand();
	  m_currentTable->FreeStyleQuery(sqlQuery);
	  SQLSelectionNode* selected = m_currentTable->GetNextEnt();
	  std::string timePoint;
	  ConvertSQLCHARToString(selected->GetRow()->at(0).data(), timePoint);
	  m_currentTable->RemoveSelect();
	  return timePoint;
  }

    std::string SynchronizeCurrentDBBase::build_CurrenTables_SqlUpdateCommand(SyncJobStructure* priceN, std::string oldPoint) // OLD LOGIC !!
	{
		std::string command;/*= "update " + m_currentTable->Get_TableName()
			+ " set " + DEF_COLUMN_PriceDateTime + "=\'" + priceN->priceDateTime + "\' ,"
			+ DEF_COLUMN_OpenPrice + "= \'" + std::string(priceN->data.openPrice,7) + "\' ," + DEF_COLUMN_ClosePrice + "=\'" + std::string(priceN->data.closePrice,7)+ "\' ,"
			+ DEF_COLUMN_HighPrice + "= \'" + std::string(priceN->data.lowPrice ,7)+ +"\' , " + DEF_COLUMN_LowPrice + " = \'" + std::string(priceN->data.lowPrice ,7)+ "\',"

			+ DEF_COLUMN_EMAM5 + " = \'" +		( std::string(priceN->data.ema_M5,7)		)+ "\' , "
			 + DEF_COLUMN_EMAM15 + "= \'" +		(std::string(priceN->data.ema_M15, 7)) + "\' , "
			+ DEF_COLUMN_EMAM30 + "= \'" +		( std::string(priceN->data.ema_M30,7)	)+ "\' , "
			+ DEF_COLUMN_EMAH1 + "= \'" +		( std::string(priceN->data.ema_H1,7)		)+ "\' , "
			+ DEF_COLUMN_EMAH4 + "= \'" +		( std::string(priceN->data.ema_H4,7)		)+ "\' , "
			+ DEF_COLUMN_EMAM1 + "= \'" +		( std::string(priceN->data.ema_M1,7)		)+ "\' , "

		   + DEF_COLUMN_EMAM1_SLOPE + "= \'" +	(std::string(priceN->data.ema_slope_M1	,7)) + "\' , "
		   + DEF_COLUMN_EMAM5_SLOPE + "= \'" +	(std::string(priceN->data.ema_slope_M5	,7)) + "\' , "
		   + DEF_COLUMN_EMAM15_SLOPE + "= \'" + (std::string(priceN->data.ema_slope_M15	,7)) + "\' , "
		   + DEF_COLUMN_EMAM30_SLOPE + "= \'" + (std::string(priceN->data.ema_slope_M30	,7)) + "\' , "
		   + DEF_COLUMN_EMAH4_SLOPE + "= \'" +	(std::string(priceN->data.ema_slope_H4	,7)) + "\' , "
		   + DEF_COLUMN_EMAH1_SLOPE + "= \'" +	(std::string(priceN->data.ema_slope_H1	,7)) + "\' , "


		   + DEF_COLUMN_RSIM1 + "= \'" +		(std::string(priceN->data.rsi_M1		,7)		)+ "\' , "
		   + DEF_COLUMN_RSIM5 + "= \'" +		(std::string(priceN->data.rsi_M5		,7)		)+	"\' , "
		   + DEF_COLUMN_RSIM15 + "= \'" +		(std::string(priceN->data.rsi_M15	,7)	)+ "\' , "
		   + DEF_COLUMN_RSIM30 + "= \'" +		(std::string(priceN->data.rsi_M30	,7)	)+ "\' , "
		   + DEF_COLUMN_RSIH1 + "= \'" +		(std::string(priceN->data.rsi_H1		,7)		)+ "\' , "
		   + DEF_COLUMN_RSIH4 + "= \'" +		(std::string(priceN->data.rsi_H4		,7)		)+ "\' , "

		   + DEF_COLUMN_RSIM1_SLOPE + "= \'" +	(std::string(priceN->data.rsi_slope_M1	,7) )+ "\' , "
		   + DEF_COLUMN_RSIM5_SLOPE + "= \'" +	(std::string(priceN->data.rsi_slope_M5	,7) )+ "\' , "
		   + DEF_COLUMN_RSIM15_SLOPE + "= \'" +	( std::string(priceN->data.rsi_slope_M15	,7) )+ "\' , "
		   + DEF_COLUMN_RSIM30_SLOPE + "= \'" +	( std::string(priceN->data.rsi_slope_M30	,7) )+ "\' , "
		   + DEF_COLUMN_RSIH1_SLOPE + "= \'" +	(std::string(priceN->data.rsi_slope_H1	,7) )+ "\' , "
		   + DEF_COLUMN_RSIH4_SLOPE + "= \'" +	(std::string(priceN->data.rsi_slope_H4	,7) )+ "\' , "


		   + DEF_COLUMN_ZLSMAM1 + "= \'" +		ProcessData::TrimSpaces(std::string(priceN->data.zlsma_M1	,7) )+ "\' , "
		   + DEF_COLUMN_ZLSMAM5 + "= \'" +		ProcessData::TrimSpaces(std::string(priceN->data.zlsma_M5	,7) )+ "\' , "
		   + DEF_COLUMN_ZLSMAM15 + "= \'" +		ProcessData::TrimSpaces( std::string(priceN->data.zlsma_M15  ,7) )+ "\' , "
		   + DEF_COLUMN_ZLSMAM30 + "= \'" +		ProcessData::TrimSpaces( std::string(priceN->data.zlsma_M30	,7) )+ "\' , "
		   + DEF_COLUMN_ZLSMAH1 + "= \'" +		ProcessData::TrimSpaces(std::string(priceN->data.zlsma_H1	,7) )+ "\' , "
		   + DEF_COLUMN_ZLSMAH4 + "= \'" +		ProcessData::TrimSpaces(std::string(priceN->data.zlsma_H4	,7) )+ "\' , "

		   + DEF_COLUMN_ZLSMAM1_SLOPE + "= \'" + ProcessData::TrimSpaces(std::string(priceN->data.zlsma_slope_M1  ,7))+ "\' , "
		   + DEF_COLUMN_ZLSMAM5_SLOPE + "= \'" + ProcessData::TrimSpaces(std::string(priceN->data.zlsma_slope_M5  ,7))+ "\' , "
		   + DEF_COLUMN_ZLSMAM15_SLOPE + "= \'" +ProcessData::TrimSpaces(std::string( priceN->data.zlsma_slope_M15,7)) + "\' , "
		   + DEF_COLUMN_ZLSMAM30_SLOPE + "= \'" +ProcessData::TrimSpaces(std::string( priceN->data.zlsma_slope_M30,7)) + "\' , "
		   + DEF_COLUMN_ZLSMAH1_SLOPE + "= \'" + ProcessData::TrimSpaces(std::string(priceN->data.zlsma_slope_H1  ,7))+ "\' , "
		   + DEF_COLUMN_ZLSMAH4_SLOPE + "= \'" + ProcessData::TrimSpaces(std::string(priceN->data.zlsma_slope_H4  ,7))+ "\' , "

		   + DEF_COLUMN_Gravity_Total + " = " +	 std::to_string( priceN->data.totalTicks ) + " , "
		   + DEF_COLUMN_Gravity_Upper + " = " +	 std::to_string(priceN->data.upperTicks )+ " , "
		   + DEF_COLUMN_Gravity_Lower + " = " +	 std::to_string(priceN->data.lowerTicks )

		   + " where " + DEF_COLUMN_PriceDateTime + "=\'" + oldPoint + "\';";*/
	   return command;
   }

	 bool SynchronizeCurrentDBBase::SetTableObj(std::string tableName)
	{
		if (m_currentTable)
			delete m_currentTable;
		m_currentTable = nullptr;

		if (tableName == DEF_TABLE_EURUSD_OneMinTimeLinePrices)
		{
			//m_currentTable = new EURUSD_OneMinTable(*m_henv, *m_hdbc);
			return true;
		}
		if (tableName == DEF_TABLE_EURUSD_CurrentPrices)
		{
			m_currentTable = new EURUSD_CurrentTable(*m_henv, *m_hdbc);
			return true;
		}
		return false;

	}
