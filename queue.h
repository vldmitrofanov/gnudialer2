/*
 * GnuDialer - Complete, free predictive dialer
 *
 * Complete, free predictive dialer for contact centers.
 *
 * Copyright (C) 2006, GnuDialer Project
 *
 * Heath Schultz <heath1444@yahoo.com>
 * Richard Lyman <richard@dynx.net>
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License.
 */

#include <algorithm>
#include <string>
#include <vector>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <time.h>
#include "agent.h"
#include "call.h"
#include "abnhelper.h"
#include "exceptions.h"
#include "DBConnection.h"
#include "Campaign.h"

#ifndef QUEUE
#define QUEUE

class Setting
{

public:
	Setting(const std::string &rawSetting)
	{
		std::stringstream SettingStream;
		SettingStream << rawSetting; // tempSetting;
		for (std::string tempLine; std::getline(SettingStream, tempLine, ':');)
		{
			itsWords.push_back(tempLine);
		}
	}

	void Set(const std::string &rawSetting)
	{

		std::vector<std::string> tempWords;
		// format: ;:setting:asdf:jklsemicolon:qwerty
		std::string tempSetting = rawSetting; //.substr(10, rawSetting.length() - 10);
		std::stringstream SettingStream;
		SettingStream << tempSetting;
		for (std::string tempLine; std::getline(SettingStream, tempLine, ':');)
		{
			tempWords.push_back(tempLine);
		}
		itsWords = tempWords;
	}

	int WordCount() const { return itsWords.size() - 1; }
	const std::string &GetType() const { return itsWords.at(0); }
	const std::string &GetWord(int whichWord) const { return itsWords.at(whichWord + 1); }
	// took out const and and string
	// std::string & GetAttribute(std::string & attribute) {
	std::string GetAttribute(const std::string &attribute)
	{

		notFound = "Not Found";
		for (unsigned int i = 0; i < itsWords.size(); i++)
		{
			if (itsWords.at(i) == attribute)
			{
				return itsWords.at(i + 1);
			}
		}
		return notFound;
	}

	const bool AttributeExists(const std::string &attribute)
	{

		for (std::vector<std::string>::iterator it = itsWords.begin(); it != itsWords.end(); it++)
		{
			if ((*it) == attribute)
			{
				return true;
			}
		}
		return false;
	}

	const bool AttributeExists(const std::string &field, const std::string &value)
	{

		for (unsigned int i = 0; i < itsWords.size() - 1; i++)
		{
			if (itsWords.at(i) == field && itsWords.at(i + 1) == value)
			{
				return true;
			}
		}
		return false;
	}

