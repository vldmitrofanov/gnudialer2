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

#include <queue>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <vector>
#include <curl/curl.h>
#include <string>
#include "DBConnection.h"
#include "ParsedCall.h"
#include "exceptions.h"
#include "Socket.h"
#include "itos.h"
#include "agent.h"

#ifndef CALL
#define CALL

const bool doColor = true;
const std::string neon = "\033[1;32m"; // set foreground color to light green
const std::string norm = "\033[0m";	   // reset to system

class Asterisk;

class Call;
std::string workingCampaign;
bool countCallsForCampaign(const Call &);

class Call
{

public:
	Call(const std::string &number,
		 const std::string &campaign,
		 u_long leadid,
		 const std::string &callerid,
		 bool usecloser,
		 const std::string &dspmode,
		 const std::string &trunk,
		 const std::string &dialprefix,
		 const std::string &transfer,
		 const unsigned short int &timeout,
		 bool setCalled,
		 bool setAnswered,
		 u_long serverId)
	{

		itsNumber = number;
		itsCampaign = campaign;
		itsLeadId = leadid;
		itsCallerId = callerid;
		itsUseCloser = usecloser;
		itsDSPMode = dspmode;
		itsTrunk = trunk;
		itsDialPrefix = dialprefix;
		itsTransfer = transfer;
		itsTimeout = timeout;
		called = setCalled;
		answered = setAnswered;
		itsServerId = serverId;

		timeval tv;
		gettimeofday(&tv, NULL);
		itsTime = tv.tv_sec % 1000000;
		//ParsedCall parsedCall = this->saveCallToDB();
		//this->SetId(parsedCall.id);
	}
	const std::string &GetId() const { return itsId; }
	const std::string &GetNumber() const { return itsNumber; }
	const std::string &GetCampaign() const { return itsCampaign; }
	u_long GetLeadId() const { return itsLeadId; }
	const std::string &GetCallerId() const { return itsCallerId; }
	const bool &GetUseCloser() const { return itsUseCloser; }
	const std::string &GetDSPMode() const { return itsDSPMode; }
	const std::string &GetTrunk() const { return itsTrunk; }
	const std::string &GetDialPrefix() const { return itsDialPrefix; }
	const std::string &GetTransfer() const { return itsTransfer; }
	const unsigned long int &GetTime() const { return itsTime; }
	const unsigned short int &GetTimeout() const { return itsTimeout; }
	const u_long GetServerId() const { return itsServerId; }
	const bool &HasBeenCalled() const { return called; }
	const bool &HasBeenAnswered() const { return answered; }

