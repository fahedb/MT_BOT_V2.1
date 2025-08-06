#pragma once
#include <string>
#include "ConfigsDefs.h"
#include <fstream>
#include <vector>
#include "DataBaseConnection.h"
#include <thread>
#include "ThreadPool.h"
#include "Policies.h"
#include "robin_hood.h"

struct EMA_Check_Details;

class Configrations // SingleTon class for all configrations of the program
{
public:
    // Deleted copy constructor and assignment operator to prevent copies
    Configrations(const Configrations&) = delete;
    Configrations& operator=(const Configrations&) = delete;
    // Static method to get the instance of the Singleton
    static Configrations* getInstance() {
        static Configrations instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }

    std::string Get_ConnectionString() { return m_connString; }
    std::string Get_DB_FileDir() { return m_filesDBDir; }
    
    std::string Get_Config_Main(const std::string& conf, const std::string& value) { return ConfigsTableStaticProxy::Get_ConfigValue(conf, value); }
    std::string Get_Config_DB(const std::string& conf, const std::string& value) { return ConfigsTableStaticProxy::Get_ConfigValue_DB(conf, value); }

    std::vector<std::vector<std::string>> Get_AllConfigRows_DB();
    
    void RefreshPolicy();

    template<typename T>
	T* Get_Policy(PolicyType type) // Template For Safe Casting to the specific policy type
    {
        auto it = m_policies.find(type);
        if (it != m_policies.end())
            return dynamic_cast<T*>(it->second);
        return nullptr;
    }

private:
    std::string m_connString;
    std::string m_filesDBDir;
   // ConfigsTable* m_confTable_DB;

    // Private constructor to prevent instantiation
    Configrations() {
        m_connString = ConfigsTableStaticProxy::Get_ConfigValue(CONFIG_DEF_CONNECTION_STRING);
        m_filesDBDir = ConfigsTableStaticProxy::Get_ConfigValue(CONFIG_DEF_FILES_DB_DIR);
        InitPolicyMap();
    }
    ~Configrations()
    {
        ResetPolicy();
    }
    robin_hood::unordered_map<PolicyType, BasePolicy*> m_policies; // Cache for config values
    
    void ResetPolicy();
    void InitPolicyMap();
	
};
