#include <iomanip>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <queue>
#include <algorithm>
#include <sys/time.h>
#include <sys/stat.h>
#include <mysql.h>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include "HttpClient.h"
#include "ConfigSingleton.h"
#include "DBConnection.h"
#include "Socket.h"
// #include "asterisk.h"
#include "ParsedConfBridge.h"
#include "evaluate.h"
#include "etcinfo.h"
#include "color.h"
#include "queue.h"
#include "call.h"
#include "log.h"
#include "isholiday.h"
#include "tzfilter.h"

#define CRASH                                                          \
    do                                                                 \
    {                                                                  \
        fprintf(stderr, "!! Forcing immediate crash a-la abort !!\n"); \
        *((int *)0) = 0;                                               \
    } while (0)

#ifdef DEBUG
bool gDebug = true;
const bool debugPos = true;
#else
bool gDebug = false;
const bool debugPos = false;
#endif

bool safeMode;

void sig_handler(int sig)
{

    if (sig == SIGSEGV)
    {
        if (safeMode == true)
        {
            if (doColorize)
            {
                std::cout << fg_light_red << "FATAL ERROR! Segmentation Fault!" << normal << std::endl;
            }
            else
            {
                std::cout << "FATAL ERROR! Segmentation Fault!" << std::endl;
            }
            std::cout << "You are running in safe mode, so GnuDialer will attempt to restart itself!" << std::endl
                      << std::endl;
            std::system("sleep 1 && gnudialer &");
        }
        else
        {
            CRASH;
            if (doColorize)
            {
                std::cout << fg_light_red << "FATAL ERROR! Segmentation Fault!" << normal << std::endl;
            }
            else
            {
                std::cout << "FATAL ERROR! Segmentation Fault!" << std::endl;
            }
            std::cout << "Please report this to the GnuDialer project." << std::endl;
            std::cout << "Please also be advised that you can start gnudialer in \"safe\" mode which will" << std::endl;
            std::cout << "automatically restart GnuDialer if you receive a fatal error." << std::endl;
            std::cout << "Type: \"gnudialer --help\" for more information." << std::endl
                      << std::endl;
        }
        exit(0);
    }

    if (sig == SIGCHLD)
    {
        int stat;
        while (waitpid(-1, &stat, WNOHANG) > 0)
            ;
    }
    return;
}

