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
    m_policies[PolicyType::ScoreCachingPolicy] = new ScoreCachingPolicy();
    m_policies[PolicyType::PatternGodStatisticsPolicy] = new PatternGodStatisticsPolicy();
    m_policies[PolicyType::BaseComparerPolicy] = new BaseComparerPolicy();
}

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

inline void Configrations::RefreshPolicy() {
    ResetPolicy();
    InitPolicyMap();
}

