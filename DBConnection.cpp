#include "DBConnection.h"
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "Campaign.h"
#include "etcinfo.h"
#include "ParsedAgent.h"
#include "ParsedConfBridge.h"

DBConnection::DBConnection()
{
    driver = sql::mysql::get_mysql_driver_instance();
    host = getMySqlHost().c_str();
    port = getMysqlPort().c_str();
    user = getMySqlUser().c_str();
    password = getMySqlPass().c_str();
    database = getDbName().c_str();
    connect();
}

DBConnection::~DBConnection()
{
    if (conn)
    {
        conn->close();
    }
}

void DBConnection::connect()
{
    try
    {
        std::string connectionString = "tcp://" + host + ":" + port;
        conn = std::shared_ptr<sql::Connection>(driver->connect(connectionString, user, password));
        conn->setSchema(database);
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
}

std::shared_ptr<sql::Connection> DBConnection::getConnection()
{
    if (!conn || conn->isClosed())
    {
        connect();
    }
    return conn;
}

std::vector<std::string> DBConnection::getCampaigns(u_long serverId)
{
    std::vector<std::string> campaigns;
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "SELECT campaigns.code "
                "FROM campaigns "
                "JOIN queues ON campaigns.id = queues.campaign_id "
                "WHERE campaigns.status = 1 "
                "AND queues.server_id = ? "
                "AND (queues.dial_method = 6 OR queues.dial_method = 3)"));
        pstmt->setUInt64(1, serverId); 
        std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next())
        {
            std::string parameter = res->getString("code");
            campaigns.push_back(parameter);
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return campaigns;
}

Campaign DBConnection::getCampaignByName(const std::string &name)
{
    Campaign campaign;
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT id, name, code FROM campaigns WHERE code = ? AND status > 0"));
        pstmt->setString(1, name);
        std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next())
        {
            campaign.id = res->getUInt64("id");
            campaign.name = res->getString("name");
            campaign.code = res->getString("code");
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return campaign;
}

std::vector<std::string> DBConnection::getCampaignSettings(u_long campaignId, u_long serverId)
{
    std::vector<std::string> itsSettings;
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "SELECT settings.parameter as parameter, settings.value as value FROM settings LEFT JOIN queues ON settings.queue_id = queues.id  WHERE queues.campaign_id = ? AND queues.server_id = ?"));
        pstmt->setUInt64(1, campaignId);
        pstmt->setUInt64(2, serverId);
        std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next())
        {
            std::string parameter = res->getString("parameter");
            std::string value = res->getString("value");
            std::string tempLine = parameter + ":" + value;
            itsSettings.push_back(tempLine);
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return itsSettings;
}

std::vector<std::string> DBConnection::getCampaignFilters(u_long campaignId, u_long serverId)
{
    std::vector<std::string> itsFilters;
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT filters.filter as filter,filters.enabled as enabled, filters.position as position FROM filters LEFT JOIN queues ON filters.queue_id = queues.id  WHERE queues.campaign_id = ? AND queues.server_id = ? ORDER by filters.position ASC"));
        pstmt->setUInt64(1, campaignId);
        pstmt->setUInt64(2, serverId);
        std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next())
        {
            std::string parameter = res->getString("filter");
            std::string enabledStr = res->getString("enabled");
            std::string enabled = (enabledStr == "1") ? "true" : "false";
            std::string position = res->getString("position");
            itsFilters.push_back("filters:number:" + position + ":enable:" + enabled + ":string:" + parameter);
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return itsFilters;
}

std::vector<u_long> DBConnection::getCampaignAgents(u_long campaignId, u_long serverId)
{
    std::vector<u_long> itsAgents;
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT agent_queue.agent_id as agent_id FROM agent_queue LEFT JOIN queues ON agent_queue.queue_id = queues.id  WHERE queues.campaign_id = ? AND queues.server_id = ?"));
        pstmt->setUInt64(1, campaignId);
        pstmt->setUInt64(2, serverId);
        std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next())
        {
            std::string parameter = res->getString("agent_id");
            itsAgents.push_back(std::stoi(parameter));
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return itsAgents;
}

std::vector<ParsedAgent> DBConnection::getAllAgents(u_long serverId)
{
    std::vector<ParsedAgent> itsAgents;
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT agent_queue.agent_id AS agent_id, agents.password AS password, CONCAT(users.first_name, ' ', users.last_name) AS name FROM agent_queue LEFT JOIN queues ON agent_queue.queue_id = queues.id LEFT JOIN agents ON agent_queue.agent_id = agents.id LEFT JOIN users ON agents.user_id = users.id WHERE queues.server_id = ?"));
        pstmt->setUInt64(1, serverId); // Corrected parameter index
        std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next())
        {
            ParsedAgent agent;
            agent.herNumber = res->getInt("agent_id");
            agent.herName = res->getString("name");
            agent.herPass = res->getString("password");
            itsAgents.push_back(agent);
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return itsAgents;
}

u_long DBConnection::getQueueIdByCode(const std::string &campaignCode, u_long serverId = 1)
{
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT queues.id FROM queues "
                                   "JOIN campaigns ON campaigns.id = queues.campaign_id "
                                   "WHERE campaigns.code = ? AND queues.server_id = ? LIMIT 1"));

        pstmt->setString(1, campaignCode);
        pstmt->setUInt64(2, serverId);

        std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next())
        {
            return res->getUInt64("id");
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return 0; // return 0 if not found
}

std::vector<ParsedConfBridge> DBConnection::getAllConfBridges(u_long serverId)
{
    std::vector<ParsedConfBridge> confBridges;

    try
    {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "SELECT id, agent_id, online, available, pause FROM conf_bridges WHERE server_id = ?"
            ));

        pstmt->setUInt64(1, serverId); // Setting the first parameter as serverId

        // Execute the query
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        // Fetch the results row by row
        while (res->next())
        {
            ParsedConfBridge bridge;
            bridge.id = res->getUInt64("id");             // Bridge ID
            bridge.agent_id = res->getUInt64("agent_id"); // Agent ID
            bridge.online = res->getInt("online");        // Online status
            bridge.available = res->getInt("available");  // Available status
            bridge.pause = res->getInt("pause");          // Pause status

            // Add the bridge to the vector
            confBridges.push_back(bridge);
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "MySQL error: " << e.what() << " (MySQL error code: " << e.getErrorCode() << ")" << std::endl;
    }

    return confBridges;
}

u_long DBConnection::getConfBridgeIdForAgent(u_long agentId, u_long serverId){
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "SELECT id FROM conf_bridges WHERE agent_id = ? AND server_id = ?"
            ));
        pstmt->setUInt64(1, agentId);
        pstmt->setUInt64(2, serverId); // Setting the first parameter as serverId

        // Execute the query
        std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next())
        {
            return res->getUInt64("id");
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return 0; // return 0 if not found
}