int main(int argc, char **argv)
{
    u_long serverId = std::stoul(getServerId());
    usleep(100000);
    safeMode = false;
    bool daemonMode = true;

    signal(SIGCHLD, sig_handler);
    signal(SIGSEGV, sig_handler);

    // set default console color to white on black
    // if (doColorize)
    //{
    //    std::cout << fg_light_white << std::endl;
    //}

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg(argv[i]);

        if (arg == "stop" ||
            arg == "-stop" ||
            arg == "--stop")
        {
            writeGnudialerLog("GnuDialer: Stopped");
            if (doColorize)
            {
                std::cout << fg_light_green << "GnuDialer: Stopped" << normal << std::endl;
            }
            else
            {
                std::cout << "GnuDialer: Stopped" << std::endl;
            }
            std::system(("killall " + std::string(argv[0])).c_str());
            exit(0);
        }
    }

    // TODO: Move this into loop
    bool isAHoliday;
    try
    {
        isAHoliday = isHoliday();
    }
    catch (const xFileOpenError &e)
    {
        std::cerr << "GnuDialer: Exception! Unable to open " << e.GetFilename() << "!" << std::endl;
        return 1;
    }

    // main loop
    while (true)
    {
        std::cout << "=====+++++++++++++++=====" << std::endl;
        // Get all campaigns
        QueueList TheQueues;
        TheQueues.ParseQueues();
        for (const Queue &queue : TheQueues)
        {
            std::string response, block, queueName, mode, calltoday, usednc, query, tzearliest, tzlatest, callerid, channel;
            std::string tempagent, usecloser, closercam;
            std::string dspmode, trunk, dialprefix, transfer, filter, usecallback, usetzfilter;
            bool debug;
            int skip;
            std::string f_areacode, f_areacode_prefix, f_zipcode, orderby;
            unsigned int maxlines = 0, timeout = 0, linesdialing = 0, availagents = 0, linestodial = 0, counter = 0;
            unsigned long int calls = 0, abandons = 0;
            int pid = 0;
            double maxratio = 0.0, maxabandons = 0.0;
            queue.DisplayInfo();
            queueName = queue.GetName();
            if (queue.GetSettingByName("active").Get() != "true")
            {
                std::cout << "Skipping inactive campaign " << queueName << std::endl;
                continue;
            }
            if (queueName == "CLOSER")
            {
                std::cout << "Skipping closer campaign " << queueName << std::endl;
                continue;
            }
            CallCache *TheCallCache = new CallCache();
            maxratio = queue.GetSettingByName("maxratio").GetFloat();
            maxlines = queue.GetSettingByName("maxlines").GetInt();
            maxabandons = queue.GetSettingByName("maxabandons").GetFloat();
            mode = queue.GetSettingByName("function").Get();
            calltoday = queue.GetSettingByName("calltoday").Get();
            usednc = queue.GetSettingByName("usednc").Get();
            callerid = queue.GetSettingByName("callerid").Get();
            filter = queue.GetSettingByName("filter").Get();
            timeout = queue.GetSettingByName("timeout").GetInt();
            usecloser = queue.GetSettingByName("usecloser").Get();
            closercam = queue.GetSettingByName("closercam").Get();
            dspmode = queue.GetSettingByName("dspmode").Get();
            trunk = queue.GetSettingByName("trunk").Get();
            dialprefix = queue.GetSettingByName("dialprefix").Get();
            usecallback = queue.GetSettingByName("usecallback").Get();
            usetzfilter = queue.GetSettingByName("usetzfilter").Get();
            debug = queue.GetSettingByName("debug").GetBool();
            skip = queue.GetSettingByName("skip").GetInt();
            f_areacode = queue.GetSettingByName("f_areacode").Get();
            f_areacode_prefix = queue.GetSettingByName("f_areacode_prefix").Get();
            f_zipcode = queue.GetSettingByName("f_zipcode").Get();
            orderby = queue.GetSettingByName("orderby").Get();

            calls = atoi(queue.GetCalls().c_str());
            abandons = atoi(queue.GetAbandons().c_str());
            linesdialing = TheCallCache->LinesDialing(queueName);
            availagents = queue.GetAvailAgents();
            linestodial = evaluate(mode, linesdialing, availagents, maxratio, maxlines, maxabandons, calls, abandons);
            unsigned int remaininglines = maxlines - linestodial;

            if (linestodial >= 5000 || remaininglines >= 5000)
            {
                if (doColorize)
                {
                    std::cout << fg_red << queueName << ": linestodial or remaininglines are greater than 5000, something is WRONG!" << normal << std::endl;
                }
                else
                {
                    std::cout << queueName << ": linestodial or remaininglines are greater than 5000, something is WRONG!" << std::endl;
                }
                std::cout << queueName << ": ldg: " << linesdialing << " aa: " << availagents << " mr: " << maxratio << " ml: " << maxlines << " ma: " << maxabandons << " mode: " << mode << " calls: " << calls << " abs: " << abandons << " l2d: " << linestodial << " rls: " << remaininglines << std::endl;
            }
            if (debug)
            {
                std::cout << "[DEBUG]" << queueName << ": ldg: " << linesdialing << " aa: " << availagents << " mr: " << maxratio << " ml: " << maxlines << " ma: " << maxabandons << " mode: " << mode << " calls: " << calls << " abs: " << abandons << " l2d: " << linestodial << " rls: " << remaininglines << std::endl;
            }

            if (linestodial && mode != "closer" && mode != "inbound" && true)
            {
                if (debug)
                {
                    if (calls != 0)
                    {
                        if (doColorize)
                        {
                            std::cout << std::setprecision(4) << queueName << fg_light_cyan << ": ABANDON (percentage): " << static_cast<double>(abandons) / static_cast<double>(calls) * 100.0 << normal << std::endl;
                        }
                        else
                        {
                            std::cout << std::setprecision(4) << queueName << ": ABANDON (percentage): " << static_cast<double>(abandons) / static_cast<double>(calls) * 100.0 << std::endl;
                        }
                    }
                    else
                    { // No div by 0 allowed!
                        if (doColorize)
                        {
                            std::cout << fg_light_cyan << "ABANDON (percentage): 0" << normal << std::endl;
                        }
                        else
                        {
                            std::cout << queueName << ": ABANDON (percentage): 0" << std::endl;
                        }
                    }
                }

                // this is just a base to get the building of the query string going
                query = "SELECT id, phone FROM campaign_" + queueName + " WHERE 1 ";

                if (calltoday != "true")
                {
                    query += " AND (LEFT(lastupdated,10) = LEFT(NOW(),10) AND disposition = 1) OR LEFT(lastupdated,10) <> LEFT(NOW(),10) ";
                }

                if (filter.empty() == false && filter != "0" && filter != "None" && filter != "none")
                {
                    if (debug)
                    {
                        std::cout << queueName << ": filter - " << filter << std::endl;
                    }
                    query += " AND " + filter;
                }

                for (int x = 0; x < queue.OccurencesOf("filters"); x++)
                {
                    std::string fnum, fstring, enabled;
                    fnum = queue.GetSettingByIndexAndName(x, "filters").GetAttribute("number");
                    fstring = queue.GetSettingByIndexAndName(x, "filters").GetAttribute("string");
                    enabled = queue.GetSettingByIndexAndName(x, "filters").GetAttribute("enable");
                    if (enabled == "true")
                    {
                        if (debug)
                        {
                            std::cout << queueName << ": filter - " << fstring << std::endl;
                        }
                        query += " AND " + fstring;
                    }
                }

                if (f_areacode.empty() == false && f_areacode != "0")
                {
                    if (debug)
                    {
                        std::cout << queueName << ": f_areacode - " << f_areacode << std::endl;
                    }
                    query += " AND LEFT(phone,3)='" + f_areacode + "'";
                }

                if (f_areacode_prefix.empty() == false && f_areacode_prefix != "0")
                {
                    if (debug)
                    {
                        std::cout << queueName << ": f_areacode_prefix - " << f_areacode_prefix << std::endl;
                    }
                    query += " AND LEFT(phone,6)='" + f_areacode_prefix + "'";
                }

                if (f_zipcode.empty() == false && f_zipcode != "0")
                {
                    if (debug)
                    {
                        std::cout << queueName << ": f_zipcode - " << f_zipcode << std::endl;
                    }
                    query += " AND LEFT(zip,5)='" + f_zipcode + "'";
                }

                if (usetzfilter == "true")
                {
                    tzearliest = queue.GetSettingByName("tzearliest").Get();
                    tzlatest = queue.GetSettingByName("tzlatest").Get();

                    query += " AND " + getFilter(tzearliest, tzlatest, isAHoliday);

                    if (debug)
                    {
                        if (doColorize)
                        {
                            std::cout << queueName << fg_green << ": tzFilter Enabled " << normal << std::endl;
                        }
                        else
                        {
                            std::cout << queueName << ": tzFilter Enabled " << std::endl;
                        }
                    }
                }
                else
                {
                    if (debug)
                    {
                        if (doColorize)
                        {
                            std::cout << queueName << fg_red << ": tzFilter Disabled " << normal << std::endl;
                        }
                        else
                        {
                            std::cout << queueName << ": tzFilter Disabled " << std::endl;
                        }
                    }
                }

                if (usednc == "true")
                {
                    // query += " AND phone NOT IN (SELECT phone FROM DNC) ";
                }

                // query += " ORDER BY attempts + pickups ASC LIMIT " + itos(skip) + "," + itos(linestodial);
                query += " AND NOT EXISTS (SELECT 1 FROM DNC WHERE DNC.phone = campaign_" + queueName + ".phone)";

                if (orderby == "id" || orderby == "phone")
                {
                    if (orderby == "id")
                    {
                        query += " ORDER BY id ASC ";
                    }
                    if (orderby == "phone")
                    {
                        query += " ORDER BY phone ASC ";
                    }
                }
                else
                {
                    query += " ORDER BY attempts + pickups ASC ";
                }

                query += " LIMIT " + itos(skip) + "," + itos(linestodial);

                if (debug)
                {
                    std::cout << queueName << ": query - " << query << std::endl;
                }

                if (debug)
                {
                    std::cout << queueName << ": Dialing " << linestodial << " calls (" << skip << ") skipped" << std::endl;
                }
                DBConnection dbConn;
                std::vector<std::pair<std::string, std::string>> leads = dbConn.selectLeads(query);
                if (leads.empty())
                {
                    if (doColorize)
                    {
                        std::cout << queueName << fg_light_red << "has ran out of leads! (CHECK YOUR FILTERS!!!)" << normal << std::endl;
                    }
                    else
                    {
                        std::cout << queueName << ": has ran out of leads! (CHECK YOUR FILTERS!!!)" << std::endl;
                    }
                    usleep(10000);
                }
                else
                {
                    // Updating selected leads
                    query = "UPDATE campaign_" + queueName + " SET attempts=attempts+1 WHERE id IN(";
                    counter = leads.size();
                    int line = 0;
                    for (const auto &lead : leads)
                    {
                        std::cout << "ID (first) " << lead.first << " PHONE (second) " << lead.second << std::endl;
                        query += lead.first;
                        if (line > 0 && line < counter - 1)
                        {
                            query += ", ";
                        }
                        TheCallCache->AddCall(lead.second, queueName, lead.first, callerid, usecloser, dspmode, trunk, dialprefix, transfer, timeout);
                        line++;
                    }
                    query += ")";
                    if (counter < linestodial)
                    {
                        std::cerr << queueName << " is running very low on leads!" << std::endl;
                    }

                    if (counter)
                    {
                        TheQueues.rWhere(queueName).AddCallsDialed(counter);
                        TheQueues.rWhere(queueName).WriteCalls();
                        if (!dbConn.executeUpdate(query))
                        {
                            std::cerr << "Error updating leads in mysql!" << std::endl;
                            return 1;
                        }
                    }
                }
            }
        }

        usleep(100000);
    }
}