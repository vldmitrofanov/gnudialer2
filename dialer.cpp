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
#include "Socket.h"
#include "asterisk.h"
#include "evaluate.h"
#include "etcinfo.h"
#include "color.h"
#include "HttpClient.h"
#include "ConfigSingleton.h"
#include "ParsedConfBridge.h"
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
    u_long serverId = getServerId()
    usleep(100000);
    safeMode = false;
    bool daemonMode = true;
    MYSQL_RES *queryResult;
    MYSQL_ROW queryRow;

    signal(SIGCHLD, sig_handler);
    signal(SIGSEGV, sig_handler);

    // set default console color to white on black
    if (doColorize)
    {
        std::cout << fg_light_white << std::endl;
    }

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
    // main loop
    while (true)
    {
        std::cout << "=====+++++++++++++++=====" << std::endl;
        // Get all campaigns

        // 
        usleep(100000);
    }
}