	int SetAttribute(const std::string &field, const std::string &value)
	{

		unsigned int counter = 0, indexOfValue = 0;
		while (counter < itsWords.size())
		{
			if (itsWords.at(counter) == field)
			{
				indexOfValue = counter + 1;
			}
			counter++;
		}

		if (indexOfValue)
		{
			if (indexOfValue < itsWords.size())
			{
				itsWords.at(indexOfValue) = value;
				return 1;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			return 0;
		}
	}

	int AddAttribute(const std::string &field, const std::string &value)
	{

		if (field.empty() == false && value.empty() == false)
		{
			itsWords.push_back(field);
			itsWords.push_back(value);
			return 1;
		}
		else
		{
			return -1;
		}
	}

	int SupAttribute(const std::string &field, const std::string &value)
	{

		int res = SetAttribute(field, value);

		if (res == -1)
		{
			return -1;
		}
		else
		{
			if (res == 0)
			{
				if (AddAttribute(field, value) == 1)
				{
					return 2;
				}
				else
				{
					return -1;
				}
			}
			else
			{
				return 1;
			}
		}
	}

	void DelAttribute(const std::string &field)
	{

		std::vector<std::string> tempWords;

		for (unsigned int i = 0; i < itsWords.size(); i++)
		{
			if (itsWords.at(i) != field)
			{
				tempWords.push_back(itsWords.at(i));
			}
			else
			{
				i++;
			}
		}
		itsWords = tempWords;
	}

	std::string GetAttr()
	{
		return std::string("hello");
	}

	// const std::string & GetWord(const std::string & type, int whichWord) {
	std::string GetWord(const std::string &type, int whichWord)
	{

		notFound = "Not Found";

		for (unsigned int i = 0; i < itsWords.size(); i++)
		{
			if (itsWords.at(i) == type)
			{
				return GetWord(whichWord);
			}
		}
		return notFound;
	}

	const std::string &Get() const { return itsWords.at(1); }
	const int GetInt() const { return atoi(itsWords.at(1).c_str()); }
	double GetFloat() const { return atof(itsWords.at(1).c_str()); }
	bool GetBool() const
	{

		if (itsWords.at(1) == "true")
			return true;
		else if (itsWords.at(1) == "false")
			return false;
		else if (itsWords.at(1) == "0")
			return true;
		else
			return false;
	}

	void print() const
	{
		std::cout << "queue.h SETTINGS ";
		for (const auto &word : itsWords)
		{
			std::cout << word << "_:_";
		}
		std::cout << std::endl;
	}

	~Setting() {}

private:
	std::vector<std::string> itsWords;
	std::string notFound;
};

const Setting ReturnSetting(const std::string &rawSetting)
{
	Setting TheSetting(rawSetting);
	return TheSetting;
}

class Queue
{

public:
	Queue() { changed = false; }
	~Queue() {}

	// bool ParseQueue(std::string name, AgentList TheAgents) {
	bool ParseQueue(const std::string &name, u_long serverId)
	{
		DBConnection dbConn;
		itsName = name;
		int pos = 0, tempMemberNumber = 0;

		if (itsName != "general")
		{
			itsAbnHelper.Read(name);
		}
		else
		{
			itsId = 0;
			itsSettings.push_back(ReturnSetting("debug:true"));
			itsSettings.push_back(ReturnSetting("log:true"));
			return true;
		}
		Campaign campaign = dbConn.getCampaignByName(name);

		if (campaign.id > 0)
		{
			// std::cout << "queue.h::Campaign found successfully!" << std::endl;
			// std::cout << "queue.h::Campaign ID: " << campaign.id << std::endl;
			// std::cout << "queue.h::Campaign Name: " << campaign.name << std::endl;
			std::cout << "queue.h::Campaign Code: " << campaign.code << std::endl;
		}
		else
		{
			std::cout << "queue.h::Campaign not found." << std::endl;
		}

		std::ifstream QueueIn;

		std::vector<std::string> settings = dbConn.getCampaignSettings(campaign.id, serverId);

		for (const auto &setting : settings)
		{
			// std::cout << "Added setting for campaign " << name << " setting: " << setting << std::endl;
			itsSettings.push_back(ReturnSetting(setting));
		}

		std::vector<std::string> filters = dbConn.getCampaignFilters(campaign.id, serverId);

		for (const auto &filter : filters)
		{
			// std::cout << "queue.h::Added filter for campaign " << name << " STRING: " << filter << std::endl;
			itsSettings.push_back(ReturnSetting(filter));
		}

		std::vector<u_long> agents = dbConn.getCampaignAgents(campaign.id, serverId);

		for (const auto &agent : agents)
		{
			// std::cout << "queue.h::Added agent for campaign " << name << " agent: " << agent << std::endl;
			itsMembersNumbers.push_back(agent);
			std::cout << name << ": itsMembersNumbers size - " << itsMembersNumbers.size() << std::endl;
		}

		return true;
	}

	void WriteAbn()
	{
		itsAbnHelper.Write(itsName);
	}

