#include "ConfigsTable.h"

std::vector<std::string> ConfigsTable::GetConfigRow(const std::string& config)
{
	std::string selectCmd = "select * from " + std::string(DEF_TABLE_Configs) + " where " + std::string(DEF_COLUMN_ConfigName) + "=\'" + config + "\';";
	SelectFromDB(selectCmd, tableCols(), 4000);
	std::vector<std::string> selected;

	if (!GetSelectedRows()->size())
		return selected;

	selected.push_back(GetSelectedRows()->at(0)->GetColumn(1)); //label 
	selected.push_back(GetSelectedRows()->at(0)->GetColumn(2)); //value 
	selected.push_back(GetSelectedRows()->at(0)->GetColumn(3)); //explain 
	selected.push_back(GetSelectedRows()->at(0)->GetColumn(4)); //permission
	selected.push_back(GetSelectedRows()->at(0)->GetColumn(5)); //catagory
	RemoveSelection();
	return selected;
}

bool ConfigsTable::SetConfigValue(const std::string& config, const std::string& value)
{
	std::string updateCmd = "update " + std::string(DEF_TABLE_Configs) + " set " + std::string(DEF_COLUMN_ConfigValue) + " = \'" + value + "\' where "
		+ std::string(DEF_COLUMN_ConfigName) + " = \'" + config + "\';";
	return UpdateDB(updateCmd);
}

bool ConfigsTable::InsertConfigRow(const std::vector<std::string>& confRow)
{
	if (confRow.size() < tableCols())
		return false;
	std::string insertCmd = "INSERT INTO " + std::string(DEF_TABLE_Configs) + " values (\'" + confRow.at(0) + "\',\'" + confRow.at(1) + "\',\'"
		+ confRow.at(2) + "\'," + confRow.at(3) + ", 1);";
	return InsertIntoDB(insertCmd);
}