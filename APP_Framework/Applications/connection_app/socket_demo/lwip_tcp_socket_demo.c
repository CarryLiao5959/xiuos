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
* @file lwip_tcp_socket_demo.c
* @brief TCP socket demo based on LwIP
* @version 1.0
* @author AIIT XUOS Lab
* @date 2022-03-21
*/

#include <transform.h>

#ifdef ADD_XIZI_FETURES
#include "sys_arch.h"
#include <lwip/sockets.h>
#include "lwip/sys.h"
#endif

#ifdef ADD_NUTTX_FETURES
#include <sys/socket.h>
#include <netinet/in.h>
#include "stdio.h"
#endif

#define TCP_DEMO_BUF_SIZE 65535

char tcp_socket_ip[] = {192, 168, 250, 252};

#ifdef ADD_NUTTX_FETURES
#define lw_print printf
#define lw_notice printf
#define lw_error printf

#define LWIP_DEMO_TIMES 3

/** Create u32_t value from bytes */
#define LWIP_MAKEU32(a,b,c,d) (((uint32_t)((a) & 0xff) << 24) | \
                               ((uint32_t)((b) & 0xff) << 16) | \
                               ((uint32_t)((c) & 0xff) << 8)  | \
                                (uint32_t)((d) & 0xff))

#define PP_HTONL(x)   ((uint32_t)(x))
#define LWIP_TARGET_PORT 6000
#endif

uint16_t tcp_socket_port = LWIP_TARGET_PORT;

/******************************************************************************/
static void TCPSocketRecvTask(void *arg)
{
    int fd = -1, clientfd;
    int recv_len;
    char *recv_buf;
    struct sockaddr_in tcp_addr;
    socklen_t addr_len;

    while(1) {
        recv_buf = (char *)malloc(TCP_DEMO_BUF_SIZE);
        if (recv_buf == NULL) {
            lw_error("No memory\n");
            continue;
        }

        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            lw_error("Socket error\n");
            free(recv_buf);
            continue;
        }

        tcp_addr.sin_family = AF_INET;
        tcp_addr.sin_addr.s_addr = INADDR_ANY;
        tcp_addr.sin_port = htons(tcp_socket_port);
        memset(&(tcp_addr.sin_zero), 0, sizeof(tcp_addr.sin_zero));

        if (bind(fd, (struct sockaddr *)&tcp_addr, sizeof(struct sockaddr)) == -1) {
            lw_error("Unable to bind\n");
            close(fd);
            free(recv_buf);
            continue;
        }

        lw_print("tcp bind success, start to receive.\n");
        lw_notice("\n\nLocal Port:%d\n\n", tcp_socket_port);

        // setup socket fd as listening mode
        if (listen(fd, 5) != 0 ) {
            lw_error("Unable to listen\n");
            close(fd);
            free(recv_buf);
            continue;
        }

        // accept client connection
        clientfd = accept(fd, (struct sockaddr *)&tcp_addr, (socklen_t*)&addr_len);
        lw_notice("client %s connected\n", inet_ntoa(tcp_addr.sin_addr));

        while(1) {
            memset(recv_buf, 0, TCP_DEMO_BUF_SIZE);
            recv_len = recvfrom(clientfd, recv_buf, TCP_DEMO_BUF_SIZE, 0, (struct sockaddr *)&tcp_addr, &addr_len);
            if(recv_len > 0) {
                lw_notice("Receive from : %s\n", inet_ntoa(tcp_addr.sin_addr));
                lw_notice("Receive data : %d - %s\n\n", recv_len, recv_buf);
            }
            sendto(clientfd, recv_buf, recv_len, 0, (struct sockaddr*)&tcp_addr, addr_len);
        }
    }

    close(fd);
    free(recv_buf);
}

