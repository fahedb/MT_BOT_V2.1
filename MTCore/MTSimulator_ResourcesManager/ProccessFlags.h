#include <string>
#include <vector>
#pragma once

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
    bool IsFlagExist(const std::string& flagToFind)
    {
        for (const auto& flag : m_flags)
        {
            if (flagToFind == flag)
                return true;
        }
        return false;
	}
    bool GetFlagValue(const std::string& flagToFind,std::string& flagValue)
    {
        for (int i = 0; i < m_flags.size(); i++)
        {
            if (flagToFind == m_flags.at(i))
            {
                if (i + 1 < m_flags.size())
                    flagValue = m_flags.at(i + 1);
                return true;
            }
        }
        return false;
    }
    

    void Set_Flags(int argc, char* argv[])
    {
        m_flags.reserve(argc);
        for (int i = 0; i < argc; i++)
        {
            std::string flag(argv[i]);
            m_flags.push_back(flag);
        }
    }

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
