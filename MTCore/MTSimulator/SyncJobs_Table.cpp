#include "SyncJobs_Table.h"


SQLSelectionNode* SyncJobs_Table::GetFirstSelection()
{
	if (!GetSelectedRows()->size())
		return nullptr;
	return GetSelectedRows()->at(0);
}

bool SyncJobs_Table::IsThereAnUpdate()
{
	std::string sqlCommand = "select * from " + TableName + " (NOLOCK) where SyncStatus=0 order by PriceDateTime asc";
	FreeStyleSelectFromDB(sqlCommand, SYNCJOBS_TABLES_COLS);
	if (GetSelectedRows()->size())
		return true;
	return false;
}

SyncJobStructure SyncJobs_Table::Build_PriceNode(SQLSelectionNode* col)
{
	SyncJobStructure pN;

	std::string symbol;
	ConvertSQLCHARToString(col->GetRow()->at(0).data(), symbol);
	pN.symbol = symbol;

	std::string pointName;
	ConvertSQLCHARToString(col->GetRow()->at(1).data(), pointName);
	pN.priceDateTime = pointName;

	std::string openPrice;
	ConvertSQLCHARToString(col->GetRow()->at(2).data(), openPrice);
	pN.OpenPrice = openPrice;

	std::string closePrice;
	ConvertSQLCHARToString(col->GetRow()->at(3).data(), closePrice);
	pN.ClosePrice = closePrice;

	std::string tableName;
	ConvertSQLCHARToString(col->GetRow()->at(5).data(), tableName);
	pN.tableName = tableName;

	return pN;
}

int SyncJobs_Table::roundToDecimalPlaces(double value, int decimalPlaces) {
	double factor = std::pow(10.0, decimalPlaces);
	int retVal = static_cast<int>(value) * static_cast<int>(factor);
	retVal = static_cast<int>(std::round(retVal));
	return retVal;
}

void SyncJobs_Table::ConvertSQLCHARToString(const SQLCHAR* sqlCharData, std::string& outputString) { outputString = reinterpret_cast<const char*>(sqlCharData); }
