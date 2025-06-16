#pragma once
#include "httplib.h"
#include "ServicesDef.h"
#include "RequestHandler.h"

class HTTPClient
{
private:

    httplib::Client m_client = httplib::Client("localhost:8080");
    int m_port =0;
    std::string m_host;
    HTTPClient()
    {
        m_host = "localhost"; // Need to implement a logic for a cfg for this
        m_port = 8080; // Need to implement a logic for a cfg for this
        std::string srv = m_host + ":" + std::to_string(m_port);
        m_client.set_keep_alive(true);
    }
public:
    httplib::Result Send_Get(const std::string& service,httplib::Headers headers = httplib::Headers())
    {
         return m_client.Get(service,headers);
    }
    httplib::Result Send_Post(const std::string& service,const std::string& body ="{}",const std::string& content_type = "application/json",const httplib::Headers& headers = httplib::Headers())
    {
        return  m_client.Post(service, headers, body, content_type);
    }
    ~HTTPClient()
    {
        m_client.stop();
    }
    HTTPClient(const HTTPClient&) = delete;
    HTTPClient& operator=(const HTTPClient&) = delete;

    // Static method to get the instance of the Singleton
    static HTTPClient& getInstance() {
        static HTTPClient instance; // Guaranteed to be destroyed and instantiated on first use
        return instance;
    }

};

