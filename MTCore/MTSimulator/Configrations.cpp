#include "Configrations.h"
#include "ConfigsTable.h"
#include "Converts.h"

std::string Configrations::DefualtConfigValues(std::string& config)
{
    if (config == CONFIG_DEF_FILES_DB_DIR)
        return DEFUALT_FILES_DB_DIR;
    if (config == CONFIG_DEF_CONNECTION_STRING)
        return DEFUALT_CONN_STRING;

    return "";
}
std::string Configrations::extractTextBetweenSymbols(const std::string& line) {
    size_t start = line.find(CONFIG_CONATINER_CHARACTER);
    size_t end = line.find(CONFIG_CONATINER_CHARACTER, start + 1);

    if (start != std::string::npos && end != std::string::npos) {
        return line.substr(start + 1, end - start - 1); // Extract the text between the symbols
    }

    return ""; // Return defualt connection string if not found
}
std::string Configrations::Get_ConfigValue_DB(const std::string& config,const std::string& defualt)
{
    ConfigsTable confTable(m_db_conn->Get_EnvHandler(), m_db_conn->Get_DBConnection());
    std::vector<std::string> row = confTable.GetConfigRow(config);
    if (row.size() > 2)
        return confTable.GetConfigRow(config).at(1);
    return defualt;
}

bool Configrations::UpdateConfigValue_DB(std::string& conf,std::string& value)
{
    ConfigsTable confTable(m_db_conn->Get_EnvHandler(), m_db_conn->Get_DBConnection());
    bool res = confTable.SetConfigValue(conf, value);
    return res;
}

std::vector<std::vector<std::string>> Configrations::Get_AllConfigRows_DB()
{
    std::vector<std::vector<std::string>> allConfigs;
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_CURR_TIME_LINE_LEN));    
    
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_PATTERN_MINIMUM_PERCENT_TO_EXECUTE_ORDER));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_PATTERN_MIN_MEAN_PIPS_TO_EXECUTE_ORDER));
    allConfigs.push_back(ReadConfigRow_DB(CONFG_DEF_PATTERN_MAX_PATTERNS_TO_FIND));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_PATTERN_MINIMUM_PATTERNS_TO_TAKE_ACTION));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_PATTERN_THRESHOLD_RATIO_TOP_SCORES));
    
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_PATTERN_ACTIVATE_ALTERNATIVE_EXECUTE_ORDER));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_PATTERN_MAX_NOISE_PERCENT_TO_EXECUTE_ORDER_ALTERNATIVE));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_PATTERN_DIFF_PERCENT_TO_EXECUTE_ORDER_ALTERNATIVE));

    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_PATTERN_FUTURE_VISION_CANDLES));
    
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_PATTERN_MAX_ORDERS_INROW));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_PATTERN_SLEEP_MINUTS_AFTER_MAX_ORDERS_INROW));

    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_PATTERN_MIN_TOTAL_SCORE_TO_ACT));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_PATTERN_MINIMUM_PIPS_CHANGE_TOCOUNT));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_SAVED_STATS_LEN));
    
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_MAIN_ORDER_SIZE_IN_LOT));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_SUPPORT_ORDER_SIZE_IN_LOT));
    

    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_RSI_WIEGHT));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_EMA_WIEGHT));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_ZLSMA_WIEGHT));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_VOLUME_WIEGHT));

    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_STOP_LOSS_DIFF));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_TAKE_PROFIT_DIFF));
    
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_STOP_LOSS_START));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_TAKE_PROFIT_START));
    
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_EMA_LEN_FOR_PATTERN_EXPORT));

    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_TIME_BETWEEN_SCANS));

    allConfigs.push_back(ReadConfigRow_DB(CONFG_DEF_EXPORT_PATTERNS_PNG_TO_TELEGRAM));
    allConfigs.push_back(ReadConfigRow_DB(CONFG_DEF_ENABLE_PATTERNS_PNG_EXPORT));
    allConfigs.push_back(ReadConfigRow_DB(CONFG_DEF_TRACE_CURR_TIMELINE_LEN));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_ENABLE_TRADE_TRACING));
    allConfigs.push_back(ReadConfigRow_DB(CONFIG_DEF_ENABLE_DYNAMIC_TIMELINE_TRADE_TRACING));
    return allConfigs;
}

