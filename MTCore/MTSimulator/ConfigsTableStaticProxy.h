#include <string>
#include "ConfigsDefs.h"
#include <vector>
#pragma once
class ConfigsTableStaticProxy
{
private:
    static std::string extractTextBetweenSymbols(const std::string& line);
    static std::string DefualtConfigValues(const std::string& config);
    static std::string ReadConfigValue(const std::string& config);

public:
    static std::string Get_ConfigValue_DB(const std::string& config, const std::string& defualt );
    static bool UpdateConfigValue_DB(const std::string& conf, const std::string& value);

    static std::string Get_ConfigValue(const std::string& conf) { return ReadConfigValue(conf); }
    static std::string Get_ConfigValue(const std::string& conf,const std::string& defualt) { return ReadConfigValue(conf).length() ? ReadConfigValue(conf) : defualt; }

    static std::vector<std::string> Get_ConfigRow_DB(const std::string& config,const std::string& defualt);
};

