#ifndef PARSEQUEUEOPERATIONS_H
#define PARSEQUEUEOPERATIONS_H

#include <string>
#include <ctime>

struct ParsedQueueOperations {
    u_long id;
    int calls;
    int totalcalls;
    int abandons;
    int totalabandons;
    int disconnects;
    int noanswers;
    int busies;
    int congestions;
    int ansmachs;
	int lines_dialing;
    u_long queue_id;
};
#endif