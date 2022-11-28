/*
* Copyright (c) 2022 AIIT XUOS Lab
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
* @file lwip_udp_socket_demo.c
* @brief UDP demo based on LwIP
* @version 1.0
* @author AIIT XUOS Lab
* @date 2022-03-21
*/
#include <transform.h>

#ifdef ADD_XIZI_FETURES
#include "sys_arch.h"
#include "lwip/sockets.h"
#endif

#ifdef ADD_NUTTX_FETURES
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

#define LWIP_DEMO_TIMES 3
#define LWIP_LOCAL_PORT 4840

#define lw_error printf
#define lw_notice printf
#define lw_print printf
#endif

#define UDP_BUF_SIZE 65536

char udp_socket_ip[] = {192, 168, 250, 252};
char udp_ip_str[128] = {0};
uint16_t udp_socket_port = LWIP_LOCAL_PORT;

/*****************************************************************************/
void UdpSocketConfigParam(char *ip_str)
{
    int ip1, ip2, ip3, ip4, port = 0;

    if(ip_str == NULL)
    {
        return;
    }

    if(sscanf(ip_str, "%d.%d.%d.%d:%d", &ip1, &ip2, &ip3, &ip4, &port))
    {
        printf("config ip %s port %d\n", ip_str, port);
        strcpy(udp_ip_str, ip_str);
        udp_socket_ip[0] = ip1;
        udp_socket_ip[1] = ip2;
        udp_socket_ip[2] = ip3;
        udp_socket_ip[3] = ip4;
        if(port)
            udp_socket_port = port;
        return;
    }

    if(sscanf(ip_str, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4))
    {
        printf("config ip %s\n", ip_str);
        udp_socket_ip[0] = ip1;
        udp_socket_ip[1] = ip2;
        udp_socket_ip[2] = ip3;
        udp_socket_ip[3] = ip4;
        strcpy(udp_ip_str, ip_str);
    }
}

static void UdpSocketRecvTask(void *arg)
{
    int fd = -1;
    char *recv_buf;
    struct sockaddr_in udp_addr, server_addr;
    int recv_len;

    while(1)
    {
        recv_buf = (char *)malloc(UDP_BUF_SIZE);
        if(recv_buf == NULL)
        {
            lw_error("No memory\n");
            continue;
        }

        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if(fd < 0)
        {
            lw_error("Socket error\n");
            free(recv_buf);
            continue;
        }

        udp_addr.sin_family = AF_INET;
        udp_addr.sin_addr.s_addr = INADDR_ANY;
        udp_addr.sin_port = htons(udp_socket_port);
        memset(&(udp_addr.sin_zero), 0, sizeof(udp_addr.sin_zero));

        if(bind(fd, (struct sockaddr *)&udp_addr, sizeof(struct sockaddr)) == -1)
        {
            lw_error("Unable to bind\n");
            close(fd);
            free(recv_buf);
            continue;
        }

        lw_notice("UDP bind success, start to receive.\n");
        lw_notice("\n\nLocal Port:%d\n\n", udp_socket_port);

        while(1)
        {
            memset(recv_buf, 0, UDP_BUF_SIZE);
            recv_len = recv(fd, recv_buf, UDP_BUF_SIZE, 0);
            if(recv_len > 0)
            {
                lw_notice("Receive from : %s\n", inet_ntoa(server_addr.sin_addr));
                lw_notice("Receive data : %s\n\n", recv_buf);
            }
            send(fd, recv_buf, recv_len, 0);
        }

        close(fd);
        free(recv_buf);
    }
}

void UdpSocketRecvTest(int argc, char *argv[])
{
    if(argc >= 2)
    {
        lw_notice("lw: [%s] target ip %s\n", __func__, argv[1]);
        UdpSocketConfigParam(argv[1]);
    }

#ifdef ADD_XIZI_FETURES
    lwip_config_tcp(lwip_ipaddr, lwip_netmask, udp_socket_ip);
    sys_thread_new("UdpSocketRecvTask", UdpSocketRecvTask, NULL,
        LWIP_TASK_STACK_SIZE, LWIP_DEMO_TASK_PRIO);
#endif
#ifdef ADD_NUTTX_FETURES
    UdpSocketRecvTask(NULL);
#endif
}
PRIV_SHELL_CMD_FUNCTION(UdpSocketRecvTest, a udp receive sample, PRIV_SHELL_CMD_MAIN_ATTR);

static void UdpSocketSendTask(void *arg)
{
    int cnt = LWIP_DEMO_TIMES;
    char send_str[128];
    int fd = -1;

    memset(send_str, 0, sizeof(send_str));

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
    {
        lw_error("Socket error\n");
        return;
    }

    struct sockaddr_in udp_sock;
    udp_sock.sin_family = AF_INET;
    udp_sock.sin_port = htons(udp_socket_port);
    udp_sock.sin_addr.s_addr = inet_addr(udp_ip_str);
    memset(&(udp_sock.sin_zero), 0, sizeof(udp_sock.sin_zero));

    if(connect(fd, (struct sockaddr *)&udp_sock, sizeof(struct sockaddr)))
    {
        lw_error("Unable to connect\n");
        close(fd);
        return;
    }

    lw_print("UDP connect %s:%d success, start to send.\n",
        udp_ip_str,
        udp_socket_port);

    while(cnt --)
    {
        snprintf(send_str, sizeof(send_str), "UDP test package times %d\r\n", cnt);
        send(fd, send_str, strlen(send_str), 0);
        lw_notice("Send UDP msg: %s ", send_str);
        PrivTaskDelay(1000);
    }

    close(fd);
    return;
}

void UdpSocketSendTest(int argc, char *argv[])
{
    if(argc >= 2)
    {
        lw_notice("lw: [%s] target ip %s\n", __func__, argv[1]);
        UdpSocketConfigParam(argv[1]);
    }

#ifdef ADD_XIZI_FETURES
    lwip_config_tcp(lwip_ipaddr, lwip_netmask, udp_socket_ip);
    sys_thread_new("UdpSocketSendTask", UdpSocketSendTask, NULL, LWIP_TASK_STACK_SIZE,
        LWIP_DEMO_TASK_PRIO);
#endif
#ifdef ADD_NUTTX_FETURES
    UdpSocketSendTask(NULL);
#endif
}
PRIV_SHELL_CMD_FUNCTION(UdpSocketSendTest, a udp send sample, PRIV_SHELL_CMD_MAIN_ATTR);

