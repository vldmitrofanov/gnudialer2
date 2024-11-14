#ifndef PARSECALL_H
#define PARSECALL_H

#include <string>
#include <ctime>

struct ParsedCall
{
    const std::string number;
    const std::string campaign;
    const std::string leadid;
    const std::string callerid;
    const std::string usecloser;
    const std::string dspmode;
    const std::string trunk;
    const std::string dialprefix;
    const std::string transfer;
    const unsigned short int timeout;
    u_long server_id;
    bool called;
    bool answered;
};
#endif