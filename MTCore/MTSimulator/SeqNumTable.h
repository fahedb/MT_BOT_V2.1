#pragma once
#include "BTables.h"
#include "DefinedTables.h"
#include "Converts.h"
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

	int GetNextSeqNum(EntityType entT)
	{
		std::string ent = GetEntityType(entT);

		std::string cmmd = "Select * from " + GetTableName() + " where " + DEF_COLUMN_ENTITY + " = \'" + ent + "\'";
		SQLSelectionNode* selection = Get_SQLSelectionNode(cmmd);
		int seqnum = Converts::ToInt(selection->GetColumn(1));
		int newSeqNum = seqnum + 1;
		RemoveSelection();
		UpdateSeqNum(entT, newSeqNum);
		return newSeqNum;

	}
	int GetCurrSeqNum(EntityType entT)
	{
		std::string ent = GetEntityType(entT);

		std::string cmmd = "Select * from " + GetTableName() + " where " + DEF_COLUMN_ENTITY + " = \'" + ent + "\'";
		SQLSelectionNode* selection = Get_SQLSelectionNode(cmmd);
		int seqnum = Converts::ToInt(selection->GetColumn(1));
		RemoveSelection();
		return seqnum;

	}

private:
	std::string GetTableName() { return m_tableName; }

	SQLSelectionNode* Get_SQLSelectionNode(std::string selectQ) { 
		SelectFromDB(selectQ, GetColsNum());
		return GetSelectedRows()->size() ? GetSelectedRows()->at(0) : nullptr ;
	}

	bool UpdateSeqNum(EntityType entT, int seq)
	{
		std::string cmmd = "Update " + GetTableName() + " set " + DEF_COLUMN_SEQ + " = " + std::to_string(seq) + " where " + DEF_COLUMN_ENTITY + " =\'" + GetEntityType(entT) + "\'";
		return UpdateDB(cmmd);
	}

	std::string GetEntityType(EntityType entT)
	{
		std::string ent;
		switch (entT)
		{
		case EntityType::Order:
			ent = "orders";
			break;
		default:
			ent = "none";
			break;
		}
		return ent;
	}
	int GetColsNum() { return SeqNumTablesCols; }
};