	void SetId(std::string& newId) { itsId = newId; }
	void SetAnswered(bool v = true) { answered = v; }
	void SetCalled(bool v) { answered = v; }
	// TODO move to ARI interface
	void DoCall()
	{

		std::string response;

		called = true;

		signal(SIGCLD, SIG_IGN);

		int pid = fork();

		if (pid == 0)
		{

			usleep(rand() % 2000000);

			CURL *curl;
			CURLcode res;

			curl = curl_easy_init();
			std::string mainHost = getMainHost();
			std::string itsUseCloserStr = itsUseCloser ? "true" : "false"; 
			std::string ariUser = getAriUser();
			std::string ariProto = getAriProto();
			std::string ariPass = getAriPass();
			std::string stasisApp = "gnudialer_stasis_app";
			// std::cout << "ARI creds: " <<  TheAsterisk.GetAriHost() << ":" + TheAsterisk.GetAriPort() + " - " + TheAsterisk.GetAriUser() + ":" + TheAsterisk.GetAriPass() << std::endl;
			if (curl)
			{
				std::string url = ariProto + "://" + mainHost + ":8088/ari/channels?api_key="+ariUser + ":" + ariPass + "&app=" + stasisApp;
				std::string dialPrefix = (itsDialPrefix == "none") ? "" : itsDialPrefix;
				std::string finalNumber = dialPrefix + itsNumber;
				std::cout << "TRUNK: " + itsTrunk << std::endl;
				size_t pos = itsTrunk.find('!');
				while (pos != std::string::npos)
				{
					itsTrunk.replace(pos, 1, ":");
					pos = itsTrunk.find('!', pos + 1);
				}
				pos = itsTrunk.find("_EXTEN_");
				if (pos != std::string::npos)
				{
					// Replace _EXTEN_ with the actual number
					itsTrunk.replace(pos, 7, finalNumber); // 7 is the length of "_EXTEN_"
				}
				else
				{
					throw std::runtime_error("Placeholder _EXTEN_ not found in the trunk string. Trunk example: SIP/faketrunk/sip!_EXTEN_@127.0.0.1!5062");
				}
				
				std::string postFields = "endpoint=" + itsTrunk +
										 "&extension=" + dialPrefix + itsNumber +
										 "&context=" + (itsTransfer == "TRANSFER" ? "gdtransfer" : "gdincoming") +
										 "&priority=1" +
										 "&callerId=" + itsCampaign + "-" + std::to_string(itsLeadId) + "-" + itsUseCloserStr +
										 "&timeout=" + itos(itsTimeout) +
										 "&variables[LEADID]=" + std::to_string(itsLeadId) +
										 "&variables[CAMPAIGN]=" + itsCampaign +
										 "&variables[DSPMODE]=" + itsDSPMode +
										 "&variables[ISTRANSFER]=" + itsTransfer;

				curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
				curl_easy_setopt(curl, CURLOPT_USERNAME, getAriUser().c_str());
				curl_easy_setopt(curl, CURLOPT_PASSWORD, getAriPass().c_str());

				// Perform the request, res will get the return code
				res = curl_easy_perform(curl);
				if (res != CURLE_OK)
				{
					std::string errorMessage = "curl_easy_perform() failed: " + std::string(curl_easy_strerror(res));
					std::cerr << errorMessage << std::endl;
					curl_easy_cleanup(curl);
					throw std::runtime_error(errorMessage);
				}
				else
				{
					std::cout << mainHost << ": " + itsCampaign + " - " + itsNumber + " - " + std::to_string(itsLeadId) + " - " + itsUseCloserStr << std::endl;
				}

				// Cleanup
				curl_easy_cleanup(curl);
			}
			if (doColor)
			{
				std::cout << mainHost << neon << ": " + itsCampaign + " - " + itsNumber + " - " + std::to_string(itsLeadId) + " - " + itsUseCloserStr << norm << std::endl;
			}
			else
			{
				std::cout << mainHost << ": " + itsCampaign + " - " + itsNumber + " - " + std::to_string(itsLeadId) + " - " + itsUseCloserStr << std::endl;
			}
			this->SetCalled(true);
			this->updateCallInDB();

			usleep(10000000);

			exit(0);
		}

		if (pid == -1)
		{

			throw xForkError();
		}
	}

	~Call() {}

private:
	std::string itsId, itsNumber, itsCampaign, itsCallerId, itsDSPMode, itsTrunk, itsDialPrefix, itsTransfer;
	unsigned long int itsTime;
	unsigned short int itsTimeout;
	u_long itsServerId, itsLeadId;
	bool called, answered, itsUseCloser;

	ParsedCall saveCallToDB()
	{
		DBConnection dbConn;
		ParsedCall parsedCall = this->convertToParsedCall(); // Convert Call to ParsedCall
		return dbConn.insertCall(parsedCall);
	}

	void updateCallInDB()
	{
		DBConnection dbConn;
		ParsedCall parsedCall = this->convertToParsedCall(); // Convert Call to ParsedCall
		dbConn.updateCall(parsedCall);
	}

	ParsedCall convertToParsedCall()
	{
		ParsedCall parsedCall;
		parsedCall.id = this->GetId();
		parsedCall.phone = this->GetNumber();
		parsedCall.campaign = this->GetCampaign();
		parsedCall.leadid = this->GetLeadId();
		parsedCall.callerid = this->GetCallerId();
		parsedCall.usecloser = this->GetUseCloser();
		parsedCall.dspmode = this->GetDSPMode();
		parsedCall.trunk = this->GetTrunk();
		parsedCall.dialprefix = this->GetDialPrefix();
		parsedCall.transfer = this->GetTransfer();
		parsedCall.timeout = this->GetTimeout();
		parsedCall.server_id = this->GetServerId(); // assuming GetServerId() returns api_id
		parsedCall.called = this->HasBeenCalled();
		parsedCall.answered = this->HasBeenAnswered();

		return parsedCall;
	}
};

class CallCache
{

public:
	CallCache()
	{
		loadCallsFromDB();
	}
	~CallCache() {}

	void push_back(const Call &TheCall)
	{

		itsCalls.push_back(TheCall);
	}

