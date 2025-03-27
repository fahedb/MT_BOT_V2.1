#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include "DataBaseConnection.h"
#include "BasePricesTable.h"


class DataLoad:BasePricesTable
{
	std::string m_dataSource;
public:
    DataLoad(std::string source, std::string loadTo, SQLHENV henv, SQLHDBC hdbc)
        : BasePricesTable(henv, hdbc, loadTo)
    {
        m_dataSource = source;
    }
private: 
    std::vector<std::vector<std::string>> LoadedVector;
    int currentLine = 0;
    std::vector<std::string> splitString(std::string str, std::string splitBy);
    bool InsertToDB(std::vector<std::string> record);
    std::string TimeForamtting(std::string line);

    std::string GenerateTimeString(std::string tm);
      bool loadToDB(const std::vector<std::string>& lines, int* lastIdx)
      {
          int i = 0;

          for (; i < lines.size(); i++)
          {
              InsertToDB(splitString(lines.at(i), ","));
          }
          return true;
      }
      
public:
    std::vector<std::vector<std::string>>* Get_LoadedVector()
    {
        return &LoadedVector;
    }
    std::vector<std::vector<std::string>>* LoadToVector()
    {
        LoadedVector.clear();
        std::ifstream file(m_dataSource); // Open file stream

        if (!file.is_open()) {
            std::cerr << "Failed to open the file." << std::endl;
            return 0;
        }

        std::vector<std::string> lines; // Vector to store lines

        std::string line;
        while (std::getline(file, line)) {
            line = TimeForamtting(line);
            lines.push_back(line); // Store each line in the vector
        }

        file.close(); // Close file stream
        for (int i = 0; i < lines.size(); i++)
        {
            LoadedVector.push_back(splitString(lines.at(i), ","));
        }
        return &LoadedVector;

    }


    bool Load(int* lastIdx)
    {
        std::ifstream file(m_dataSource); // Open file stream

        if (!file.is_open()) {
            std::cerr << "Failed to open the file." << std::endl;
            return 0;
        }

        std::vector<std::string> lines; // Vector to store lines

        std::string line;
        while (std::getline(file, line)) {
            line = TimeForamtting(line);
            lines.push_back(line); // Store each line in the vector
        }

        file.close(); // Close file stream
        return loadToDB(lines, lastIdx);
    }

          virtual void StartInsertCommand() {};
          virtual void StartQueryCommand() {};
          virtual void AddFilter_PriceDateTime(std::string pdt) {};
          virtual void AddFilter_PriceDateTime(std::string pdt, std::string relation) {};
          virtual void AddFilter_SellPrice(float sp) {};
          virtual void AddFilter_ActionSubmitted(bool as) {};
          virtual void AddSort_ProceDateTime(bool asc) {};
          virtual void AddFilter_BuyPrice(float bp) {};
          virtual void RunQuery() {};
          virtual void FreeStyleQuery(std::string sqlCommand) {};
          virtual void RemoveSelect() {};
          virtual void RunInsert() {};
};


