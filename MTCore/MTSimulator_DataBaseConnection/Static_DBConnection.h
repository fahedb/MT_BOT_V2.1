#pragma once
#include "DataBaseConnection.h"
/// <Static_DBConnection>
/// This class is should be used for frequently used structs\classes that uses the DB connections for one time for the object life time, 
/// this can prevent creating a connection to the DB over and over again
/// </Static_DBConnection>
/// 
class Static_DBConnection 
{
public:
    Static_DBConnection(const Static_DBConnection&) = delete;
    Static_DBConnection& operator=(const Static_DBConnection&) = delete;
    static Static_DBConnection* getInstance() {
        static Static_DBConnection instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }
    DataBaseConnection* GetConnection() { 
        if (EnsureConnected())
            return m_db_conn;
        return nullptr;
    }




private:
    DataBaseConnection* m_db_conn;

    bool EnsureConnected()
    {
        if (!m_db_conn)
            m_db_conn = new DataBaseConnection();

        if (m_db_conn->IsConnected())
            return true;

        m_db_conn->CloseConnection();
        delete m_db_conn;
        m_db_conn = new DataBaseConnection();

        return m_db_conn->IsConnected();
    }

    Static_DBConnection() {
        m_db_conn = new DataBaseConnection();
    }
    ~Static_DBConnection()
    {
        m_db_conn->CloseConnection();
        delete m_db_conn;
    }
};

