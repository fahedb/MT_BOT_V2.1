#include "ConfigsTableStaticProxy.h"
#include "ConfigsTable.h"
#include "Static_DBConnection.h"


inline std::string ConfigsTableStaticProxy::Get_ConfigValue_DB(const std::string& config, const std::string& defualt)
{
    DataBaseConnection* db_conn = Static_DBConnection::getInstance()->GetConnection();
    ConfigsTable confTable(db_conn->Get_EnvHandler(), db_conn->Get_DBConnection());

    std::vector<std::string> row = confTable.GetConfigRow(config);
    if (row.size() > 2)
        return confTable.GetConfigRow(config).at(1);
    return defualt;
}

inline bool ConfigsTableStaticProxy::UpdateConfigValue_DB(const std::string& conf, const std::string& value)
{
    DataBaseConnection* db_conn = Static_DBConnection::getInstance()->GetConnection();
    ConfigsTable confTable(db_conn->Get_EnvHandler(), db_conn->Get_DBConnection());
    bool res = confTable.SetConfigValue(conf, value);
    return res;
}

std::vector<std::string> ConfigsTableStaticProxy::Get_ConfigRow_DB(const std::string& config, const std::string& defualt)
{
    DataBaseConnection* db_conn = Static_DBConnection::getInstance()->GetConnection();
    ConfigsTable confTable(db_conn->Get_EnvHandler(), db_conn->Get_DBConnection());

    std::vector<std::string> conf = confTable.GetConfigRow(config);
    if (conf.size())
        return conf;
    std::vector<std::string> cacheConf(confTable.tableCols());
    cacheConf[0] = config;
    cacheConf[1] = defualt;
    cacheConf[2] = "Not found In DB\nTook it from cache !!";
    cacheConf[3] = "1"; // Permission
    cacheConf[4] = "1"; // Catagory
    return cacheConf;
}

inline std::string ConfigsTableStaticProxy::ReadConfigValue(const std::string& config)
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

inline std::string ConfigsTableStaticProxy::extractTextBetweenSymbols(const std::string& line) {
    size_t start = line.find(CONFIG_CONATINER_CHARACTER);
    size_t end = line.find(CONFIG_CONATINER_CHARACTER, start + 1);

    if (start != std::string::npos && end != std::string::npos) {
        return line.substr(start + 1, end - start - 1); // Extract the text between the symbols
    }

    return ""; // Return defualt connection string if not found
}
