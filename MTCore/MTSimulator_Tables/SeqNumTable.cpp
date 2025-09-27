#include "SeqNumTable.h"
#include "Converts.h"


std::string SeqNumTable::GetEntityType(EntityType entT)
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

bool SeqNumTable::UpdateSeqNum(EntityType entT, int seq)
{
	std::string cmmd = "Update " + GetTableName() + " set " + DEF_COLUMN_SEQ + " = " + std::to_string(seq) + " where " + DEF_COLUMN_ENTITY + " =\'" + GetEntityType(entT) + "\'";
	return UpdateDB(cmmd);
}

SQLSelectionNode* SeqNumTable::Get_SQLSelectionNode(std::string selectQ) {
	SelectFromDB(selectQ, GetColsNum());
	return GetSelectedRows()->size() ? GetSelectedRows()->at(0) : nullptr;
}

int SeqNumTable::GetCurrSeqNum(EntityType entT)
{
	std::string ent = GetEntityType(entT);

	std::string cmmd = "Select * from " + GetTableName() + " where " + DEF_COLUMN_ENTITY + " = \'" + ent + "\'";
	SQLSelectionNode* selection = Get_SQLSelectionNode(cmmd);
	int seqnum = Converts::ToInt(selection->GetColumn(1));
	RemoveSelection();
	return seqnum;

}

int SeqNumTable::GetNextSeqNum(EntityType entT)
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