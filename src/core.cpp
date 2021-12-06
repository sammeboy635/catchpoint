#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>

#include "settings.h"
#include "core.h"

core::core()
{
    agent_ID = 0;
    if ((corefd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        error("Core: Socket Failed: ");

    //Core Address setup
    struct sockaddr_in coreAddr;

    coreAddr.sin_family = AF_INET;
    coreAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    coreAddr.sin_port = htons(CORE_PORT);

    //Only need for consecutive use to prevent lingering in the background on sig interp
    struct timeval tv;
    tv.tv_sec = CORE_TIME_OUT_SEC;
    tv.tv_usec = 0;
    int optval = 1;
    setsockopt(corefd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    setsockopt(corefd, SOL_SOCKET, SO_REUSEPORT, (const void *)&optval, sizeof(int));
    setsockopt(corefd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    //Bind address for core
    if (bind(corefd, (const struct sockaddr *)&coreAddr, sizeof(coreAddr)) < 0)
        error("Core: Bind Failed: ");
}

//Adds a new network agent to a array of agents
void core::network_new_agent(const char *ip, int port)
{
    struct sockaddr_in agentAddr;
    agentAddr.sin_family = AF_INET;
    agentAddr.sin_addr.s_addr = inet_addr(ip);
    agentAddr.sin_port = htons(port);
    agentAddrLens[agent_ID] = sizeof(agentAddr);
    agentAddrs[agent_ID] = agentAddr;
    agent_ID++;
}

core::~core() //Clean up
{
    close(corefd);
}

//Starting funciton for core;
void core::core_start()
{
    file_process();
}

void core::network_send(struct pkt_agent packet)
{
    sendto(corefd, &packet, sizeof(struct pkt_agent), 0, (struct sockaddr *)&agentAddrs[packet.agent - 1], agentAddrLens[packet.agent - 1]);
}

//Revieces all network packes from agent and processes avg seconds
void core::network_recieve_packets(struct pkt_agent pkt)
{

    struct sockaddr_in agentAddr;
    socklen_t agentAddrLen;
    int i = 0;
    double seconds[AGENT_RUN_COUNT];
    struct pkt_core packet;

    while (true)
    {
        packet.finished = false;
        recvfrom(corefd, &packet, sizeof(struct pkt_core), MSG_WAITALL, (struct sockaddr *)&agentAddr, &agentAddrLen);
        if (packet.finished == true) // Process has completed on agent
        {
            calculate_avg_seconds(seconds, pkt);
            break;
        }
        else //add to array to get processed later.
        {
            seconds[i] = packet.seconds;
            i++;
        }
    }
}
//Kills all agents
void core::network_kill_all_agent()
{
    struct pkt_agent packet;
    packet.agent = -1;
    for (int i = 0; i < 3; i++)
    {
        sleep(1);
        sendto(corefd, &packet, sizeof(struct pkt_agent), 0, (struct sockaddr *)&agentAddrs[i], agentAddrLens[i]);
    }
}
//Reads the file in to get processed one at a time.
void core::file_process()
{
    struct pkt_agent packet;
    FILE *fp = fopen("src/res/config.txt", "r");

    if (fp == NULL)
        error("CORE: FILE ERROR: ");

    while ((fscanf(fp, "%d %s %d", &packet.agent, packet.website, &packet.interval)) == 3)
    {
        network_send(packet);
        network_recieve_packets(packet);
    }

    network_kill_all_agent();
    fclose(fp);
}

//Calculates avg seconds as well as outputs avg seconds.
void core::calculate_avg_seconds(double seconds[AGENT_RUN_COUNT], struct pkt_agent pkt)
{
    double avg = 0;
    int successes = 0;

    for (int i = 0; i < AGENT_RUN_COUNT; i++)
    {
        if (seconds[i] != 0) // If 0 then curl recieved no results.
        {
            avg += seconds[i];
            successes++;
        }
    }

    if (avg == 0) //Prevent / by zero
    {
        printf("%s %.10f (%d runs)\n", pkt.website, avg, successes);
    }
    else
    {
        avg = avg / successes;
        printf("%s %.10f (%d runs)\n", pkt.website, avg, successes);
    }
}
//Main error for core;
void core::error(std::string message)
{
    std::perror(message.data());
    exit(EXIT_FAILURE);
}