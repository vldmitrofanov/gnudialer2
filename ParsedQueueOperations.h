#ifndef PARSEQUEUEOPERATIONS_H
#define PARSEQUEUEOPERATIONS_H

#include <string>
#include <ctime>

struct ParsedQueueOperations
{
    u_long id;
    int calls = 0;
    int totalcalls = 0;
    int abandons = 0;
    int totalabandons = 0;
    int disconnects = 0;
    int noanswers = 0;
    int busies = 0;
    int congestions = 0;
    int ansmachs = 0;
    int lines_dialing = 0;
    u_long queue_id;
    std::time_t updated_at;
};
#endif