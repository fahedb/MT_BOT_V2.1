#pragma once
#include <string>
#include "ConfigsTableStaticProxy.h"
#include "PolicyTypeEnum.h"



/// <summary>
/// Base class for all policies.
/// </summary>

class BasePolicy
{
public:
    virtual ~BasePolicy() = default;
	virtual std::vector<std::vector<std::string>> Get_All();
	virtual void Refresh() { InitPolicy(); } // Refresh the policy by re-initializing it
	virtual PolicyType GetPolicyType() const = 0; // Pure virtual function to get the policy type
protected:
	std::vector<std::pair<std::string, std::string>> m_allConfigs; // Store all configurations as pairs of <config_name, config_value>
private:
	virtual void InitPolicy() = 0; // Pure virtual function to initialize the policy
};

// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

// <summary>
/// Class for Infra Configurations Policy.
/// Contains configurations related to the infrastructure such as HTTP server port.
/// </summary>

class InfraConfigrationsPolicy : public BasePolicy
{
public:
    InfraConfigrationsPolicy()
    {
        InitPolicy();
    }
    std::string http_server_port;
	virtual PolicyType GetPolicyType() const override { return PolicyType::InfraConfigrationsPolicy; }
private:
    void InitPolicy();
};

// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

/// <summary>
/// Defines a policy for tracing orders, inheriting from BasePolicy.
/// </summary>

struct OrderTracerPolicy : public BasePolicy
{
public:
    OrderTracerPolicy()
    {
        InitPolicy();
    }
	virtual PolicyType GetPolicyType() const override { return PolicyType::OrderTracerPolicy; }
    
    bool EnableTradeTracing = false;
    bool EnableDynamicTimeLine_TradeTracing = false;
    int TraceCurrTimeLineLen = 0;

private:
    void InitPolicy() override;
};



// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

/// <summary>
/// Score caching policy
/// </summary>

struct ScoreCachingPolicy : public BasePolicy
{
public:
    ScoreCachingPolicy()
    {
        InitPolicy();
    }
	virtual PolicyType GetPolicyType() const override { return PolicyType::ScoreCachingPolicy; }

    bool EnableScoreCaching = true;
    long ScoreResultsCacheSizePerThread = 100000; // Default size for each thread's cache
    int maxScorePossibleForChangeNodes = 0;
    int CleanEveryXScans = 3; // Default to clean cache every 10 scans

private:
    void InitPolicy();
};


// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------



// <summary>
/// Class for Configurations Policy.
/// Contains general policy configs
// </summary>

class ConfigrationsPolicy : public BasePolicy
{
public:
    ConfigrationsPolicy()
    {
        InitPolicy(); // Initialize the policy
	}
	virtual PolicyType GetPolicyType() const override { return PolicyType::ConfigrationsPolicy; }
    int ThreadsCount = 0; // Default to number of hardware threads minus one

    
private:
    void InitPolicy() override;
};

// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------




/////////////////////////////////////////////////
////////// Scoring layer (Layer 1) //////////////
/////////////////////////////////////////////////

class ScoringLayerPolicy : public BasePolicy
{
public:
    ScoringLayerPolicy()
    {
        InitPolicy(); // Initialize the policy
    }
    virtual PolicyType GetPolicyType() const override { return PolicyType::ScoringLayerPolicy; }
    int Pattern_CurrentTimeLineLen = 0;
    int Num_Of_Top_Results_To_Filter_Out = 100; // Number of top results to filter out

private:
    void InitPolicy() override;
};

struct ScoreComparerPolicy : public BasePolicy
{
    // <summary>
    /// Class for Score Comparer Policy.
    /// Contains configurations related to score comparison, such as weights for different score types.
    /// </summary>
public:
    ScoreComparerPolicy()
    {
        InitPolicy();
    }
    virtual PolicyType GetPolicyType() const override { return PolicyType::ScoreComparerPolicy; }

    int RSI_Wieght = 0;
    int EMA_Wieght = 0;
    int ZLSMA_Wieght = 0;
    int VOLUME_Wieght = 0;
    bool EnableScoreScan = 0;
    int ScoreToAct = 0;
    int Diff_Between_Max_And_ScoreToAct = 250; // Difference between maximum score and score to act
    std::string scanFilters;
private:
    void InitPolicy();
};

