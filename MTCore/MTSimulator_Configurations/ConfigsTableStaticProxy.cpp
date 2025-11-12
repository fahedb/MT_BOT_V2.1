#include "ConfigsTableStaticProxy.h"
#include "ConfigsTable.h"
#include "Static_DBConnection.h"

robin_hood::unordered_map<std::string, std::string> ConfigsTableStaticProxy::m_forcedConfigs; // Definition of the static member

std::string ConfigsTableStaticProxy::Get_ConfigValue_DB(const std::string& config, const std::string& defualt)
{
    std::string forceValue = Get_ForcedConfigValues(config);
    if (!forceValue.empty())
        return forceValue; // Return forced value if exists


    DataBaseConnection* db_conn = Static_DBConnection::getInstance()->GetConnection();
    if (!db_conn || !db_conn->IsConnected())
    {
		Logger::getInstance().WriteLog("Failed to connect to the database. Returning default value for config: " + config, true, true, LogType::ERR);
		return defualt; // Return default value if DB connection fails
	}

    ConfigsTable confTable(db_conn->Get_EnvHandler(), db_conn->Get_DBConnection());

    std::vector<std::string> row = confTable.GetConfigRow(config);
    if (row.size() > 2)
        return row.at(1);
    return defualt;
}

bool ConfigsTableStaticProxy::UpdateConfigValue_DB(const std::string& conf, const std::string& value)
{
    DataBaseConnection* db_conn = Static_DBConnection::getInstance()->GetConnection();
    if (!db_conn || !db_conn->IsConnected())
    {
        Logger::getInstance().WriteLog("Failed to connect to the database. Could not update config !!! ", true, true, LogType::ERR);
		return false; // Return false if DB connection fails
    }

    ConfigsTable confTable(db_conn->Get_EnvHandler(), db_conn->Get_DBConnection());
    bool res = confTable.SetConfigValue(conf, value);
    return res;
}

bool ConfigsTableStaticProxy::InsertConfigRow_DB(const std::vector<std::string>& confRow)
{
    if(IsConfigExists_DB(confRow.at(0))) {
        Logger::getInstance().WriteLog("Config already exists in DB: " + confRow.at(0), true, true, LogType::ERR);
        return false; // Return false if config already exists
	}

    DataBaseConnection* db_conn = Static_DBConnection::getInstance()->GetConnection();
    if (!db_conn || !db_conn->IsConnected())
    {
        Logger::getInstance().WriteLog("Failed to connect to the database. Could not insert config row !!! ", true, true, LogType::ERR);
        return false; // Return false if DB connection fails
    }
    ConfigsTable confTable(db_conn->Get_EnvHandler(), db_conn->Get_DBConnection());

	return confTable.InsertConfigRow(confRow); // Insert the config row into the database
}

bool ConfigsTableStaticProxy::IsConfigExists_DB(const std::string& conf) {
    DataBaseConnection* db_conn = Static_DBConnection::getInstance()->GetConnection();
    if (!db_conn || !db_conn->IsConnected())
        return false; // Return false if DB connection fails
    ConfigsTable confTable(db_conn->Get_EnvHandler(), db_conn->Get_DBConnection());
    auto confRow = confTable.GetConfigRow(conf);
    if (confRow.empty())
		return false; // Not found in DB
	return true; // Found in DB
}

std::vector<std::string> ConfigsTableStaticProxy::Get_ConfigRow_DB(const std::string& config, const std::string& defualt)
{
    auto createDefaultRow = [&](size_t colCount) {
        std::vector<std::string> row(colCount);
        row[0] = config;
        row[1] = defualt;
        row[2] = "Not found in DB\nTook it from cache !!";
        row[3] = "1"; // Permission
        row[4] = "1"; // Category
        return row;
        };

    DataBaseConnection* dbConn = Static_DBConnection::getInstance()->GetConnection();
    if (!dbConn || !dbConn->IsConnected()) {
        // DB connection failed — return default row
        return createDefaultRow(ConfigsTablesCols);
    }

    ConfigsTable confTable(dbConn->Get_EnvHandler(), dbConn->Get_DBConnection());
    auto conf = confTable.GetConfigRow(config);
	std::string forceValue = Get_ForcedConfigValues(config);

    if(!forceValue.empty()) {
        conf[1] = forceValue; // Override with forced value
	}

    if (!conf.empty()) {
        return conf; // Found in DB
    }
    else
    {
		Logger::getInstance().WriteLog("Config not found in DB: " + config + ". Inserting Config In DB, and return Defualt value.", true, true, LogType::ERR);
		bool res = InsertConfigRow_DB(createDefaultRow(confTable.tableCols())); // Insert default row if not found
        if(!res) {
            Logger::getInstance().WriteLog("Failed to insert default config row into DB for config: " + config, true, true, LogType::ERR);
		}
    }

    // Not found in DB — return default row
    return createDefaultRow(confTable.tableCols());
}

void ConfigsTableStaticProxy::Insert_IntoForcedConfigs_DB(const std::string& config, std::string value)
{
    m_forcedConfigs[config] = value;
}

std::string ConfigsTableStaticProxy::Get_ForcedConfigValues(const std::string& config)
{
    auto it = m_forcedConfigs.find(config);
    if (it != m_forcedConfigs.end())
        return it->second;
	return "";
}

std::string ConfigsTableStaticProxy::ReadConfigValue(const std::string& config)
{
    std::ifstream confFile(PATH_TO_CONFIG);
    if (!confFile.is_open())
    {
        return ConfigsTableStaticProxy::DefualtConfigValues(config);
    }
    std::string line;
    while (std::getline(confFile, line)) { // Read the file line by line
        if (line.find(config) != std::string::npos) { // Check if config is found
            std::string confValue = ConfigsTableStaticProxy::extractTextBetweenSymbols(line);
            if (confValue.length())
                return confValue;
            else
                return ConfigsTableStaticProxy::DefualtConfigValues(config);
        }
    }
    confFile.close();
    return ConfigsTableStaticProxy::DefualtConfigValues(config);
}

std::string ConfigsTableStaticProxy::DefualtConfigValues(const std::string& config)
{
    if (config == CONFIG_DEF_FILES_DB_DIR)
        return DEFUALT_FILES_DB_DIR;
    if (config == CONFIG_DEF_CONNECTION_STRING)
        return DEFUALT_CONN_STRING;

    return "";
}

std::string ConfigsTableStaticProxy::extractTextBetweenSymbols(const std::string& line) {
    size_t start = line.find(CONFIG_CONATINER_CHARACTER);
    size_t end = line.find(CONFIG_CONATINER_CHARACTER, start + 1);

    if (start != std::string::npos && end != std::string::npos) {
        return line.substr(start + 1, end - start - 1); // Extract the text between the symbols
    }

    return ""; // Return defualt connection string if not found
}