	const std::string &GetCalls() const { return itsAbnHelper.GetCalls(); }
	const std::string &GetTotalCalls() const { return itsAbnHelper.GetTotalCalls(); }
	const std::string &GetAbandons() const { return itsAbnHelper.GetAbandons(); }
	const std::string &GetTotalAbandons() const { return itsAbnHelper.GetTotalAbandons(); }
	const std::string &GetDateString() const { return itsAbnHelper.GetDateString(); }
	const std::string &GetDisconnects() const { return itsAbnHelper.GetDisconnects(); }
	const std::string &GetNoanswers() const { return itsAbnHelper.GetNoanswers(); }
	const std::string &GetBusies() const { return itsAbnHelper.GetBusies(); }
	const std::string &GetCongestions() const { return itsAbnHelper.GetCongestions(); }

	int OccurencesOf(const std::string &type) const
	{

		int occurences = 0;
		for (unsigned int i = 0; i < itsSettings.size(); i++)
		{
			if (itsSettings.at(i).GetType() == type)
			{
				occurences++;
			}
		}
		return occurences;
	}

	void AddTalkTime(const long int &theTime)
	{
		SetSetting("talktime", itos(GetSetting("talktime").GetInt() + theTime));
	}

	int SettingCount() const { return itsSettings.size(); }
	Setting GetSetting(int whichSetting) const { return itsSettings.at(whichSetting); }
	// took const out after the func
	Setting &GetSetting(const int &whichSetting, const std::string &type)
	{

		int occurence = 0;
		for (unsigned int i = 0; i < itsSettings.size(); i++)
		{
			// std::cout << "Checking setting type " << type << " at index " << i << " against type " << itsSettings.at(i).GetType() << std::endl;
			if (itsSettings.at(i).GetType() == type)
			{
				occurence++;
				// std::cout << "Occurrence " << occurence << " of type " << type << std::endl;
			}
			if (occurence == whichSetting + 1)
			{
				// std::cout << "Returning setting at index " << i << " for occurrence " << whichSetting << std::endl;
				return itsSettings.at(i);
			}
		}

		// throw(xLoopEnd("Exception thrown: Didn't find a setting at " + itos(whichSetting)));
		throw std::runtime_error("Exception thrown: Didn't find a setting at index " + std::to_string(whichSetting) + " for type " + type);
	}

	const bool DelSetting(const int &whichSetting, const std::string &type)
	{

		int occurence = 0;
		std::vector<Setting> tempSettings;

		for (unsigned int i = 0; i < itsSettings.size(); i++)
		{
			if (itsSettings.at(i).GetType() == type)
			{
				occurence++;
			}
			if (occurence != whichSetting + 1)
			{
				tempSettings.push_back(itsSettings.at(i));
			}
		}

		if (itsSettings.size() == tempSettings.size())
		{
			return false;
		}
		else
		{
			itsSettings = tempSettings;
			return true;
		}
	}

	const bool DelSetting(const std::string &type)
	{

		std::vector<Setting> tempSettings;

		for (std::vector<Setting>::iterator it = itsSettings.begin(); it != itsSettings.end(); it++)
		{
			if (it->GetType() != type)
			{
				tempSettings.push_back((*it));
			}
		}

		if (itsSettings.size() == tempSettings.size())
		{
			return false;
		}
		else
		{
			itsSettings = tempSettings;
			return true;
		}
	}

	int GetSettingNumber(const int &whichSetting, const std::string &type) const
	{

		int occurence = 0;
		for (unsigned int i = 0; i < itsSettings.size(); i++)
		{
			if (itsSettings.at(i).GetType() == type)
			{
				occurence++;
			}
			if (occurence == whichSetting + 1)
			{
				return i;
			}
		}

		throw(xLoopEnd("Exception thrown: Didn't find any setting at " + itos(whichSetting)));
	}

	// took const out afterward here as well

	const bool SettingExists(const std::string &type, const std::string &field, const std::string &value)
	{

		for (unsigned int i = 0; i < itsSettings.size(); i++)
		{
			if (itsSettings.at(i).GetType() == type)
			{
				if (itsSettings.at(i).AttributeExists(field, value))
				{
					return true;
				}
			}
		}

		return false;
	}

