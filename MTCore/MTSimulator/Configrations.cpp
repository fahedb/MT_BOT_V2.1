#include "Configrations.h"
#include "ConfigsTable.h"
#include "Converts.h"
#include "ConfigsTableStaticProxy.h"


inline void Configrations::ResetPolicy()
{
    PolicyTypeList policyTypes;
    for (const auto& type : policyTypes.types)
    {
        delete m_policies[type];
    }
    m_policies.clear();
}

inline void Configrations::InitPolicyMap()
{
    m_policies[PolicyType::InfraConfigrationsPolicy] = new InfraConfigrationsPolicy();
    m_policies[PolicyType::FusionEnginePolicy] = new FusionEnginePolicy();
    m_policies[PolicyType::ScoreComparerPolicy] = new ScoreComparerPolicy();
    m_policies[PolicyType::PolynomComparerPolicy] = new PolynomComparerPolicy();
    m_policies[PolicyType::ConfigrationsPolicy] = new ConfigrationsPolicy();
    m_policies[PolicyType::OrderTracerPolicy] = new OrderTracerPolicy();
    m_policies[PolicyType::OrdersPolicy] = new OrdersPolicy();
    m_policies[PolicyType::TradeManagerPolicy] = new TradeManagerPolicy();
}

std::vector<std::vector<std::string>> Configrations::Get_AllConfigRows_DB()
{
    std::vector<std::vector<std::string>> allConfigs;
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_CURR_TIME_LINE_LEN));
    
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_PATTERN_MINIMUM_PERCENT_TO_EXECUTE_ORDER));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_PATTERN_MIN_MEAN_PIPS_TO_EXECUTE_ORDER));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFG_DEF_PATTERN_MAX_PATTERNS_TO_FIND));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_PATTERN_MINIMUM_PATTERNS_TO_TAKE_ACTION));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_PATTERN_THRESHOLD_RATIO_TOP_SCORES));
    
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_PATTERN_ACTIVATE_ALTERNATIVE_EXECUTE_ORDER));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_PATTERN_MAX_NOISE_PERCENT_TO_EXECUTE_ORDER_ALTERNATIVE));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_PATTERN_DIFF_PERCENT_TO_EXECUTE_ORDER_ALTERNATIVE));

    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_PATTERN_FUTURE_VISION_CANDLES));
    
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_PATTERN_MAX_ORDERS_INROW));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_PATTERN_SLEEP_MINUTS_AFTER_MAX_ORDERS_INROW));

    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_PATTERN_MIN_TOTAL_SCORE_TO_ACT));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_PATTERN_MINIMUM_PIPS_CHANGE_TOCOUNT));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_SAVED_STATS_LEN));
    
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_MAIN_ORDER_SIZE_IN_LOT));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_SUPPORT_ORDER_SIZE_IN_LOT));
    

    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_RSI_WIEGHT));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_EMA_WIEGHT));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_ZLSMA_WIEGHT));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_VOLUME_WIEGHT));

    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_STOP_LOSS_DIFF));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_TAKE_PROFIT_DIFF));
    
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_STOP_LOSS_START));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_TAKE_PROFIT_START));
    
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_EMA_LEN_FOR_PATTERN_EXPORT));

    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_TIME_BETWEEN_SCANS));

    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFG_DEF_EXPORT_PATTERNS_PNG_TO_TELEGRAM));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFG_DEF_ENABLE_PATTERNS_PNG_EXPORT));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFG_DEF_TRACE_CURR_TIMELINE_LEN));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_ENABLE_TRADE_TRACING));
    allConfigs.push_back(ConfigsTableStaticProxy::Get_ConfigRow_DB(CONFIG_DEF_ENABLE_DYNAMIC_TIMELINE_TRADE_TRACING));
    return allConfigs;
}

inline void Configrations::RefreshPolicy() {
    ResetPolicy();
    InitPolicyMap();
}

