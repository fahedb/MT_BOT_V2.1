#pragma once
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <codecvt> // For wstring_convert


class DataBaseConnection
{

public:
    DataBaseConnection(const std::string& serverName,const std::string& dataBase,const std::string& userName,const std::string& pass);
    SQLHENV Get_EnvHandler() { return m_envHandler; }
    SQLHDBC Get_DBConnection() { return m_DBConnectionHandler; }

    DataBaseConnection();
    DataBaseConnection(const std::string& connString);

      ~DataBaseConnection()
      {
          CloseConnection();
      }
      
public :
    void CloseConnection() {
        SQLDisconnect(m_envHandler);
        SQLFreeHandle(SQL_HANDLE_DBC, m_DBConnectionHandler);
        SQLFreeHandle(SQL_HANDLE_ENV, m_envHandler);
    }
    bool IsConnected();

private:
    SQLHENV m_envHandler;
    SQLHDBC m_DBConnectionHandler;

};

