#include "Configrations.h"
#include "ConfigsTableStaticProxy.h"

//#include "ConfigsTable.h"
//#include "Converts.h"



// Private constructor to prevent instantiation
Configrations::Configrations() {
    m_connString = ConfigsTableStaticProxy::Get_ConfigValue(CONFIG_DEF_CONNECTION_STRING);
    m_filesDBDir = ConfigsTableStaticProxy::Get_ConfigValue(CONFIG_DEF_FILES_DB_DIR);
    InitPolicyMap();
}

void Configrations::ResetPolicy()
{
    PolicyTypeList policyTypes;
    for (const auto& type : policyTypes.types)
    {
        delete m_policies[type];
    }
    m_policies.clear();
}

void Configrations::InitPolicyMap()
{
    m_policies[PolicyType::InfraConfigrationsPolicy] = new InfraConfigrationsPolicy();
    m_policies[PolicyType::FusionEnginePolicy] = new FusionEnginePolicy();
    m_policies[PolicyType::ScoreComparerPolicy] = new ScoreComparerPolicy();
    m_policies[PolicyType::PolynomComparerPolicy] = new PolynomComparerPolicy();
    m_policies[PolicyType::ConfigrationsPolicy] = new ConfigrationsPolicy();
    m_policies[PolicyType::OrderTracerPolicy] = new OrderTracerPolicy();
    m_policies[PolicyType::OrdersPolicy] = new OrdersPolicy();
    m_policies[PolicyType::TradeManagerPolicy] = new TradeManagerPolicy();
    m_policies[PolicyType::ScoreCachingPolicy] = new ScoreCachingPolicy();
    m_policies[PolicyType::PatternGodStatisticsPolicy] = new PatternGodStatisticsPolicy();
    m_policies[PolicyType::BaseComparerPolicy] = new BaseComparerPolicy();
}

std::string Configrations::Get_Config_Main(const std::string& conf, const std::string& value) { return ConfigsTableStaticProxy::Get_ConfigValue(conf, value); }
std::string Configrations::Get_Config_DB(const std::string& conf, const std::string& value) { return ConfigsTableStaticProxy::Get_ConfigValue_DB(conf, value); }

std::vector<std::vector<std::string>> Configrations::Get_AllConfigRows_DB()
{
    std::vector<std::vector<std::string>> allConfigs;
	PolicyTypeList policyTypes;
    for(const auto& type : policyTypes.types)
    {
        BasePolicy* policy = m_policies[type];
        if (policy)
        {
            std::vector<std::vector<std::string>> configs = policy->Get_All();
            allConfigs.insert(allConfigs.end(), configs.begin(), configs.end());
        }
	}
    return allConfigs;
}

void Configrations::RefreshPolicy() {
    for(const auto& pair : m_policies) {
        pair.second->Refresh(); // Refresh each policy
	}
}

