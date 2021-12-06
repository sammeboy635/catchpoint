//Started @10:27am
//Ended @2:35pm

#include <unistd.h>
#include <stdio.h>
#include <curl/curl.h>
#include "settings.h"
#include "core.h"
#include "agent.h"

void client(int agentid) //Init agent object
{
    agent cli(agentid);
    cli.agent_start();
}

void server() //Init core object
{
    core server;
    server.network_new_agent("127.0.0.1", 8081);
    server.network_new_agent("127.0.0.1", 8082);
    server.network_new_agent("127.0.0.1", 8083);
    server.core_start();
}

int main(int argc, char const *argv[])
{
    if (argc > 1)
    {
        client(atoi(argv[1]));
    }
    else
    {
        printf("Starting CORE:\n");
        sleep(1);
        server();
    }
    exit(0);
}