	void DelSetting(const std::string &type, const std::string &field, const std::string &value)
	{

		std::vector<Setting> tempSettings;

		for (std::vector<Setting>::iterator it = itsSettings.begin(); it != itsSettings.end(); it++)
		{
			if (it->GetType() != type)
			{
				tempSettings.push_back((*it));
			}
			else
			{
				if (!it->AttributeExists(field, value))
				{
					tempSettings.push_back((*it));
				}
			}
		}

		itsSettings = tempSettings;
	}

	const Setting &GetSetting(const std::string &type) { return GetSetting(0, type); }

	int GetSettingNumber(const std::string &type) { return GetSettingNumber(0, type); }

	void SetSetting(const std::string &type, const std::string &rawSetting)
	{
		// itsSettings.at(GetSettingNumber(type)).Set(";:setting:" + type + ":" + rawSetting);
		itsSettings.at(GetSettingNumber(type)).Set(type + ":" + rawSetting);
		changed = true;
	}

	void AddSetting(const std::string &type, const std::string &settingString)
	{
		// itsSettings.push_back(ReturnSetting(";:setting:" + type + ":" + settingString));
		itsSettings.push_back(ReturnSetting(type + ":" + settingString));
	}

	void SupSetting(const std::string &type, const std::string &settingString)
	{
		bool exists = false;
		for (unsigned int i = 0; i < itsSettings.size(); i++)
		{
			if (itsSettings.at(i).GetType() == type && exists == false)
			{
				exists = true;
			}
		}
		if (!exists)
		{
			AddSetting(type, settingString);
			std::cout << type << " not found in settings" << std::endl;
			std::cout << type << ":" << settingString << " setting up" << std::endl;
		}
	}

	// void SetName(std::string name) { itsName = name; }
	std::string GetName() const { return itsName; }

	int GetMemberNumber(int whichMember) const { return itsMembersNumbers.at(whichMember); }
	std::string GetMemberName(int whichMember) const { return itsMembersNames.at(whichMember); }

	bool HasMemberNumber(int whichMember) const
	{
		for (unsigned int i = 0; i < itsMembersNumbers.size(); i++)
		{
			if (itsMembersNumbers.at(i) == whichMember)
			{
				return true;
			}
		}
		return false;
	}

	void dumpAllAgents()
	{

		 std::cout << "[DEBUG}[dumpAllAgents] Member List: " << std::endl;
			for (int i = 0; i < itsMembersNumbers.size(); i++) {
				std::cout << itsMembersNumbers.at(i) << std::endl;
			}

		std::cout << "Number of Agents: " << std::endl;
		std::cout << itsMembersNumbers.size() << std::endl;
	}

	void IncrementDisconnects()
	{
		itsAbnHelper.IncrementDisconnects();
	}
	void IncrementNoanswers()
	{
		itsAbnHelper.IncrementNoanswers();
	}
	void IncrementBusies()
	{
		itsAbnHelper.IncrementBusies();
	}
	void IncrementCongestions()
	{
		itsAbnHelper.IncrementCongestions();
	}
	void IncrementAnsmachs()
	{
		itsAbnHelper.IncrementAnsmachs();
	}
	void DecrementAnsmachs()
	{
		itsAbnHelper.DecrementAnsmachs();
	}

	void IncrementAbandons()
	{
		itsAbnHelper.IncrementAbandons();
	}

	void WriteCalls()
	{
		itsAbnHelper.Write(itsName);
	}

	void AddCallsDialed(const int &calls)
	{
		itsAbnHelper.AddCallsDialed(calls);
	}

