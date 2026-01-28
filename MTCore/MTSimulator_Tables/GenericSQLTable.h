#pragma once
#include "BTables.h"

class GenericSQLTable : BTables
{
private:
		std::string m_tableName;
public:
	GenericSQLTable(const SQLHENV& henv, const SQLHDBC& hdbc, const std::string& tableName) : BTables(henv, hdbc) {
		m_tableName = tableName;
	}
public:
	std::string GetDB_PhysicalFileName() override { return m_tableName; }
	std::string GetDB_PhysicalFileName_Json() override { return m_tableName; }

	void InsertRecord(const std::string& insertSQL) { InsertIntoDB(insertSQL); }
	void SelectRecords(const std::string& selectSQL, int cols, int colLen = 24) { SelectFromDB(selectSQL, cols, colLen); }
	void UpdateRecords(const std::string& updateSQL) { UpdateDB(updateSQL); }

};