/////////////////////////////////////////////////
////////// Filter layer (Layer 2) ///////////////
/////////////////////////////////////////////////
class FilterLayerPolicy : public BasePolicy
{
public:
    virtual PolicyType GetPolicyType() const override { return PolicyType::FilterLayerPolicy; }
    FilterLayerPolicy()
    {
        InitPolicy();
    }
    double Pattern_Max_Patterns_To_Find = 0.0;
    double Pattern_TopScore_ThresholdRatio = 0.0;

private:
    void InitPolicy() override;
};

struct PolynomComparerPolicy : public BasePolicy
{
    // <summary>
    /// Class for Polynom Comparer Policy.
    /// Contains configurations related to polynomial comparison, such as weights for evaluation, RMSE, and coefficient similarity.
    /// </summary>
public:
    PolynomComparerPolicy()
    {
        InitPolicy();
    }
    virtual PolicyType GetPolicyType() const override { return PolicyType::PolynomComparerPolicy; }

    double weight_eval = 0.5; // Weight for evaluation score
    double weight_rmse = 0.3; // Weight for RMSE score
    double weight_coeff = 0.2; // Weight for coefficient similarity score

    bool EnablePolynomScan = 0;
	bool EnablePolynomTimeLineCaching = 0;
    int PolynomLen_ForScan = 5; // Length of the polynomial to scan

    int ScoreToAct = 0;
    int Diff_Between_Max_And_ScoreToAct = 250; // Difference between maximum score and score to act
private:
    void InitPolicy();
};


////////////////////////////////////////////////////
////////// Statistics layer (Layer 3) //////////////
////////////////////////////////////////////////////
class PatternGodStatisticsPolicy : public BasePolicy
{
public:
    PatternGodStatisticsPolicy()
    {
        InitPolicy();
    }
    virtual PolicyType GetPolicyType() const override { return PolicyType::PatternGodStatisticsPolicy; }

    int Score_Future_Vision_Candles = 0;
    int SavedDirectionStaticsLen = 0;

    bool Enable_Weighted_Voting = false;
    int Weighting_Method_for_Weighting = 0; // 0 = Linear, 1 = PowerLaw, 2 = SoftMax

    double softmax_T_param_for_Weighting = 0.5;
    double powerlaw_exponent_for_Weighting = 2.0;

private:
    void InitPolicy();
};



///////////////////////////////////////////////////////
////////// Trade Manager layer policies (Layer 4) //////////////
///////////////////////////////////////////////////////
struct TradeManagerPolicy : public BasePolicy
{
public:
    TradeManagerPolicy()
    {
        InitPolicy();
    }
    virtual PolicyType GetPolicyType() const override { return PolicyType::TradeManagerPolicy; }

    double Pattern_Min_Patterns_ToTake_Action = 0.0;
    double Pattern_Min_Percent_To_ExecuteOrder = 0.0;
    double Pattern_Diff_Percent_To_ExecuteOrder_Alternative = 0.0;
    double Pattern_Max_Noise_To_ExecuteOrder_Alternative = 0.0;
    int Pattern_Activate_Alternative_ExecuteOrder = 0;
    double Pattern_Min_PipsChange_ToCount = 0.0;
    double Min_MeanPips_ToExecute_Order = 0.0;

private:
    void InitPolicy();
};

struct OrdersPolicy : public BasePolicy
{
public:
    OrdersPolicy()
    {
        InitPolicy();
    }
    virtual PolicyType GetPolicyType() const override { return PolicyType::OrdersPolicy; }

    int SL_start = 0;
    int TP_start = 0;

    int SL_diff = 0;
    int TP_diff = 0;

    double Main_Order_Size_In_Lot = 0.0; // Default main order size in lots
    double Support_Order_Size_In_Lot = 0.0; // Default support order size in lots

    bool Dynamic_SL_TP = false; // Dynamic Stop Loss and Take Profit

private:
    void InitPolicy() override;
};



/////////////////////////////////////////////////
////////// Export layer /////////////////////////
/////////////////////////////////////////////////

class ExportDataLayerPolicy : public BasePolicy
{
public:
    virtual PolicyType GetPolicyType() const override { return PolicyType::ExportDataLayerPolicy; }

    ExportDataLayerPolicy()
    {
        InitPolicy();
    }
    bool Export_Patterns_PNG_To_Telegram = false;
    bool Enable_Patterns_PNG_Export = false;
    int EMA_len_for_Pattern_Export = 0;

private:
    void InitPolicy() override;
};