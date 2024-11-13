#ifndef PARSECONFBRIDGE_H
#define PARSECONFBRIDGE_H

#include <string>
#include <ctime>

struct ParsedConfBridge {
    u_long agent_id;
    std::string bridge_id;
    int online;
    int available;
    int pause;
    u_long id;
    std::string agent_channel_id;
    std::string agent_channel;
    std::time_t updated_at;
};
#endif 