std::vector<std::string> Configrations::ReadConfigRow_DB(std::string config,std::string defualt)
{
    ConfigsTable confTable(m_db_conn->Get_EnvHandler(), m_db_conn->Get_DBConnection());
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


std::string Configrations::ReadConfigValue(std::string config)
{
    std::ifstream confFile(PATH_TO_CONFIG);
    if (!confFile.is_open())
    {
        return DefualtConfigValues(config);
    }
    std::string line;
    while (std::getline(confFile, line)) { // Read the file line by line
        if (line.find(config) != std::string::npos) { // Check if config is found
            std::string confValue = extractTextBetweenSymbols(line);
            if (confValue.length())
                return confValue;
            else
                return DefualtConfigValues(config);
        }
    }
    confFile.close();
    return DefualtConfigValues(config);
}

void Configrations::Init_PatternPolicy()
{
    m_policy.Score_Min_TotalScore_ToAct_PerCandle =
        Converts::ToDouble(Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MIN_TOTAL_SCORE_TO_ACT));

    m_policy.Score_Future_Vision_Candles = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_PATTERN_FUTURE_VISION_CANDLES));

    m_policy.Patter_Min_Patterns_ToTake_Action =
        Converts::ToDouble(Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MINIMUM_PATTERNS_TO_TAKE_ACTION));
    m_policy.Patter_Max_Patterns_To_Find =
        Converts::ToDouble(Get_ConfigValue_DB(CONFG_DEF_PATTERN_MAX_PATTERNS_TO_FIND));
    m_policy.Pattern_Min_Percent_To_ExecuteOrder =
        Converts::ToDouble(Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MINIMUM_PERCENT_TO_EXECUTE_ORDER));
    m_policy.Pattern_TopScore_ThresholdRatio =
        Converts::ToDouble(Get_ConfigValue_DB(CONFIG_DEF_PATTERN_THRESHOLD_RATIO_TOP_SCORES));

    m_policy.Pattern_Diff_Percent_To_ExecuteOrder_Alternative =
        Converts::ToDouble(Get_ConfigValue_DB(CONFIG_DEF_PATTERN_DIFF_PERCENT_TO_EXECUTE_ORDER_ALTERNATIVE));
    m_policy.Pattern_Max_Noise_To_ExecuteOrder_Alternative =
        Converts::ToDouble(Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MAX_NOISE_PERCENT_TO_EXECUTE_ORDER_ALTERNATIVE));
    m_policy.Pattern_Activate_Alternative_ExecuteOrder =
        Converts::ToDouble(Get_ConfigValue_DB(CONFIG_DEF_PATTERN_ACTIVATE_ALTERNATIVE_EXECUTE_ORDER));

    m_policy.Pattern_Min_PipsChange_ToCount =
        Converts::ToDouble(Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MINIMUM_PIPS_CHANGE_TOCOUNT)); // Need to implement a logic
    
    m_policy.Min_MeanPips_ToExecute_Order = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MIN_MEAN_PIPS_TO_EXECUTE_ORDER));

    m_policy.Max_Orders_InRow = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MAX_ORDERS_INROW));
    m_policy.Sleep_Minuts_after_Max_Orders_InRow = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_PATTERN_SLEEP_MINUTS_AFTER_MAX_ORDERS_INROW));

    m_policy.Pattern_CurrentTimeLineLen = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_CURR_TIME_LINE_LEN));
    m_policy.SavedDirectionStaticsLen = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_SAVED_STATS_LEN));

    m_policy.EMA_len_for_Pattern_Export = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_EMA_LEN_FOR_PATTERN_EXPORT));
    m_policy.RSI_Wieght =    Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_RSI_WIEGHT));
    m_policy.EMA_Wieght =    Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_EMA_WIEGHT));
    m_policy.ZLSMA_Wieght =  Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_ZLSMA_WIEGHT));
    m_policy.VOLUME_Wieght = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_VOLUME_WIEGHT));
    
    m_policy.SL_diff = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_STOP_LOSS_DIFF));
    m_policy.TP_diff = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_TAKE_PROFIT_DIFF));

    m_policy.SL_start = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_STOP_LOSS_START));
    m_policy.TP_start = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_TAKE_PROFIT_START));

    m_policy.TimeBetweenScans = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_TIME_BETWEEN_SCANS));
    m_policy.Export_Patterns_PNG_To_Telegram = Converts::ToInt(Get_ConfigValue_DB(CONFG_DEF_EXPORT_PATTERNS_PNG_TO_TELEGRAM)) > 0 ? true : false;
    m_policy.Enable_Patterns_PNG_Export = Converts::ToInt(Get_ConfigValue_DB(CONFG_DEF_ENABLE_PATTERNS_PNG_EXPORT)) > 0 ? true : false;

    m_policy.TraceCurrTimeLineLen = Converts::ToInt(Get_ConfigValue_DB(CONFG_DEF_TRACE_CURR_TIMELINE_LEN));
    m_policy.EnableTradeTracing =   Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_ENABLE_TRADE_TRACING)) > 0 ? true : false;
    m_policy.EnableDynamicTimeLine_TradeTracing = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_ENABLE_DYNAMIC_TIMELINE_TRADE_TRACING)) > 0 ? true : false;

    m_policy.EnableScoreCaching= Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_ENABLE_DYNAMIC_TIMELINE_TRADE_TRACING)) > 0 ? true : false;


	// Configs to control polynom scan
	m_policy.EnablePolynomScan = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_ENABLE_POLYNOM_SCAN)) > 0 ? true : false;



	//m_policy.PolynomLen_ForScan = Converts::ToInt(Get_ConfigValue_DB(CONFIG_DEF_POLYNOM_LEN_FOR_SCAN)); // NOTE: Moved to PolynomPolicy
}