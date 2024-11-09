#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <memory>
#include <string>
#include <vector>       
#include <stdexcept>
#include "Campaign.h"
#include "ParsedAgent.h"
#include "ParsedConfBridge.h"

class DBConnection {
public:
    DBConnection();
    ~DBConnection();

    std::shared_ptr<sql::Connection> getConnection();
    std::vector<std::string> getCampaigns(u_long serverId);
    Campaign getCampaignByName(const std::string& name);
    std::vector<std::string> getCampaignSettings(u_long campaigId, u_long serverId);
    std::vector<std::string> getCampaignFilters(u_long campaignId, u_long serverId);
    std::vector<u_long> getCampaignAgents(u_long campaignId, u_long serverId);
    std::vector<ParsedAgent> getAllAgents(u_long serverId);
    u_long getQueueIdByCode(const std::string& campaignCode, u_long serverId);
    u_long getConfBridgeIdForAgent(u_long agentId, u_long serverId);
    std::vector<ParsedConfBridge> getAllConfBridges(u_long serverId);
    std::shared_ptr<sql::PreparedStatement> prepareStatement(const std::string &query)
    {
        return std::shared_ptr<sql::PreparedStatement>(conn->prepareStatement(query));
    }

private:
    sql::mysql::MySQL_Driver* driver;
    std::shared_ptr<sql::Connection> conn;
    std::string host;
    std::string port;
    std::string user;
    std::string password;
    std::string database;

    void connect();
};

#endif // DBCONNECTION_H
