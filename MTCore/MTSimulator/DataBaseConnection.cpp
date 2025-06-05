#include "DataBaseConnection.h"
#include "CONFIGRATIONS.H"

DataBaseConnection::DataBaseConnection(const std::string& serverName,const std::string& dataBase, const std::string& userName, const std::string& pass) {
    SQLHENV henv;       // Environment handle
    SQLHDBC hdbc;       // Connection handle
    SQLHSTMT hstmt;     // Statement handle
    SQLRETURN retcode;  // Return code

      // Allocate environment handle
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

    // Allocate connection handle
    SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

    // Connection string parameters
    std::string connString = "DRIVER={SQL Server};SERVER=" + serverName + ";DATABASE=" + dataBase + ";UID=" + userName + ";PWD=" + pass + ";";


    // convert to wchar string
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wide_str_connStr = converter.from_bytes(connString);

    // convert to wchar sql string
    SQLWCHAR* connectionString = const_cast<SQLWCHAR*>(wide_str_connStr.c_str());

    // Connect to SQL Server
    retcode = SQLDriverConnect(hdbc, NULL, connectionString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

    if (SQL_SUCCEEDED(retcode)) {
     //   std::cout << "Connected to SQL Server.\n";
        m_DBConnectionHandler = hdbc;
        m_envHandler = henv;
    }
    else {
        std::cout << "Connection to SQL Server failed.\n";
    }

}

DataBaseConnection::DataBaseConnection()
{
    SQLHENV henv;       // Environment handle
    SQLHDBC hdbc;       // Connection handle
    SQLHSTMT hstmt;     // Statement handle
    SQLRETURN retcode;  // Return code

      // Allocate environment handle
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

    // Allocate connection handle
    SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

    // Connection string parameters
    std::string connString = Configrations::getInstance()->Get_ConnectionString();

    // convert to wchar string
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wide_str_connStr = converter.from_bytes(connString);

    // convert to wchar sql string
    SQLWCHAR* connectionString = const_cast<SQLWCHAR*>(wide_str_connStr.c_str());

    // Connect to SQL Server
    retcode = SQLDriverConnect(hdbc, NULL, connectionString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

    if (SQL_SUCCEEDED(retcode)) {
        m_DBConnectionHandler = hdbc;
        m_envHandler = henv;
    }
    else {
        std::cout << "Connection to SQL Server failed.\n";
    }

}

DataBaseConnection::DataBaseConnection(const std::string& connString)
{
    SQLHENV henv;       // Environment handle
    SQLHDBC hdbc;       // Connection handle
    SQLHSTMT hstmt;     // Statement handle
    SQLRETURN retcode;  // Return code

      // Allocate environment handle
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

    // Allocate connection handle
    SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

    // Connection string parameters

    // convert to wchar string
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wide_str_connStr = converter.from_bytes(connString);

    // convert to wchar sql string
    SQLWCHAR* connectionString = const_cast<SQLWCHAR*>(wide_str_connStr.c_str());

    // Connect to SQL Server
    retcode = SQLDriverConnect(hdbc, NULL, connectionString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

    if (SQL_SUCCEEDED(retcode)) {
        m_DBConnectionHandler = hdbc;
        m_envHandler = henv;
    }
    else {
        std::cout << "Connection to SQL Server failed.\n";
    }
}


bool DataBaseConnection::IsConnected() {
    if (!m_DBConnectionHandler || !m_envHandler)
        return false;

    SQLRETURN ret;
    SQLUINTEGER connStatus = 0;

    ret = SQLGetConnectAttr(m_DBConnectionHandler, SQL_ATTR_CONNECTION_DEAD, &connStatus, 0, NULL);

    return (SQL_SUCCEEDED(ret) && connStatus == SQL_CD_FALSE); // אם False - החיבור חי
}