#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <regex>
#include <nlohmann/json.hpp>
#include <set>
#include "Socket.h"
#include "exceptions.h"
#include "etcinfo.h"
#include "DBConnection.h"
#include "Campaign.h"
#include "ParsedAgent.h"
#include "ParsedConfBridge.h"
#include "HttpClient.h"

#ifndef AGENT
#define AGENT

using json = nlohmann::json;

class Agent
{

public:
    Agent() {}
    ~Agent() {}

    // Getters
    u_long getAgentId() const { return agentId; }
    u_long getServerId() const { return serverId; }
    std::string getBridgeUId() const { return bridgeUId; }
    u_long getBridgeId() const { return bridgeId; }
    int isOnline() const { return online; }
    int isAvailable() const { return available; }
    int isPaused() const { return pause; }
    std::string getAgentChannelId() const { return agentChannelId; }
    std::string getAgentChannel() const { return agentChannel; }
    std::time_t getUpdatedAt() const { return updatedAt; }

    // Setters (if needed)
    void setAgentId(u_long id) { agentId = id; }
    void setServerId(u_long id) { serverId = id; }
    void setBridgeUId(const std::string& uid) { bridgeUId = uid; }
    void setBridgeId(u_long id) { bridgeId = id; }
    void setOnline(int status) { online = status; }
    void setAvailable(int status) { available = status; }
    void setPause(int status) { pause = status; }
    void setAgentChannelId(const std::string& channelId) { agentChannelId = channelId; }
    void setAgentChannel(const std::string& channel) { agentChannel = channel; }
    void setUpdatedAt(std::time_t time) { updatedAt = time; }
    
private:
    u_long agentId;
    u_long serverId;
    std::string bridgeUId;
    u_long bridgeId;
    int online;
    int available;
    int pause;
    std::string agentChannelId;
    std::string agentChannel;
    std::time_t updatedAt;
};

class AgentList
{

public:
    AgentList() {}
    ~AgentList() {}

private:
    std::vector<Agent> ItsAgents;
};

#endif