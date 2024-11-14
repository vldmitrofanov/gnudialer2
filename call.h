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
		 const std::string &leadid,
		 const std::string &callerid,
		 const std::string &usecloser,
		 const std::string &dspmode,
		 const std::string &trunk,
		 const std::string &dialprefix,
		 const std::string &transfer,
		 const unsigned short int &timeout)
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
		called = false;
		answered = false;
		itsServerId = std::stoul(getServerId());

		timeval tv;
		gettimeofday(&tv, NULL);
		itsTime = tv.tv_sec % 1000000;
	}

	const std::string &GetNumber() const { return itsNumber; }
	const std::string &GetCampaign() const { return itsCampaign; }
	const std::string &GetLeadId() const { return itsLeadId; }
	const std::string &GetCallerId() const { return itsCallerId; }
	const std::string &GetUseCloser() const { return itsUseCloser; }
	const std::string &GetDSPMode() const { return itsDSPMode; }
	const std::string &GetTrunk() const { return itsTrunk; }
	const std::string &GetDialPrefix() const { return itsDialPrefix; }
	const std::string &GetTransfer() const { return itsTransfer; }
	const unsigned long int &GetTime() const { return itsTime; }
	const unsigned short int &GetTimeout() const { return itsTimeout; }
	const u_long &GetServerId() const { return itsServerId; }
	const bool &HasBeenCalled() const { return called; }
	const bool &HasBeenAnswered() const { return answered; }

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
			std::string mainHost = getApiUrl();
			// std::cout << "ARI creds: " <<  TheAsterisk.GetAriHost() << ":" + TheAsterisk.GetAriPort() + " - " + TheAsterisk.GetAriUser() + ":" + TheAsterisk.GetAriPass() << std::endl;
			if (curl)
			{
				std::string url = "http://" + mainHost + ":8000/ari/channels";
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
										 "&callerId=" + itsCampaign + "-" + itsLeadId + "-" + itsUseCloser +
										 "&timeout=" + itos(itsTimeout) +
										 "&variables[__LEADID]=" + itsLeadId +
										 "&variables[__CAMPAIGN]=" + itsCampaign +
										 "&variables[__DSPMODE]=" + itsDSPMode +
										 "&variables[__ISTRANSFER]=" + itsTransfer +
										 "&account=" + itsCampaign;
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
					std::cout << mainHost << ": " + itsCampaign + " - " + itsNumber + " - " + itsLeadId + " - " + itsUseCloser << std::endl;
				}

				// Cleanup
				curl_easy_cleanup(curl);
			}
			if (doColor)
			{
				std::cout << mainHost << neon << ": " + itsCampaign + " - " + itsNumber + " - " + itsLeadId + " - " + itsUseCloser << norm << std::endl;
			}
			else
			{
				std::cout << mainHost << ": " + itsCampaign + " - " + itsNumber + " - " + itsLeadId + " - " + itsUseCloser << std::endl;
			}

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
	std::string itsNumber, itsCampaign, itsLeadId, itsCallerId, itsUseCloser, itsDSPMode, itsTrunk, itsDialPrefix, itsTransfer;
	unsigned long int itsTime;
	unsigned short int itsTimeout;
	u_long itsServerId;
	bool called, answered;
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
				 const std::string &leadid,
				 const std::string &callerid,
				 const std::string &usecloser,
				 const std::string &dspmode,
				 const std::string &trunk,
				 const std::string &dialprefix,
				 const std::string &transfer,
				 const unsigned short int &itsTimeout)
	{

		Call TheCall(phone, campaign, leadid, callerid, usecloser, dspmode, trunk, dialprefix, transfer, itsTimeout);
		itsCalls.push_back(TheCall);
	}

	void SetAnswered(const std::string &campaign, const std::string &leadid)
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

		for (unsigned int i = 0; i < itsCalls.size(); i++)
		{
			if (!itsCalls.at(i).HasBeenCalled())
			{
				try
				{
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
		return ParsedCall{
			call.GetNumber(),
			call.GetCampaign(),
			call.GetLeadId(),
			call.GetCallerId(),
			call.GetUseCloser(),
			call.GetDSPMode(),
			call.GetTrunk(),
			call.GetDialPrefix(),
			call.GetTransfer(),
			call.GetTimeout(),
			call.GetServerId(),  // Assuming `GetApiId()` exists in Call for the API ID
			call.HasBeenCalled(),  // Assuming `IsCalled()` exists in Call
			call.HasBeenAnswered() // Assuming `IsAnswered()` exists in Call
		};
	}

	void loadCallsFromDB()
	{
		u_long serverId = std::stoul(getServerId());
		DBConnection dbConn;
		itsCalls.clear();
		auto calls = dbConn.fetchAllCalls(serverId);

		for (const auto &dbCall : calls)
		{
			Call call(dbCall.number, dbCall.campaign, dbCall.leadid, dbCall.callerid,
					  dbCall.usecloser, dbCall.dspmode, dbCall.trunk, dbCall.dialprefix,
					  dbCall.transfer, dbCall.timeout);
			call.SetCalled(dbCall.called);
			call.SetAnswered(dbCall.answered);
			itsCalls.push_back(call);
		}
	}

	void saveCallToDB(const Call &call)
	{
		DBConnection dbConn;
		ParsedCall parsedCall = convertToParsedCall(call); // Convert Call to ParsedCall
		dbConn.insertCall(parsedCall);
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
