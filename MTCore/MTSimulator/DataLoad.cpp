#include "DataLoad.h"

std::vector<std::string> DataLoad::splitString(std::string str, std::string splitBy)
{
    std::string currStr = "";
    std::vector<std::string> record;
    
    for (int i = 0; i < str.length(); i++)
    {
        if (str[i] == ',')
        {
            record.push_back(currStr);
            currStr = "";
            i++;
        }
        currStr += str[i];

    }
    record.push_back(currStr);
    return record;
}

bool DataLoad::InsertToDB(std::vector<std::string> record)
{
    if (!std::stoi(record.at(5)))
    {
        return true;
    }
    ClearAll();
    FPriceDateTime(record.at(0));
    FBuyPrice(std::stof(record.at(1)));
    FSellPrice(std::stof(record.at(4)));
    FActionSubmitted(1);
    return SubmitInsert();
}

std::string DataLoad::TimeForamtting(std::string line)
{
    std::string time = "";
    std::string restOfString = "";
    bool SwitCh = true;
    for (int i = 0; i < line.length(); i++)
    {
        if (SwitCh) {
            if (line[i] == ',') {
                SwitCh = false;
            }
            time += line[i];
        }
        else {
            restOfString += line[i];
        }

    }
    time = GenerateTimeString(time);
    time += ",";
    time += restOfString;
    return time;

}

std::string DataLoad::GenerateTimeString(std::string tm)
{
    std::string toReturn = "";
    toReturn += TCHAR(tm[6]);
    toReturn += TCHAR(tm[7]);
    toReturn += TCHAR(tm[8]);
    toReturn += TCHAR(tm[9]);
    toReturn += TCHAR(tm[3]);
    toReturn += TCHAR(tm[4]);
    toReturn += TCHAR(tm[0]);
    toReturn += TCHAR(tm[1]);
    toReturn += TCHAR(tm[11]);
    toReturn += TCHAR(tm[12]);
    toReturn += TCHAR(tm[14]);
    toReturn += TCHAR(tm[15]);
    toReturn += TCHAR(tm[17]);
    toReturn += TCHAR(tm[18]);
    toReturn += TCHAR(tm[20]);
    toReturn += TCHAR(tm[21]);
    toReturn += TCHAR(tm[22]);
    return toReturn;
}
