#include "Policies.h"
#include "Converts.h"
#include "ThreadPool.h"


/////////////////////////////////////////////////////////////////////////
///////////////Policy Initialize for ConfigrationsPolicy ////////////////
/////////////////////////////////////////////////////////////////////////

std::vector<std::vector<std::string>> BasePolicy::Get_All()
{
    std::vector<std::vector<std::string>> result;
    for (const auto& config : m_allConfigs)
    {
        auto currConf = ConfigsTableStaticProxy::Get_ConfigRow_DB(config.first, config.second);
        currConf.push_back(PolicyTypeUtils::GetStringFromEnum(GetPolicyType()));
        result.push_back(currConf);
        
    }
    return result;
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

void ConfigrationsPolicy::InitPolicy()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
    std::make_pair(CONFIG_DEF_CURR_TIME_LINE_LEN),
    std::make_pair(CONFIG_DEF_EMA_LEN_FOR_PATTERN_EXPORT),
    std::make_pair(CONFIG_DEF_TIME_BETWEEN_SCANS),
    std::make_pair(CONFG_DEF_EXPORT_PATTERNS_PNG_TO_TELEGRAM),
    std::make_pair(CONFG_DEF_ENABLE_PATTERNS_PNG_EXPORT),
    };
	m_allConfigs = allConfigs;

    Pattern_CurrentTimeLineLen = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_CURR_TIME_LINE_LEN));

    EMA_len_for_Pattern_Export = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_EMA_LEN_FOR_PATTERN_EXPORT));

    TimeBetweenScans = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_TIME_BETWEEN_SCANS));
    Export_Patterns_PNG_To_Telegram = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFG_DEF_EXPORT_PATTERNS_PNG_TO_TELEGRAM)) > 0 ? true : false;
    Enable_Patterns_PNG_Export = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFG_DEF_ENABLE_PATTERNS_PNG_EXPORT)) > 0 ? true : false;
    ThreadsCount = MTThreadPool::getInstance()->Get_ThreadsNum();
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

void TradeManagerPolicy::InitPolicy()
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
	m_allConfigs = allConfigs;

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

void ScoreCachingPolicy::InitPolicy()
{

    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIG_DEF_ENABLE_SCORE_CACHEING),
        std::make_pair(CONFIG_DEF_PATTERN_MAX_ORDERS_INROW),
        std::make_pair(CONFIG_DEF_PATTERN_SLEEP_MINUTS_AFTER_MAX_ORDERS_INROW)

    };
	m_allConfigs = allConfigs;
    EnableScoreCaching = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_ENABLE_SCORE_CACHEING)) > 0 ? true : false;
    Max_Orders_InRow = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MAX_ORDERS_INROW));
    Sleep_Minuts_after_Max_Orders_InRow = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_SLEEP_MINUTS_AFTER_MAX_ORDERS_INROW));
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

void OrdersPolicy::InitPolicy()
{

    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIG_DEF_STOP_LOSS_START),
        std::make_pair(CONFIG_DEF_TAKE_PROFIT_START),
        std::make_pair(CONFIG_DEF_STOP_LOSS_DIFF),
        std::make_pair(CONFIG_DEF_TAKE_PROFIT_DIFF),
        std::make_pair(CONFiG_DEF_ENABLE_DYNAMIC_TP_SL),
        std::make_pair(CONFIG_DEF_MAIN_ORDER_SIZE_IN_LOT),
        std::make_pair(CONFIG_DEF_SUPPORT_ORDER_SIZE_IN_LOT)
    };
	m_allConfigs = allConfigs;

    SL_diff = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_STOP_LOSS_DIFF));
    TP_diff = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_TAKE_PROFIT_DIFF));

    SL_start = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_STOP_LOSS_START));
    TP_start = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_TAKE_PROFIT_START));

    Dynamic_SL_TP = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFiG_DEF_ENABLE_DYNAMIC_TP_SL)) != 0 ? true : false;
	Main_Order_Size_In_Lot = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_MAIN_ORDER_SIZE_IN_LOT));
	Support_Order_Size_In_Lot = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_SUPPORT_ORDER_SIZE_IN_LOT));
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------


void OrderTracerPolicy::InitPolicy()
{

    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIG_DEF_ENABLE_TRADE_TRACING),
        std::make_pair(CONFIG_DEF_ENABLE_DYNAMIC_TIMELINE_TRADE_TRACING),
        std::make_pair(CONFIG_DEF_ENABLE_TRADE_TRACING)
    };
	m_allConfigs = allConfigs;

    EnableTradeTracing = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_ENABLE_TRADE_TRACING)) > 0 ? true : false;
    EnableDynamicTimeLine_TradeTracing = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_ENABLE_DYNAMIC_TIMELINE_TRADE_TRACING)) > 0 ? true : false;
    TraceCurrTimeLineLen = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFG_DEF_TRACE_CURR_TIMELINE_LEN));
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------


