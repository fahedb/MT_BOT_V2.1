#pragma once
#include <string>
#include <vector>
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::Collections::Generic;
using namespace msclr::interop;

public ref class NativeToManagedConverter
{
public:
    // std::string -> System::String^
    static String^ ConvertString(const std::string& nativeStr)
    {
        return gcnew String(nativeStr.c_str());
    }

    // std::vector<std::string> -> List<String^>
    static List<String^>^ ConvertStringVector(const std::vector<std::string>& nativeVec)
    {
        List<String^>^ managedList = gcnew List<String^>();
        for (const auto& str : nativeVec)
        {
            managedList->Add(ConvertString(str));
        }
        return managedList;
    }
};

