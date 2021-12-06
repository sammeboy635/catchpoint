#ifndef __CORE_H_
#define __CORE_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

class core
{
private:
    int corefd, agent_ID;
    struct sockaddr_in agentAddrs[3];
    socklen_t agentAddrLens[3];

public:
    core();
    ~core();
    void core_start();
    void network_send(struct pkt_agent packet);
    void network_recieve_packets(struct pkt_agent pkt);
    void network_new_agent(const char *ip, int port);
    void network_kill_all_agent();
    void file_process();
    void calculate_avg_seconds(double seconds[AGENT_RUN_COUNT], struct pkt_agent pkt);
    void error(std::string message);
};

#endif