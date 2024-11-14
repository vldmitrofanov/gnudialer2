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
#include <ctime>
#include <iomanip>
#include <sstream>
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
                "SELECT id, bridge_id, agent_id, online, available, pause, updated_at FROM conf_bridges WHERE server_id = ?"));

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

std::vector<ParsedConfBridge> DBConnection::getQueueConfBridges(u_long serverId, std::string &queueName)
{
    std::vector<ParsedConfBridge> confBridges;

    try
    {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "SELECT "
                "conf_bridges.id, "
                "conf_bridges.bridge_id, "
                "conf_bridges.agent_id, "
                "conf_bridges.online, "
                "conf_bridges.available, "
                "conf_bridges.pause, "
                "conf_bridges.agent_channel_id, "
                "conf_bridges.agent_channel, "
                "conf_bridges.updated_at "
                "FROM conf_bridges "
                "LEFT JOIN agents ON conf_bridges.agent_id=agents.id "
                "LEFT JOIN agent_queue ON agent_queue.agent_id=agents.id "
                "LEFT JOIN queues ON queues.queue_id=queues.id "
                "LEFT JOIN campaigns ON queues.campaign_id=campaigns.id AND queues.server_id = ?"
                "WHERE campaigns.code = ?"));

        pstmt->setUInt64(1, serverId); // Setting the first parameter as serverId
        pstmt->setString(2, queueName);
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

u_long DBConnection::getConfBridgeIdForAgent(u_long agentId, u_long serverId)
{
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "SELECT id FROM conf_bridges WHERE agent_id = ? AND server_id = ?"));
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

ParsedQueueOperations DBConnection::fetchAbnStats(const std::string &queueName, u_long serverId, int resetHours)
{
    ParsedQueueOperations result = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, std::time_t(0)};
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "SELECT "
                "queue_operations.id, "
                "queue_operations.calls, "
                "queue_operations.totalcalls, "
                "queue_operations.abandons, "
                "queue_operations.totalabandons, "
                "queue_operations.disconnects, "
                "queue_operations.noanswers, "
                "queue_operations.busies, "
                "queue_operations.congestions, "
                "queue_operations.ansmachs, "
                "queue_operations.lines_dialing, "
                "queue_operations.queue_id, "
                "queue_operations.updated_at "
                "FROM queue_operations "
                "LEFT JOIN queues ON queue_operations.queue_id=queues.id "
                "LEFT JOIN campaigns ON queues.campaign_id = campaigns.id "
                "WHERE campaigns.code = ? AND queues.server_id = ?"));
        pstmt->setString(1, queueName);
        pstmt->setUInt64(2, serverId);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next())
        {
            result.id = res->getUInt64("id");
            result.calls = res->getInt("calls");
            result.totalcalls = res->getInt("totalcalls");
            result.abandons = res->getInt("abandons");
            result.totalabandons = res->getInt("totalabandons");
            result.disconnects = res->getInt("disconnects");
            result.noanswers = res->getInt("noanswers");
            result.busies = res->getInt("busies");
            result.congestions = res->getInt("congestions");
            result.ansmachs = res->getInt("ansmachs");
            result.lines_dialing = res->getInt("lines_dialing");
            result.queue_id = res->getUInt64("queue_id");

            std::string updatedAtStr = res->getString("updated_at");
            std::tm tm = {};
            std::istringstream ss(updatedAtStr);
            if (!(ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S")))
            {
                std::cerr << "Failed to parse updated_at: " << updatedAtStr << std::endl;
            }
            else
            {
                result.updated_at = std::mktime(&tm);
            }

            time_t now = time(nullptr);
            double hoursDiff = std::difftime(now, result.updated_at) / 3600.0;

            // Check if `updated_at` is too old
            if (hoursDiff >= resetHours)
            {
                // Reset values and update database
                result.calls = 0;
                result.abandons = 0;
                result.disconnects = 0;
                result.noanswers = 0;
                result.busies = 0;
                result.congestions = 0;
                result.ansmachs = 0;
                result.lines_dialing = 0;

                std::shared_ptr<sql::PreparedStatement> resetStmt(
                    conn->prepareStatement(
                        "UPDATE queue_operations SET calls = 0, totalcalls = ?, abandons = 0, "
                        "totalabandons = ?, disconnects = 0, noanswers = 0, busies = 0, "
                        "congestions = 0, ansmachs = 0, lines_dialing = 0, updated_at = NOW() "
                        "WHERE id = ?"));
                resetStmt->setInt(1, result.totalcalls);
                resetStmt->setInt(2, result.totalabandons);
                resetStmt->setUInt64(3, result.id);
                resetStmt->executeUpdate();
            }
        }
        else
        {
            // If no record exists, insert a new entry
            std::shared_ptr<sql::PreparedStatement> insertStmt(
                conn->prepareStatement(
                    "INSERT INTO queue_operations (calls, totalcalls, abandons, totalabandons, "
                    "disconnects, noanswers, busies, congestions, ansmachs, lines_dialing, "
                    "queue_id, updated_at, created_at) "
                    "VALUES (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "
                    "(SELECT id FROM queues WHERE server_id = ? AND campaign_id = "
                    "(SELECT id FROM campaigns WHERE code = ?)), NOW(), NOW())"));
            insertStmt->setUInt64(1, serverId);
            insertStmt->setString(2, queueName);
            insertStmt->executeUpdate();

            // You may want to retrieve the inserted row for `result` if needed
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return result;
}

bool DBConnection::updateAbnStats(const std::string &queueName, u_long serverId, const ParsedQueueOperations &queueOperations)
{
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "UPDATE queue_operations "
                "JOIN queues ON queue_operations.queue_id = queues.id "
                "JOIN campaigns ON queues.campaign_id = campaigns.id "
                "SET queue_operations.calls = ?, queue_operations.totalcalls = ?, queue_operations.abandons = ?, queue_operations.totalabandons = ?, "
                "queue_operations.disconnects = ?, queue_operations.noanswers = ?, queue_operations.busies = ?, queue_operations.congestions = ?, "
                "queue_operations.ansmachs = ?, queue_operations.updated_at = NOW() "
                "WHERE campaigns.code = ? AND queues.server_id = ?"));

        pstmt->setInt(1, queueOperations.calls ? queueOperations.calls : 0);
        pstmt->setInt(2, queueOperations.totalcalls ? queueOperations.totalcalls : 0);
        pstmt->setInt(3, queueOperations.abandons ? queueOperations.abandons : 0);
        pstmt->setInt(4, queueOperations.totalabandons ? queueOperations.totalabandons : 0);
        pstmt->setInt(5, queueOperations.disconnects ? queueOperations.disconnects : 0);
        pstmt->setInt(6, queueOperations.noanswers ? queueOperations.noanswers : 0);
        pstmt->setInt(7, queueOperations.busies ? queueOperations.busies : 0);
        pstmt->setInt(8, queueOperations.congestions ? queueOperations.congestions : 0);
        pstmt->setInt(9, queueOperations.ansmachs ? queueOperations.ansmachs : 0);
        pstmt->setString(10, queueName);
        pstmt->setUInt64(11, serverId);

        pstmt->executeUpdate();
        return true;
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return false;
    }
}