	void AddCall(const std::string &phone,
				 const std::string &campaign,
				 u_long leadid,
				 const std::string &callerid,
				 bool usecloser,
				 const std::string &dspmode,
				 const std::string &trunk,
				 const std::string &dialprefix,
				 const std::string &transfer,
				 const unsigned short int &itsTimeout)
	{

		Call TheCall(phone, campaign, leadid, callerid, usecloser, dspmode, trunk, dialprefix, transfer, itsTimeout, false, false, std::stoul(getServerId()));
		ParsedCall parsedCall = saveCallToDB(TheCall);
		TheCall.SetId(parsedCall.id);
		itsCalls.push_back(TheCall);
	}

	void SetAnswered(const std::string &campaign, u_long leadid)
	{

		for (unsigned int i = 0; i < itsCalls.size(); i++)
		{
			if (itsCalls.at(i).GetCampaign() == campaign && itsCalls.at(i).GetLeadId() == leadid)
			{
				itsCalls.at(i).SetAnswered();
			}
		}
	}

	unsigned int LinesDialing_OBSOLETE(const std::string &campaign)
	{

		workingCampaign = campaign;

		timeval tv;
		gettimeofday(&tv, NULL);

		for (unsigned long int cur = tv.tv_sec % 1000000; itsCalls.size() && cur - itsCalls.front().GetTime() >
																				 itsCalls.front().GetTimeout() / 1000;)
		{

			itsCalls.pop_front();
		}

		return std::count_if(itsCalls.begin(), itsCalls.end(), countCallsForCampaign);
	}

	unsigned int LinesDialing(const std::string &campaign)
	{
		DBConnection dbConn;
		u_long serverId = std::stoul(getServerId());
		return dbConn.getLinesDialing(campaign, serverId);
	}

	void CallAll()
	{
		std::cout << "CalAll SIZE: " << itsCalls.size() << std::endl;
		for (unsigned int i = 0; i < itsCalls.size(); i++)
		{
			if (!itsCalls.at(i).HasBeenCalled())
			{
				try
				{					
					itsCalls.at(i).SetCalled(true);
					itsCalls.at(i).DoCall();					
				}
				catch (const xConnectionError &e)
				{
					std::cerr << "Exception: Unable to connect to " << e.GetHost() << "! Disabling host." << std::endl;
				}
			}
		}
	}

private:
	std::deque<Call> itsCalls;

	ParsedCall convertToParsedCall(const Call &call)
	{
		ParsedCall parsedCall;
		parsedCall.id = call.GetId();
		parsedCall.phone = call.GetNumber();
		parsedCall.campaign = call.GetCampaign();
		parsedCall.leadid = call.GetLeadId();
		parsedCall.callerid = call.GetCallerId();
		parsedCall.usecloser = call.GetUseCloser();
		parsedCall.dspmode = call.GetDSPMode();
		parsedCall.trunk = call.GetTrunk();
		parsedCall.dialprefix = call.GetDialPrefix();
		parsedCall.transfer = call.GetTransfer();
		parsedCall.timeout = call.GetTimeout();
		parsedCall.server_id = call.GetServerId(); // assuming GetServerId() returns api_id
		parsedCall.called = call.HasBeenCalled();
		parsedCall.answered = call.HasBeenAnswered();

		return parsedCall;
	}

	void loadCallsFromDB()
	{
		u_long serverId = std::stoul(getServerId());
		DBConnection dbConn;
		itsCalls.clear();
		auto calls = dbConn.fetchAllCalls(serverId);

		for (const auto &dbCall : calls)
		{
			Call call(dbCall.phone, dbCall.campaign, dbCall.leadid, dbCall.callerid,
					  dbCall.usecloser, dbCall.dspmode, dbCall.trunk, dbCall.dialprefix,
					  dbCall.transfer, dbCall.timeout, dbCall.called, dbCall.answered, dbCall.server_id);
			itsCalls.push_back(call);
		}
	}

	ParsedCall saveCallToDB(const Call &call)
	{
		DBConnection dbConn;
		ParsedCall parsedCall = convertToParsedCall(call); // Convert Call to ParsedCall
		return dbConn.insertCall(parsedCall);
	}

	void updateCallInDB(const Call &call)
	{
		DBConnection dbConn;
		ParsedCall parsedCall = convertToParsedCall(call); // Convert Call to ParsedCall
		dbConn.updateCall(parsedCall);
	}
};

bool countCallsForCampaign(const Call &TheCall)
{

	if (TheCall.GetCampaign() == workingCampaign && TheCall.HasBeenAnswered() == false)
	{

		return true;
	}
	else
	{

		return false;
	}
}

#endif
