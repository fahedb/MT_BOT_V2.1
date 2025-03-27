#pragma once
#include "BTables.h"
#include "DefinedTables.h"
#define ConfigsTablesCols 5

class ConfigsTable : BTables
{
public:
	ConfigsTable(SQLHENV henv,SQLHDBC hdbc) : BTables(henv, hdbc)
	{

	}
	std::vector<std::string> GetConfigRow(std::string config)
	{
		std::string selectCmd = "select * from " + std::string(DEF_TABLE_Configs) + " where " + std::string(DEF_COLUMN_ConfigName) + "=\'" + config + "\';";
		SelectFromDB(selectCmd,tableCols(),4000);
		std::vector<std::string> selected;

		if (!GetSelectedRows()->size())
			return selected;

		selected.push_back(GetSelectedRows()->at(0)->GetColumn(1)); //label 
		selected.push_back(GetSelectedRows()->at(0)->GetColumn(2)); //value 
		selected.push_back(GetSelectedRows()->at(0)->GetColumn(3)); //explain 
		selected.push_back(GetSelectedRows()->at(0)->GetColumn(4)); //permission

		return selected;
	}
	bool SetConfigValue(std::string config,std::string value)
	{
		std::string updateCmd = "update " + std::string(DEF_TABLE_Configs) + " set " + std::string(DEF_COLUMN_ConfigValue) + " = \'" + value + "\' where "
			+ std::string(DEF_COLUMN_ConfigName) + " = \'" + config + "\';";
		return UpdateDB(updateCmd);
	}
private:
	virtual int tableCols() {
		return ConfigsTablesCols;
	}

};