int DBConnection::getAvailableAgentBridges(const std::string &queueName, u_long serverId)
{
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "SELECT count(*) FROM conf_bridges "
                "LEFT JOIN agent_queue ON conf_bridges.agent_id = agent_queue.agent_id "
                "LEFT JOIN queues ON agent_queue.queue_id = queues.id "
                "LEFT JOIN campaigns ON queues.campaign_id = campaigns.id "
                "WHERE campaigns.code = ? AND conf_bridges.server_id = ? "
                "AND online = 1 AND available = 1 AND pause = 1"));
        pstmt->setString(1, queueName);
        pstmt->setUInt64(2, serverId); // Setting the first parameter as serverId
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        // Check if the result has a row and retrieve the count
        if (res->next())
        {
            return res->getInt(1); // Fetch the count from the first column
        }
        else
        {
            return 0; // No rows matched, return 0 as count
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return -1; // Return -1 or another error code to indicate a failure
    }
}

std::vector<ParsedCall> DBConnection::fetchAllCalls(u_long serverId)
{
    std::vector<ParsedCall> calls;
    try
    {
        // Prepare the SQL statement
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT phone, campaign, leadid, callerid, usecloser, dspmode, trunk, dialprefix, transfer, timeout, api_id, called, answered "
                                   "FROM placed_calls WHERE server_id = ?"));
        pstmt->setUInt64(1, serverId);

        // Execute the query
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        // Loop through the result set and create Call objects
        while (res->next())
        {
            ParsedCall parsedCall{
                .phone = res->getString("phone"),
                .campaign = res->getString("campaign"),
                .leadid = res->getUInt64("leadid"),
                .callerid = res->getString("callerid"),
                .usecloser = res->getBoolean("usecloser") ? "true" : "false",
                .dspmode = res->getString("dspmode"),
                .trunk = res->getString("trunk"),
                .dialprefix = res->getString("dialprefix"),
                .transfer = res->getString("transfer"),
                .timeout = static_cast<unsigned short int>(res->getUInt("timeout")),
                .server_id = res->getUInt64("server_id"),
                .called = res->getBoolean("called"),
                .answered = res->getBoolean("answered")};
            calls.push_back(parsedCall);

            // Create a Call object and add it to the vector
            // Call call(phone, campaign, leadid, callerid, usecloser, dspmode, trunk, dialprefix, transfer, timeout);
            // call.SetCalled(called);     // Assuming Call has a method to set 'called'
            // call.SetAnswered(answered); // Assuming Call has a method to set 'answered'
            // calls.push_back(call);
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    return calls;
}

