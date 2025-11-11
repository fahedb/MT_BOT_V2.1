#include <string>
#include "ConfigsDefs.h"
#include <vector>
#include "..\MyRobinHoodMap\robin_hood.h"
#pragma once
class ConfigsTableStaticProxy
{
private:
    static std::string extractTextBetweenSymbols(const std::string& line);
    static std::string DefualtConfigValues(const std::string& config);
    static std::string ReadConfigValue(const std::string& config);

public:
    static std::vector<std::string> Get_ConfigRow_DB(const std::string& config,const std::string& defualt);
    static std::string              Get_ConfigValue_DB(const std::string& config, const std::string& defualt );
    static bool                     UpdateConfigValue_DB(const std::string& conf, const std::string& value);
    static bool                     InsertConfigRow_DB(const std::vector<std::string>& confRow);
    static bool                     IsConfigExists_DB(const std::string& conf);

    static std::string Get_ConfigValue(const std::string& conf) { return ReadConfigValue(conf); }
    static std::string Get_ConfigValue(const std::string& conf,const std::string& defualt) { 
        std::string res = ReadConfigValue(conf);
        return res.length() ? res : defualt;
    }


    static void Insert_IntoForcedConfigs_DB(const std::string& config, std::string value);
private:
    static std::string Get_ForcedConfigValues(const std::string& config);
	static robin_hood::unordered_map<std::string, std::string> m_forcedConfigs; // Configs that are forced to specific values

};