void PolynomComparerPolicy::InitPolicy()
{

    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIG_DEF_ENABLE_POLYNOM_SCAN),
        std::make_pair(CONFIGS_POLYNOM_LEN_FOR_SCAN),
        std::make_pair(CONFIG_DEF_PATTERN_MIN_TOTAL_SCORE_TO_ACT_POLYNOM_COMPARER),
        std::make_pair(CONFIGS_POLYNOM_EVAL_WEIGHT),
        std::make_pair(CONFIGS_POLYNOM_RMSE_WEIGHT),
        std::make_pair(CONFIGS_POLYNOM_COEFF_WEIGHT)
    };
	m_allConfigs = allConfigs;

    EnablePolynomScan = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_ENABLE_POLYNOM_SCAN)) > 0 ? true : false;
	ScoreToAct = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MIN_TOTAL_SCORE_TO_ACT_POLYNOM_COMPARER));
	PolynomLen_ForScan = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIGS_POLYNOM_LEN_FOR_SCAN));
    weight_eval = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIGS_POLYNOM_EVAL_WEIGHT));
    weight_rmse = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIGS_POLYNOM_RMSE_WEIGHT));
    weight_coeff = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIGS_POLYNOM_COEFF_WEIGHT));
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------


void ScoreComparerPolicy::InitPolicy()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
    std::make_pair(CONFIG_DEF_RSI_WIEGHT),
    std::make_pair(CONFIG_DEF_EMA_WIEGHT),
    std::make_pair(CONFIG_DEF_ZLSMA_WIEGHT),
    std::make_pair(CONFIG_DEF_VOLUME_WIEGHT),
    std::make_pair(CONFIG_DEF_PATTERN_MIN_TOTAL_SCORE_TO_ACT_SCORE_COMPARER),
    std::make_pair(CONFIG_DEF_ENABLE_SCORE_SCAN)
    };
	m_allConfigs = allConfigs;

    RSI_Wieght = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_RSI_WIEGHT));
    EMA_Wieght = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_EMA_WIEGHT));
    ZLSMA_Wieght = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_ZLSMA_WIEGHT));
    VOLUME_Wieght = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_VOLUME_WIEGHT));
	EnableScoreScan = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_ENABLE_SCORE_SCAN)) > 0 ? true : false;
	ScoreToAct = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_MIN_TOTAL_SCORE_TO_ACT_SCORE_COMPARER));
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

void FusionEnginePolicy::InitPolicy()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
    std::make_pair(CONFIGS_MIN_SIMILAR_RESULTS_TO_FUSION)
    };

    Min_Similar_Results_To_Fusuion = 2;
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

void InfraConfigrationsPolicy::InitPolicy()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
    std::make_pair(CONFIG_HTTP_PORT)
    };
    m_allConfigs = allConfigs;


    http_server_port = "8080";
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------


void PatternGodStatisticsPolicy::InitPolicy()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIG_DEF_PATTERN_FUTURE_VISION_CANDLES),
        std::make_pair(CONFIG_DEF_SAVED_STATS_LEN),
        std::make_pair(CONFIG_DEF_ENABLE_WEIGHTED_VOTING),
        std::make_pair(CONFIG_DEF_WEIGHTING_METHOD_FOR_VOTING),
        std::make_pair(CONFIG_DEF_SOFTMAX_T_PARAM_FOR_VOTING),
        std::make_pair(CONFIG_DEF_POWERLAW_EXPONENT_FOR_VOTING)
    };
	m_allConfigs = allConfigs;

    Score_Future_Vision_Candles = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_PATTERN_FUTURE_VISION_CANDLES));
    SavedDirectionStaticsLen = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_SAVED_STATS_LEN));
	Enable_Weighted_Voting = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_ENABLE_WEIGHTED_VOTING)) > 0 ? true : false;
	Weighting_Method_for_Weighting = Converts::ToInt(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_WEIGHTING_METHOD_FOR_VOTING));
	softmax_T_param_for_Weighting = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_SOFTMAX_T_PARAM_FOR_VOTING));
	powerlaw_exponent_for_Weighting = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_DEF_POWERLAW_EXPONENT_FOR_VOTING));
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

void BaseComparerPolicy::InitPolicy()
{
    std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
        std::make_pair(CONFIGS_MIN_SIMILAR_RESULTS_TO_FUSION),
        std::make_pair(CONFIG_POLYNOM_FUSION_WEIGHT),
        std::make_pair(CONFIG_SCORE_FUSION_WEIGHT)
    };
	m_allConfigs = allConfigs;

    Min_Similar_Results_To_Fusion = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIGS_MIN_SIMILAR_RESULTS_TO_FUSION));
    PolynomComparerWeight = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_POLYNOM_FUSION_WEIGHT));
    ScoreComparerWeight = Converts::ToDouble(ConfigsTableStaticProxy::Get_ConfigValue_DB(CONFIG_SCORE_FUSION_WEIGHT));
}
