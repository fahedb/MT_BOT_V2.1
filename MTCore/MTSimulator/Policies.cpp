#include "Policies.h"
#include "Converts.h"

/////////////////////////////////////////////////////////////////////////
///////////////Policy Initialize for ConfigrationsPolicy ////////////////
/////////////////////////////////////////////////////////////////////////

std::vector<std::vector<std::string>> BasePolicy::Get_AllConfigs(std::vector<std::pair<std::string, std::string>> allConfigs)
{
    std::vector<std::vector<std::string>> result;
    for (const auto& config : allConfigs)
    {
        result.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(config.first, config.second));
    }
    return result;
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------


inline std::vector<std::vector<std::string>> ConfigrationsPolicy::Get_All()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIG_DEF_CURR_TIME_LINE_LEN),
        std::make_pair(CONFIG_DEF_EMA_LEN_FOR_PATTERN_EXPORT),
        std::make_pair(CONFIG_DEF_TIME_BETWEEN_SCANS),
        std::make_pair(CONFG_DEF_EXPORT_PATTERNS_PNG_TO_TELEGRAM),
        std::make_pair(CONFG_DEF_ENABLE_PATTERNS_PNG_EXPORT),
    };
    return Get_AllConfigs(allConfigs);
}

void ConfigrationsPolicy::InitPolicy()
{

    Pattern_CurrentTimeLineLen = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_CURR_TIME_LINE_LEN));

    EMA_len_for_Pattern_Export = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_EMA_LEN_FOR_PATTERN_EXPORT));

    TimeBetweenScans = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_TIME_BETWEEN_SCANS));
    Export_Patterns_PNG_To_Telegram = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFG_DEF_EXPORT_PATTERNS_PNG_TO_TELEGRAM)) > 0 ? true : false;
    Enable_Patterns_PNG_Export = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFG_DEF_ENABLE_PATTERNS_PNG_EXPORT)) > 0 ? true : false;
    ThreadsCount = ThreadPool::getInstance()->Get_ThreadsNum();
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

inline std::vector<std::vector<std::string>> TradeManagerPolicy::Get_All()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIG_DEF_PATTERN_MINIMUM_PATTERNS_TO_TAKE_ACTION),
        std::make_pair(CONFG_DEF_PATTERN_MAX_PATTERNS_TO_FIND),
        std::make_pair(CONFIG_DEF_PATTERN_THRESHOLD_RATIO_TOP_SCORES),
        std::make_pair(CONFIG_DEF_PATTERN_MINIMUM_PERCENT_TO_EXECUTE_ORDER),
        std::make_pair(CONFIG_DEF_PATTERN_DIFF_PERCENT_TO_EXECUTE_ORDER_ALTERNATIVE),
        std::make_pair(CONFIG_DEF_PATTERN_MAX_NOISE_PERCENT_TO_EXECUTE_ORDER_ALTERNATIVE),
        std::make_pair(CONFIG_DEF_PATTERN_ACTIVATE_ALTERNATIVE_EXECUTE_ORDER),
        std::make_pair(CONFIG_DEF_PATTERN_MINIMUM_PIPS_CHANGE_TOCOUNT),
        std::make_pair(CONFIG_DEF_PATTERN_MIN_MEAN_PIPS_TO_EXECUTE_ORDER)


    };
    return Get_AllConfigs(allConfigs);
}

void TradeManagerPolicy::InitPolicy()
{
    Pattern_Min_Patterns_ToTake_Action =
        Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MINIMUM_PATTERNS_TO_TAKE_ACTION));
    Pattern_Max_Patterns_To_Find =
        Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFG_DEF_PATTERN_MAX_PATTERNS_TO_FIND));
    Pattern_Min_Percent_To_ExecuteOrder =
        Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MINIMUM_PERCENT_TO_EXECUTE_ORDER));
    Pattern_TopScore_ThresholdRatio =
        Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_THRESHOLD_RATIO_TOP_SCORES));

    Pattern_Diff_Percent_To_ExecuteOrder_Alternative =
        Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_DIFF_PERCENT_TO_EXECUTE_ORDER_ALTERNATIVE));
    Pattern_Max_Noise_To_ExecuteOrder_Alternative =
        Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MAX_NOISE_PERCENT_TO_EXECUTE_ORDER_ALTERNATIVE));
    Pattern_Activate_Alternative_ExecuteOrder =
        Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_ACTIVATE_ALTERNATIVE_EXECUTE_ORDER));

    Pattern_Min_PipsChange_ToCount =
        Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MINIMUM_PIPS_CHANGE_TOCOUNT)); // Need to implement a logic

    Min_MeanPips_ToExecute_Order = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MIN_MEAN_PIPS_TO_EXECUTE_ORDER));

}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

inline std::vector<std::vector<std::string>> ScoreCachingPolicy::Get_All()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIG_DEF_ENABLE_SCORE_CACHEING),
        std::make_pair(CONFIG_DEF_PATTERN_MAX_ORDERS_INROW),
        std::make_pair(CONFIG_DEF_PATTERN_SLEEP_MINUTS_AFTER_MAX_ORDERS_INROW)

    };

    return Get_AllConfigs(allConfigs);
}

void ScoreCachingPolicy::InitPolicy()
{
    EnableScoreCaching = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_ENABLE_SCORE_CACHEING)) > 0 ? true : false;
    Max_Orders_InRow = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MAX_ORDERS_INROW));
    Sleep_Minuts_after_Max_Orders_InRow = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_SLEEP_MINUTS_AFTER_MAX_ORDERS_INROW));
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