ParsedCall DBConnection::insertCall(const ParsedCall &call) {
    ParsedCall result = call;  // Start with a copy of the original ParsedCall
    try {
        // Prepare the INSERT statement
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "INSERT INTO placed_calls (phone, campaign, leadid, callerid, usecloser, dspmode, "
                "trunk, dialprefix, transfer, timeout, server_id, called, answered) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

        // Bind values to the prepared statement
        pstmt->setString(1, call.phone);
        pstmt->setString(2, call.campaign);
        pstmt->setUInt64(3, call.leadid);
        pstmt->setString(4, call.callerid);
        pstmt->setBoolean(5, call.usecloser);
        pstmt->setString(6, call.dspmode);
        pstmt->setString(7, call.trunk);
        pstmt->setString(8, call.dialprefix);
        pstmt->setString(9, call.transfer);
        pstmt->setUInt(10, call.timeout);
        pstmt->setUInt64(11, call.server_id);
        pstmt->setBoolean(12, call.called);
        pstmt->setBoolean(13, call.answered);

        // Execute the insertion
        pstmt->executeUpdate();

        // Retrieve the newly inserted `id` and `placed_at` timestamp
        std::shared_ptr<sql::PreparedStatement> selectStmt(
            conn->prepareStatement("SELECT id, placed_at FROM placed_calls WHERE id = LAST_INSERT_ID()"));

        std::unique_ptr<sql::ResultSet> res(selectStmt->executeQuery());
        if (res->next()) {
            result.id = res->getString("id");  // Assuming id is an unsigned 64-bit integer
        }
    } catch (const sql::SQLException &e) {
        std::cerr << "SQL error in insertCall: " << e.what() << std::endl;
    }

    return result;
}

bool DBConnection::updateCall(const ParsedCall &call)
{
    try {
        // Prepare the INSERT statement
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "UPDATE placed_calls SET called = ?, answered = ? "
                "WHERE campaign = ? AND leadid = ? and server_id = ?"));

        // Bind values to the prepared statement
        pstmt->setBoolean(1, call.called);
        pstmt->setBoolean(2, call.answered);
        pstmt->setString(3, call.campaign);
        pstmt->setUInt64(4, call.leadid);
        pstmt->setUInt64(5, call.server_id);
        

        // Execute the insertion
        pstmt->executeUpdate();

        return true;
    } catch (const sql::SQLException &e) {
        std::cerr << "SQL error in insertCall: " << e.what() << std::endl;
        return false;
    }
}

int DBConnection::getLinesDialing(const std::string &queueName, u_long serverId)
{
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT * FROM placed_calls WHERE called = true AND answered = true AND campaign = ? AND server_id = ?;"));
        pstmt->setString(1, queueName);
        pstmt->setUInt64(2, serverId); // Setting the first parameter as serverId
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        // Check if the result has a row and retrieve the count
        if (res->next())
        {
            return res->getInt(1); // Fetch the count from the first column
        }
        else
        {
            return 0; // No rows matched, return 0 as count
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return -1; // Return -1 or another error code to i
    }
}

std::vector<std::pair<std::string, std::string>> DBConnection::selectLeads(const std::string &query)
{
    std::vector<std::pair<std::string, std::string>> leads;
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(query));
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        // Process each row in the result set
        while (res->next())
        {
            std::string id = res->getString("id");
            std::string phone = res->getString("phone");
            leads.emplace_back(id, phone);
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "Error selecting leads from mysql! Did you run --tzpopulate?" << std::endl;
        std::cerr << "SQL error: " << e.what() << std::endl;
    }

    return leads;
}

bool DBConnection::executeUpdate(const std::string &query)
{
    std::cout << "[DBConnection::executeUpdate] " << query << std::endl;
    try
    {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(query));

        int affectedRows = pstmt->executeUpdate(); // For non-SELECT queries
        return affectedRows > 0;
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQL error on executeUpdate: " << e.what() << std::endl;
        return false;
    }
}