	int size() const { return itsMembersNumbers.size(); }

	
	std::string join(const std::vector<std::string>& elements, const std::string& delimiter)
{
    std::string result;
    for (size_t i = 0; i < elements.size(); ++i)
    {
        result += elements[i];
        if (i != elements.size() - 1)  // Don't add the delimiter after the last element
        {
            result += delimiter;
        }
    }
    return result;
}

	void Write()
	{
		DBConnection dbConn;
		std::string serverIdStr = getServerId();   // Assuming getServerId() returns std::string
		u_long serverId = std::stoul(serverIdStr); // Convert the string to u_long
		u_long queueId = dbConn.getQueueIdByCode(itsName, serverId);
		if (queueId == 0)
		{
			std::cout << "queue.h Write: Campaign not found!" << std::endl;
			return;
		}

		for (unsigned int i = 0; i < itsSettings.size(); i++)
		{
			std::string settingType = itsSettings.at(i).GetType();
			std::vector<std::string> settingWords;
			for (int j = 0; j < itsSettings.at(i).WordCount(); j++)
			{
				settingWords.push_back(itsSettings.at(i).GetWord(j));
			}
			std::string settingValue = join(settingWords, ":"); // assuming a join function

			try
			{
				std::shared_ptr<sql::PreparedStatement> pstmt(
					dbConn.prepareStatement("INSERT INTO settings (queue_id, parameter, value) "
										   "VALUES (?, ?, ?) "
										   "ON DUPLICATE KEY UPDATE value = VALUES(value)"));
				pstmt->setUInt64(1, queueId);
				pstmt->setString(2, settingType);
				pstmt->setString(3, settingValue);
				pstmt->executeUpdate();
			}
			catch (sql::SQLException &e)
			{
				std::cerr << "SQLException: " << e.what() << std::endl;
				std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
				std::cerr << "SQLState: " << e.getSQLState() << std::endl;
			}
		}
	}

	int GetAvailAgents(const AgentList &TheAgents) const
	{

		timeval tv;
		gettimeofday(&tv, NULL);

		int availAgents = 0;

		int theAgent, theAgentStatus;

		std::cout << "Members: ";

		for (int i = 0; i < TheAgents.size(); i++)
		{

			theAgent = TheAgents.at(i).GetNumber();
			theAgentStatus = TheAgents.at(i).GetStatus();
			if (HasMemberNumber(theAgent))
			{
				std::cout << "(" << theAgent << ":" << theAgentStatus << ":" << (tv.tv_sec % 1000000 - theAgentStatus) << ")";
			}

			if (TheAgents.at(i).GetStatus() > 0 && tv.tv_sec % 1000000 - TheAgents.at(i).GetStatus() > 5)
			{
				if (HasMemberNumber(TheAgents.at(i).GetNumber()))
				{

					std::cout << "(AVAIL:" << theAgent << ")";

					availAgents++;
				}
			}
		}

		std::cout << std::endl;

		return availAgents;
	}

	void printSettings() const
	{
		std::cout << "Queue settings:" << std::endl;
		for (const auto &setting : itsSettings)
		{
			setting.print();
		}
	}

private:
	u_long itsId;
	std::string itsName;
	std::vector<int> itsMembersNumbers;
	std::vector<std::string> itsMembersNames, otherSettings;
	std::vector<Setting> itsSettings;
	bool changed;
	CallCache itsCalls;
	AbnHelper itsAbnHelper;
	u_long serverId;
};

Queue ReturnQueue(std::string name)
{
	Queue TheQueue;
	TheQueue.ParseQueue(name, 1);
	return TheQueue;
}

class QueueList
{

public:
	QueueList() {}
	~QueueList() {}

