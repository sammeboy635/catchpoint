#ifndef __SETTINGS_H_
#define __SETTINGS_H_

//Settings for all the code base

#define CORE_PORT 8080
#define CORE_TIME_OUT_SEC 30

#define AGENT_PORT 8081
#define AGENT_TIME_OUT_SEC 300
#define AGENT_RUN_COUNT 3

struct pkt_core
{
    int agentID;
    double seconds;
    bool finished;
};

struct pkt_agent
{
    int agent, interval;
    char website[50];
};
struct pkt_rec
{
    bool rec;
};

#endif