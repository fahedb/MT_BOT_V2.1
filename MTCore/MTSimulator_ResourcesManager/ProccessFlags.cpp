#include "ProccessFlags.h"


void ProccessFlagsRep::Set_Flags(int argc, char* argv[])
{
    m_flags.reserve(argc);
    for (int i = 0; i < argc; i++)
    {
        std::string flag(argv[i]);
        m_flags.push_back(flag);
    }
}


bool ProccessFlagsRep::IsFlagExist(const std::string& flagToFind)
{
    for (const auto& flag : m_flags)
    {
        if (flagToFind == flag)
            return true;
    }
    return false;
}

bool ProccessFlagsRep::GetFlagValue(const std::string& flagToFind, std::string& flagValue)
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