#pragma once

#include <string>
#include <vector>
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::Collections::Generic;
using namespace msclr::interop;

ref class ManagedToNativeConverter
{

public:
    // System::String^ -> std::string
    static std::string ConvertString(String^ managedStr)
    {
        return marshal_as<std::string>(managedStr);
    }

    // List<String^> -> std::vector<std::string>
    static std::vector<std::string> ConvertStringList(List<String^>^ managedList)
    {
        std::vector<std::string> nativeVec;
        for each (String ^ str in managedList)
        {
            nativeVec.push_back(ConvertString(str));
        }
        return nativeVec;
    }
};

