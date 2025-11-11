#pragma once
#include "BTables.h"
#include "DefinedTables.h"

#define ConfigsTablesCols 6

class ConfigsTable : BTables
{
public:
	ConfigsTable(SQLHENV henv,SQLHDBC hdbc) : BTables(henv, hdbc)
	{

	}
	std::vector<std::string> GetConfigRow(const std::string& config);
	bool SetConfigValue(const std::string& config,const std::string& value);
	bool InsertConfigRow(const std::vector<std::string>& confRow);

	virtual std::string GetDB_PhysicalFileName() override { return "Configs"; }
	virtual std::string GetDB_PhysicalFileName_Json() override { return ""; }

	virtual int tableCols() { return ConfigsTablesCols; }

private:

};

