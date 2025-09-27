#pragma once
#include "BTables.h"
#include "DefinedTables.h"
#include <string.h>

#define SeqNumTablesCols 2

enum class EntityType
{
	Order =0
};

class SeqNumTable : BTables
{
	SQLHENV* m_henv;
	SQLHDBC* m_hdbc;
	std::string m_tableName;
public:
	SeqNumTable(SQLHENV henv, SQLHDBC hdbc) : BTables(henv, hdbc)
	{
		m_tableName = DEF_TABLE_SEQ_NUMS;
		m_henv = &henv;
		m_hdbc = &hdbc;
	}

	int GetNextSeqNum(EntityType entT);
	int GetCurrSeqNum(EntityType entT);
	virtual std::string GetDB_PhysicalFileName() override { return DEF_TABLE_SEQ_NUMS; }
	virtual std::string GetDB_PhysicalFileName_Json() override { return ""; }

private:

	std::string GetTableName() { return m_tableName; }
	SQLSelectionNode* Get_SQLSelectionNode(std::string selectQ);
	bool UpdateSeqNum(EntityType entT, int seq);
	std::string GetEntityType(EntityType entT);
	int GetColsNum() { return SeqNumTablesCols; }
};