#ifdef ADD_XIZI_FETURES
void TCPSocketRecvTest(int argc, char *argv[])
{
    int result = 0;

    if(argc >= 2) {
        lw_print("lw: [%s] target ip %s\n", __func__, argv[1]);
        if(sscanf(argv[1], "%d.%d.%d.%d:%d", &tcp_socket_ip[0], &tcp_socket_ip[1], &tcp_socket_ip[2], &tcp_socket_ip[3], &tcp_socket_port) == 0) {
            sscanf(argv[1], "%d.%d.%d.%d", &tcp_socket_ip[0], &tcp_socket_ip[1], &tcp_socket_ip[2], &tcp_socket_ip[3]);
        }
    }

    lwip_config_tcp(lwip_ipaddr, lwip_netmask, tcp_socket_ip);
    sys_thread_new("TCPSocketRecvTask", TCPSocketRecvTask, NULL, LWIP_TASK_STACK_SIZE, LWIP_DEMO_TASK_PRIO);
}
PRIV_SHELL_CMD_FUNCTION(TCPSocketRecvTest, a tcp receive sample, PRIV_SHELL_CMD_MAIN_ATTR);
#endif

static void TCPSocketSendTask(void *arg)
{
    int cnt = LWIP_DEMO_TIMES;
    int fd = -1;
    char send_msg[128];

    lw_print("%s start\n", __func__);

    memset(send_msg, 0, sizeof(send_msg));
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        lw_print("Socket error\n");
        return;
    }

    struct sockaddr_in tcp_sock;
    tcp_sock.sin_family = AF_INET;
    tcp_sock.sin_port = htons(tcp_socket_port);
    tcp_sock.sin_addr.s_addr = PP_HTONL(LWIP_MAKEU32(tcp_socket_ip[0], tcp_socket_ip[1], tcp_socket_ip[2], tcp_socket_ip[3]));
    memset(&(tcp_sock.sin_zero), 0, sizeof(tcp_sock.sin_zero));

    if (connect(fd, (struct sockaddr *)&tcp_sock, sizeof(struct sockaddr))) {
        lw_print("Unable to connect\n");
        close(fd);
        return;
    }

    lw_notice("\n\nTarget Port:%d\n\n", tcp_socket_port);

    while (cnt --) {
        lw_print("Lwip client is running.\n");
        snprintf(send_msg, sizeof(send_msg), "TCP test package times %d\r\n", cnt);
        sendto(fd, send_msg, strlen(send_msg), 0, (struct sockaddr*)&tcp_sock, sizeof(struct sockaddr));
        lw_notice("Send tcp msg: %s ", send_msg);
        PrivTaskDelay(1000);
    }

    close(fd);
    return;
}

#ifdef ADD_XIZI_FETURES
void TCPSocketSendTest(int argc, char *argv[])
{
    if(argc >= 2) {
        lw_print("lw: [%s] target ip %s\n", __func__, argv[1]);
        if(sscanf(argv[1], "%d.%d.%d.%d:%d", &tcp_socket_ip[0], &tcp_socket_ip[1], &tcp_socket_ip[2], &tcp_socket_ip[3], &tcp_socket_port) == 0)
        {
            sscanf(argv[1], "%d.%d.%d.%d", &tcp_socket_ip[0], &tcp_socket_ip[1], &tcp_socket_ip[2], &tcp_socket_ip[3]);
        }
    }

    lwip_config_tcp(lwip_ipaddr, lwip_netmask, tcp_socket_ip);
    sys_thread_new("TCP Socket Send", TCPSocketSendTask, NULL, LWIP_TASK_STACK_SIZE, LWIP_DEMO_TASK_PRIO);
}
PRIV_SHELL_CMD_FUNCTION(TCPSocketSendTest, a tcp send sample, PRIV_SHELL_CMD_MAIN_ATTR);
#endif

#ifdef ADD_NUTTX_FETURES
void tcp_recv_demo(void)
{
    TCPSocketRecvTask(NULL);
}

void tcp_send_demo(void)
{
    TCPSocketSendTask(NULL);
}
#endif

