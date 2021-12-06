#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <curl/curl.h>

#include "settings.h"
#include "agent.h"

agent::agent(int agentid)
{
    agentID = agentid;
    if ((agentfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        error("Socket Failed");

    //Server Address setup
    coreAddr.sin_family = AF_INET;
    coreAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    coreAddr.sin_port = htons(CORE_PORT);

    coreAddrLen = sizeof(coreAddr);

    //Agent Address setup
    agentAddr.sin_family = AF_INET;
    agentAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    agentAddr.sin_port = htons(AGENT_PORT + agentID - 1);

    //Only need for consecutive use to prevent lingering in the backroung on sig interp
    struct timeval tv;
    tv.tv_sec = AGENT_TIME_OUT_SEC;
    tv.tv_usec = 0;
    int optval = 1;

    setsockopt(agentfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    setsockopt(agentfd, SOL_SOCKET, SO_REUSEPORT, (const void *)&optval, sizeof(int));
    setsockopt(agentfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    //binding IP and port for agent;
    if (bind(agentfd, (const struct sockaddr *)&agentAddr, sizeof(agentAddr)) < 0)
        error("Bind Failed");
}

agent::~agent() //Clean up
{
    close(agentfd);
}
//Starting funciton for agent;
void agent::agent_start()
{
    network_recieve_work();
}

void agent::network_send(struct pkt_core packet)
{
    sendto(agentfd, &packet, sizeof(struct pkt_core), 0, (struct sockaddr *)&coreAddr, coreAddrLen);
}

//Waits for a job and processes it.
void agent::network_recieve_work()
{
    int n;
    struct pkt_agent packet;
    packet.agent = agentID;
    while (packet.agent != -1)
    {
        n = recvfrom(agentfd, &packet, sizeof(struct pkt_agent), MSG_WAITALL, (struct sockaddr *)&coreAddr, &coreAddrLen);
        if ((n > 0) & (packet.agent != -1)) //negative one on agent packet is to shutdown.
        {
            procces_request(packet);
        }
    }
    printf("Agent %d: Exiting\n", agentID);
}

//Main processing for request to do curl work
void agent::procces_request(struct pkt_agent packet)
{
    CURL *curl = curl_easy_init();
    CURLcode res;
    struct pkt_core pkt;

    pkt.agentID = agentID;
    pkt.finished = false;

    for (int i = 0; i < AGENT_RUN_COUNT; i++)
    {

        if (!curl)
            error("Curl Error:");

        double seconds = 0;

        curl_easy_setopt(curl, CURLOPT_URL, packet.website);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, throw_away);
        res = curl_easy_perform(curl);

        if (CURLE_OK == res)
            curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &seconds);

        pkt.seconds = seconds;
        network_send(pkt);
        sleep(packet.interval);
    }
    pkt.seconds = 0;
    pkt.finished = true;
    network_send(pkt);
    curl_easy_cleanup(curl);
}

//agent error handling
void agent::error(std::string message)
{
    printf("AGENT %d: %s\n", agentID, message.data());
    std::perror(message.data());
    exit(EXIT_FAILURE);
}

//Throw away function for curl information.
static size_t throw_away(void *ptr, size_t size, size_t nmemb, void *data)
{
    (void)ptr;
    (void)data;
    return (size_t)(size * nmemb);
}