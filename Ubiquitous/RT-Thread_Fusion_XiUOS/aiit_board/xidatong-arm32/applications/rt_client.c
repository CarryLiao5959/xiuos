#include <rtthread.h>

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>

#include "rt_client.h"

#define DBG_SECTION_NAME               "RT_CLIENT"
#define DBG_LEVEL                      DBG_INFO
#include <rtdbg.h>

int rt_client(int argc, char **argv)
{
    int sockfd;
    int nbytes;
    struct sockaddr_in addr;
    const char *url;
    int port;
    struct hostent *host;

    char buf[1024] = {0};
    process_info_t info;
    SystemInfo_t sys_info;
    int cmd;

    if (argc < 4)
    {
        rt_kprintf("Usage: rt_client URL PORT cmd\n");
        rt_kprintf("Like: rt_client 192.168.130.52 5000 1\n");
        return -1;
    }

    url = argv[1];
    port = strtoul(argv[2], 0, 10);
    host = gethostbyname(url);
    cmd = atoi(argv[3]); 

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_E("create socket failed!\n");
        return -1;
    }

    addr.sin_family = PF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr= *((struct in_addr *)host->h_addr);
    if (connect(sockfd, (const struct sockaddr *)&addr, sizeof(addr))<0) {
        LOG_E("Connect to Linux server failed, Waiting for the server to open!\n");
        closesocket(sockfd);
        return -1;
    }

    LOG_I("Connect to Linux server successful!");

    switch (cmd)
    {
    case GET_PROCESS_INFO:
        process_info_t info;
        rt_strcpy(buf, "get process info");
        if(send(sockfd, buf, rt_strlen(buf), 0) < 0) {
            rt_kprintf("send failed\n");
            closesocket(sockfd);
            break;
        }

        // recv data
        memset(buf, 0, sizeof(buf));
        nbytes = recv(sockfd, buf, sizeof(buf), 0);
        if(nbytes < 0) {
            printf("recv failed\n");
            break;
        }
        // start
        printf("| %-6s| %-6s| %-20s| %-7s| %-10s|\n", "PID", "PPID", "Name", "State", "Stime");
        printf("------------------------------------------------------------\n");
       
        // parse data
        char *pos = buf;
        while(pos < buf + nbytes) {
            sscanf(pos, "%d %d %s %s %s", &info.pid, &info.ppid, info.name, info.state, info.stime);
            printf("| %-6d| %-6d| %-20s| %-7s| %-10s|\n", info.pid, info.ppid, info.name, info.state, info.stime);

            pos = strchr(pos, '\n');
            if(pos == NULL) {
                break;
            }
            pos++;
        }
        // end
        printf("------------------------------------------------------------\n");
        break;
    
    case GET_SYSTEM_INFO:
        rt_strcpy(buf, "get system info");
        if(send(sockfd, buf, rt_strlen(buf), 0) < 0) {
            rt_kprintf("send failed\n");
            closesocket(sockfd);
            break;
        }
        
        // recv data
        memset(buf, 0, sizeof(buf));
        nbytes = recv(sockfd, buf, sizeof(buf), 0);
        if(nbytes < 0) {
            printf("recv failed\n");
            break;
        }

        // output system info in table format
        printf("----------------------------------------------------------------------------------\n");
        printf("                                   SYSTIME INFO                                   \n");
        printf("----------------------------------------------------------------------------------\n");
        printf("buf:%s",buf);
        printf("----------------------------------------------------------------------------------\n");

        break;
    default:
        break;
    }

    closesocket(sockfd);
    return 0;
}

#ifdef FINSH_USING_MSH
#include <finsh.h>
MSH_CMD_EXPORT(rt_client, the network bandwidth measurement tool);
#endif