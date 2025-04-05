#pragma once
#include <string>
#include "ConfigsDefs.h"
#include <fstream>
#include <vector>
#include "DataBaseConnection.h"

struct EMA_Check_Details;

struct InfraConfigrationsPolicy {
    std::string http_server_port;
};

struct ConfigrationsPolicy {

    double Pattern_upperGravity_Check = 0.0;
    double Pattern_lowerGravity_Check = 0.0;

    double Pattern_Gravity_Range_Check = 0.0;

    double Score_Min_TotalScore_ToAct_PerCandle = 0;
    int Score_Future_Vision_Candles = 0;


    double Patter_Min_Patterns_ToTake_Action = 0.0;
    double Patter_Max_Patterns_To_Find = 0.0;
    double Pattern_TopScore_ThresholdRatio = 0.0;

    double Pattern_Min_Percent_To_ExecuteOrder = 0.0;
    
    int Pattern_Activate_Alternative_ExecuteOrder = 0;
    double Pattern_Diff_Percent_To_ExecuteOrder_Alternative = 0.0;
    double Pattern_Max_Noise_To_ExecuteOrder_Alternative = 0.0;

    double Min_MeanPips_ToExecute_Order = 0.0;
    double Pattern_Min_PipsChange_ToCount = 0.0;
    int Pattern_CurrentTimeLineLen = 0;
    int Max_Orders_InRow = 0;
    int Sleep_Minuts_after_Max_Orders_InRow = 0;
    int EMA_len_for_Pattern_Export = 0;
    int RSI_Wieght = 0;
    int EMA_Wieght = 0;
    int ZLSMA_Wieght = 0;
    int SL_start = 0;
    int TP_start = 0;

    int TP_diff = 0;
    int SL_diff = 0;

    int TimeBetweenScans = 0;

    bool Export_Patterns_PNG_To_Telegram = false;
};

class Configrations // SingleTon class for all configrations of the program
{
public:
    // Deleted copy constructor and assignment operator to prevent copies
    Configrations(const Configrations&) = delete;
    Configrations& operator=(const Configrations&) = delete;
    DataBaseConnection* m_db_conn;
    // Static method to get the instance of the Singleton
    static Configrations* getInstance() {
        static Configrations instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }

    std::string Get_ConnectionString() { return m_connString; }
    std::string Get_DB_FileDir() { return m_filesDBDir; }
    std::string Get_ConfigValue(std::string conf) { ReadConfigValue(conf); }
    std::string Get_ConfigValue(std::string conf, std::string defualt) { return ReadConfigValue(conf).length() ? ReadConfigValue(conf) : defualt; }
    std::string Get_ConfigValue_DB(std::string conf, std::string defualt);
    bool UpdateConfigValue_DB(std::string conf,std::string value);
    std::vector<std::vector<std::string>> Get_AllConfigRows_DB();
    std::vector<std::string> ReadConfigRow_DB(std::string conf,std::string defualt);
    
    ConfigrationsPolicy* Get_Policy() { return &m_policy; }
    InfraConfigrationsPolicy* Get_InfraPolicy() { return &m_infraPolicy; }
    void RefreshPolicy() { Init_PatternPolicy(); }

private:
    std::string m_connString;
    std::string m_filesDBDir;
   // ConfigsTable* m_confTable_DB;

    // Private constructor to prevent instantiation
    Configrations() {
        //DataBaseConnection m_db_con;
        m_connString = ReadConfigValue(CONFIG_DEF_CONNECTION_STRING);
        m_filesDBDir = ReadConfigValue(CONFIG_DEF_FILES_DB_DIR);
        //m_confTable_DB = new ConfigsTable(m_db_con.Get_EnvHandler(), m_db_con.Get_DBConnection());
        m_db_conn = new DataBaseConnection(m_connString);
        Init_PatternPolicy();
        Init_InfraPolicy();
    }
    ~Configrations()
    {
        m_db_conn->CloseConnection();
        delete m_db_conn;
    }
    void Init_InfraPolicy() {
        m_infraPolicy.http_server_port = Get_ConfigValue(CONFIG_HTTP_PORT);
    }

    void Init_PatternPolicy();
    std::string ReadConfigValue(std::string config);
    std::string extractTextBetweenSymbols(const std::string& line);
    std::string DefualtConfigValues(std::string config);

    // Members
    ConfigrationsPolicy m_policy;
    InfraConfigrationsPolicy m_infraPolicy;
};
