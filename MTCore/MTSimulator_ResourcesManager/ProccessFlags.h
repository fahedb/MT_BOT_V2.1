#pragma once
#include <string>
#include <vector>
#include "FlagsDef.h"


class ProccessFlagsRep
{
public:
    // Deleted copy constructor and assignment operator to prevent copies
    ProccessFlagsRep(const ProccessFlagsRep&) = delete;
    ProccessFlagsRep& operator=(const ProccessFlagsRep&) = delete;

    // Static method to get the instance of the Singleton
    static ProccessFlagsRep& getInstance() {
        static ProccessFlagsRep instance; // Guaranteed to be destroyed and instantiated on first use
        return instance;
    }
    bool IsFlagExist(const std::string& flagToFind);
    bool GetFlagValue(const std::string& flagToFind, std::string& flagValue);
    

    void Set_Flags(int argc, char* argv[]);

private:
    std::vector<std::string> m_flags;
    ProccessFlagsRep()
    {

    }
    ~ProccessFlagsRep()
    {
    }

};

class ProccessFlags
{
public:
    static bool FindSwitch(const std::string& flag) { return ProccessFlagsRep::getInstance().IsFlagExist(flag); }
    static std::string GetSwitch(const std::string& flag) {
        std::string flagValue;
        ProccessFlagsRep::getInstance().GetFlagValue(flag, flagValue);
        return flagValue;
    }

    static void InitProccessFlags(int argc, char* argv[])
    {
        ProccessFlagsRep::getInstance().Set_Flags(argc, argv);
    }
};