	void ParseQueues()
	{

		//	std::cout << "Got here beginning of ParseQueues" << std::endl;

		ItsQueues.clear();
		//	TheAgents.ParseAgentList();

		std::vector<std::string> queueNames;

		DBConnection dbConn;
		u_long serverId = std::stoull(getServerId());
		queueNames = dbConn.getCampaigns(serverId);

		std::stable_sort(queueNames.begin(), queueNames.end());

		for (unsigned int i = 0; i < queueNames.size(); i++)
		{
			//		std::cout << "pushnig back" << std::endl;
			ItsQueues.push_back(ReturnQueue(queueNames.at(i)));
		}

		//	std::cout << "Got to end of ParseQueues" << std::endl;
		std::cout << "size of queues " << ItsQueues.size() << std::endl;
	}

	Queue at(int whichQueue) const { return ItsQueues.at(whichQueue); }
	int size() { return ItsQueues.size(); }

	Queue where(const std::string &queue)
	{
		for (unsigned int i = 0; i < ItsQueues.size(); i++)
		{
			if (ItsQueues.at(i).GetName() == queue)
			{
				return ItsQueues.at(i);
			}
		}

		throw(xLoopEnd("Exception thrown: Didn't find a queue with the name " + queue));
	}

	std::string LeastRecent(std::string name, AgentList &TheAgents)
	{

		// dumpAllAgents();

		Queue tempQueue;
		for (unsigned int i = 0; i < ItsQueues.size(); i++)
		{
			if (name == ItsQueues.at(i).GetName())
			{
				tempQueue = ItsQueues.at(i);
				break;
			}
		}

		timeval tv;
		gettimeofday(&tv, NULL);

		int lowestTime = 999999, agentNum = 0;
		int tempAgentStatus, tempAgentNum = 0;
		std::string sStatus;

		// this is all fluff, just display only, sets nothing
		for (int i = 0; i < TheAgents.size(); i++)
		{
			std::cout << "[DEBUG] LeastRecent [" << name << "]: ALL AGENTS: " << TheAgents.at(i).GetNumber() << " STATUS: "<<  TheAgents.at(i).GetStatus() << std::endl;
			if (tempQueue.HasMemberNumber(TheAgents.at(i).GetNumber()) && TheAgents.at(i).GetStatus() != -2)
			{
				tempAgentStatus = TheAgents.at(i).GetStatus();
				tempAgentNum = TheAgents.at(i).GetNumber();

				std::cout << name << ": Online Agent: " << tempAgentNum << " Status: " << tempAgentStatus << " Waiting: " << tv.tv_sec % 1000000 - tempAgentStatus << std::endl;
			}
		}

		tempQueue.dumpAllAgents();

		// actual loop looking for available agents
		for (int i = 0; i < TheAgents.size(); i++)
		{
			tempAgentNum = TheAgents.at(i).GetNumber();
			tempAgentStatus = TheAgents.at(i).GetStatus();

			if (tempQueue.HasMemberNumber(tempAgentNum))
			{
				// the > x, is equiv to wrapuptime
				if (tempAgentStatus < lowestTime && tv.tv_sec % 1000000 - tempAgentStatus > 2 && tempAgentStatus > 0)
				{
					lowestTime = tempAgentStatus;
					agentNum = tempAgentNum;
					std::cout << "[DEBUG] queue.h " << name << ": Selecting Available Agent: " << agentNum << " lowest time: " << lowestTime << std::endl;
				}
			}
		}

		std::ostringstream AgentStream;
		AgentStream << agentNum;

		return AgentStream.str();
	}

	Queue &rWhere(const std::string &queue)
	{
		for (unsigned int i = 0; i < ItsQueues.size(); i++)
		{
			if (ItsQueues.at(i).GetName() == queue)
			{
				return ItsQueues.at(i);
			}
		}

		throw(xLoopEnd("Exception thrown: Didn't find a queue with the name: " + queue));
	}

	bool exists(const std::string &name)
	{
		for (unsigned int i = 0; i < ItsQueues.size(); i++)
		{
			if (ItsQueues.at(i).GetName() == name)
			{
				return true;
			}
		}
		return false;
	}

private:
	std::vector<Queue> ItsQueues;
	// AgentList TheAgents;
};

#endif
