#ifndef __AGENT_H_
#define __AGENT_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <curl/curl.h>

class agent
{
private:
    int agentfd, agentID;
    struct sockaddr_in coreAddr, agentAddr;
    socklen_t coreAddrLen;

public:
    agent(int agentid);
    ~agent();
    void agent_start();
    void network_send(struct pkt_core packet);
    void network_recieve_work();
    void procces_request(struct pkt_agent packet);
    void error(std::string message);
};

static size_t throw_away(void *ptr, size_t size, size_t nmemb, void *data);

#endif