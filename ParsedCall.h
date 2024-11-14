#ifndef PARSECALL_H
#define PARSECALL_H

#include <string>
#include <ctime>

struct ParsedCall
{
    std::string id;
    std::string phone;
    std::string campaign;
    u_long leadid;
    std::string callerid;
    std::string usecloser;
    std::string dspmode;
    std::string trunk;
    std::string dialprefix;
    std::string transfer;
    unsigned short int timeout;
    u_long server_id;
    bool called;
    bool answered;
};
#endif