inline std::vector<std::vector<std::string>> OrdersPolicy::Get_All()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIG_DEF_STOP_LOSS_START),
        std::make_pair(CONFIG_DEF_TAKE_PROFIT_START),
        std::make_pair(CONFIG_DEF_STOP_LOSS_DIFF),
        std::make_pair(CONFIG_DEF_TAKE_PROFIT_DIFF),
        std::make_pair(CONFiG_DEF_ENABLE_DYNAMIC_TP_SL)
    };

    return Get_AllConfigs(allConfigs);
}

void OrdersPolicy::InitPolicy()
{
    SL_diff = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_STOP_LOSS_DIFF));
    TP_diff = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_TAKE_PROFIT_DIFF));

    SL_start = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_STOP_LOSS_START));
    TP_start = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_TAKE_PROFIT_START));

    Dynamic_SL_TP = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFiG_DEF_ENABLE_DYNAMIC_TP_SL)) != 0 ? true : false;


}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

inline std::vector<std::vector<std::string>> OrderTracerPolicy::Get_All()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIG_DEF_ENABLE_TRADE_TRACING),
        std::make_pair(CONFIG_DEF_ENABLE_DYNAMIC_TIMELINE_TRADE_TRACING),
        std::make_pair(CONFIG_DEF_ENABLE_TRADE_TRACING)
    };

    return Get_AllConfigs(allConfigs);
}

void OrderTracerPolicy::InitPolicy()
{
    EnableTradeTracing = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_ENABLE_TRADE_TRACING)) > 0 ? true : false;
    EnableDynamicTimeLine_TradeTracing = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_ENABLE_DYNAMIC_TIMELINE_TRADE_TRACING)) > 0 ? true : false;
    TraceCurrTimeLineLen = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFG_DEF_TRACE_CURR_TIMELINE_LEN));
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

inline std::vector<std::vector<std::string>> PolynomComparerPolicy::Get_All()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIGS_POLYNOM_EVAL_WEIGHT),
        std::make_pair(CONFIGS_POLYNOM_RMSE_WEIGHT),
        std::make_pair(CONFIGS_POLYNOM_COEFF_WEIGHT),
        std::make_pair(CONFIG_DEF_ENABLE_POLYNOM_SCAN),
        std::make_pair(CONFIGS_POLYNOM_LEN_FOR_SCAN),
		std::make_pair(CONFIG_DEF_PATTERN_MIN_TOTAL_SCORE_TO_ACT_POLYNOM_COMPARER)
    };

    return Get_AllConfigs(allConfigs);
}

void PolynomComparerPolicy::InitPolicy()
{
    EnablePolynomScan = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_ENABLE_POLYNOM_SCAN)) > 0 ? true : false;
	ScoreToAct = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MIN_TOTAL_SCORE_TO_ACT_POLYNOM_COMPARER));
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

inline std::vector<std::vector<std::string>> ScoreComparerPolicy::Get_All()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIG_DEF_RSI_WIEGHT),
        std::make_pair(CONFIG_DEF_EMA_WIEGHT),
        std::make_pair(CONFIG_DEF_ZLSMA_WIEGHT),
        std::make_pair(CONFIG_DEF_VOLUME_WIEGHT),
		std::make_pair(CONFIG_DEF_PATTERN_MIN_TOTAL_SCORE_TO_ACT_SCORE_COMPARER),
		std::make_pair(CONFIG_DEF_ENABLE_SCORE_SCAN)
    };

    return Get_AllConfigs(allConfigs);
}

void ScoreComparerPolicy::InitPolicy()
{
    RSI_Wieght = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_RSI_WIEGHT));
    EMA_Wieght = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_EMA_WIEGHT));
    ZLSMA_Wieght = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_ZLSMA_WIEGHT));
    VOLUME_Wieght = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_VOLUME_WIEGHT));
	EnableScoreScan = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_ENABLE_SCORE_SCAN)) > 0 ? true : false;
	ScoreToAct = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MIN_TOTAL_SCORE_TO_ACT_SCORE_COMPARER));
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

std::vector<std::vector<std::string>> FusionEnginePolicy::Get_All()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIGS_MIN_SIMILAR_RESULTS_TO_FUSION)
    };

    return Get_AllConfigs(allConfigs);
}

void FusionEnginePolicy::InitPolicy()
{
    Min_Similar_Results_To_Fusuion = 2;
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

inline std::vector<std::vector<std::string>> InfraConfigrationsPolicy::Get_All()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIG_HTTP_PORT)
    };

    return Get_AllConfigs(allConfigs);
}

void InfraConfigrationsPolicy::InitPolicy()
{
    http_server_port = "8080";
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

inline std::vector<std::vector<std::string>> PatternGodStatisticsPolicy::Get_All()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIG_DEF_PATTERN_FUTURE_VISION_CANDLES),
        std::make_pair(CONFIG_DEF_SAVED_STATS_LEN)
    };
    return Get_AllConfigs(allConfigs);
}

inline void PatternGodStatisticsPolicy::InitPolicy()
{
    Score_Future_Vision_Candles = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_FUTURE_VISION_CANDLES));
    SavedDirectionStaticsLen = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_SAVED_STATS_LEN));
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

inline void BaseComparerPolicy::InitPolicy()
{
    Min_Similar_Results_To_Fusion = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIGS_MIN_SIMILAR_RESULTS_TO_FUSION));
    PolynomComparerWeight = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_POLYNOM_FUSION_WEIGHT));
    ScoreComparerWeight = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_SCORE_FUSION_WEIGHT));
}
