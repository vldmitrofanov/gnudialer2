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
#include <ctime>
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
    static constexpr int STATUS_AVAILABLE = 1;
    static constexpr int STATUS_ONCALL = -1;
    static constexpr int STATUS_OFFLINE = -2;
    static constexpr int STATUS_ONPAUSE = -3;
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

    const u_long &GetNumber() const { return agentId; }

    const long int &GetStatus() const
	{
		if(online==0){
            return STATUS_OFFLINE;
        } else if(online==1 && pause == 1) {
            return STATUS_ONPAUSE;
        } else if(online==1 && pause == 0 && available == 0) {
            return STATUS_ONCALL;
        }  else if(online==1 && pause == 0 && available == 0) {
            return STATUS_AVAILABLE;
        }
        return 0;
	}

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

    const int size() const { return ItsAgents.size(); }
	const Agent &at(const int &whichAgent) const { return ItsAgents.at(whichAgent); }

	bool exists(const int &agentNum)
	{
		//	Agent TheAgent;
		for (unsigned int i = 0; i < ItsAgents.size(); i++)
		{
			if (agentNum == ItsAgents.at(i).GetNumber())
			{
				return true;
			}
		}
		return false;
	}

    void ParseAgentList(std::string queueName)
	{

		//	std::cout << "Got to beginning of ParseAgentList" << std::endl;
		DBConnection dbConn;
		u_long serverId = std::stoull(getServerId());
		std::vector<ParsedConfBridge> agents = dbConn.getQueueConfBridges(serverId, queueName);

		if (agents.empty())
		{
			std::cout << "No agents found in the database." << std::endl;
		}
		else
		{
			std::cout << "agent.h Agents found: " << agents.size() << " ServerID: " << serverId << std::endl;
		}

		for (const auto &agent : agents)
		{
			Agent TempAgent = ReturnAgent(agent);
			std::cout << " - Agent ID: " << TempAgent.GetNumber() << std::endl;
			int tempAgentNum = TempAgent.GetNumber();
			if (!this->exists(tempAgentNum))
			{
				ItsAgents.push_back(TempAgent);
			}
		}

		std::stable_sort(ItsAgents.begin(), ItsAgents.end());
		//ParseAgentChannelStatus();
		//	std::cout << "Got to end of ParseAgentList" << std::endl;
	}

    void Initialize(std::string queueName)
	{
		ParseAgentQueueStatus(queueName);
		// Since "show queue" doesn't show us current call status of the agent we'll get info via ARI
		//ParseAgentChannelStatus();
	}

private:
    std::vector<Agent> ItsAgents;
};

#endif