/*
* Copyright (c) 2020 AIIT XUOS Lab
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
 * @file hfa21.c
 * @brief Implement the connection wifi adapter function, using HFA21 device
 * @version 1.1
 * @author AIIT XUOS Lab
 * @date 2021.07.08
 */

#include <adapter.h>
#include <at_agent.h>

#define LEN_PARA_BUF 128

static int Hfa21SetDown(struct Adapter *adapter_at);

/**
 * @description: enter AT command mode
 * @param at_agent - wifi device agent pointer
 * @return success: EOK
 */
static int Hfa21InitAtCmd(ATAgentType at_agent)
{
    ATOrderSend(at_agent, REPLY_TIME_OUT, NULL, "+++");
    PrivTaskDelay(100);

    ATOrderSend(at_agent, REPLY_TIME_OUT, NULL, "a");
    PrivTaskDelay(500);

    return 0;
}

/**
 * @description: Open wifi
 * @param adapter - wifi device pointer
 * @return success: EOK, failure: ENOMEMORY
 */
static int Hfa21Open(struct Adapter *adapter)
{
    /*step1: open ec200t serial port*/
    adapter->fd = PrivOpen(ADAPTER_HFA21_DRIVER, O_RDWR);
    if (adapter->fd < 0) {
        printf("Hfa21Open get serial %s fd error\n", ADAPTER_HFA21_DRIVER);
        return -1;
    }

    /*step2: init AT agent*/
    if (!adapter->agent) {
        char *agent_name = "wifi_uart_client";
        if (EOK != InitATAgent(agent_name, adapter->fd, 512)) {
            printf("at agent init failed !\n");
            return -1;
        }
        ATAgentType at_agent = GetATAgent(agent_name);

        adapter->agent = at_agent;
    }

    ADAPTER_DEBUG("Hfa21 open done\n");

    return 0;
}

/**
 * @description: Close wifi
 * @param adapter - wifi device pointer
 * @return success: EOK
 */
static int Hfa21Close(struct Adapter *adapter)
{
    return Hfa21SetDown(adapter);
}

/**
 * @description: send data to adapter
 * @param adapter - wifi device pointer
 * @param data - data bufferd
 * @param data - data length
 * @return success: EOK
 */
static int Hfa21Send(struct Adapter *adapter, const void *data, size_t len)
{
    x_err_t result = EOK;
    if (adapter->agent) {
        EntmSend(adapter->agent, (const char *)data, len);
    }else {
        printf("Can not find agent \n");
	}

__exit:

    return result;
}

/**
 * @description: receive data from adapter
 * @param adapter - wifi device pointer
 * @param data - data bufferd
 * @param data - data length
 * @return success: EOK
 */
static int Hfa21Receive(struct Adapter *adapter, void *rev_buffer, size_t buffer_len)
{
    x_err_t result = EOK;
    printf("hfa21 receive waiting ... \n");

    if (adapter->agent) {
        return EntmRecv(adapter->agent, (char *)rev_buffer, buffer_len, 40000);
    } else {
        printf("Can not find agent \n");
	}

__exit:

    return result;
}

/**
 * @description: connnect wifi to internet
 * @param adapter - wifi device pointer
 * @return success: EOK
 */
static int Hfa21SetUp(struct Adapter *adapter)
{
    uint8 wifi_ssid[LEN_PARA_BUF] = "AIIT-Guest";
    uint8 wifi_pwd[LEN_PARA_BUF] = "";
    char cmd[LEN_PARA_BUF];

    //struct at_device_esp8266 *esp8266 = (struct at_device_esp8266 *) device->UserData;
    struct ATAgent *agent = adapter->agent;

    /* wait hfa21 device startup finish */
    PrivTaskDelay(5000);

    Hfa21InitAtCmd(agent);

    memset(cmd,0,sizeof(cmd));
    strcpy(cmd,"AT+FCLR\r");
    ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
    PrivTaskDelay(20000);

    Hfa21InitAtCmd(agent);

    memset(cmd,0,sizeof(cmd));
    strcpy(cmd,"AT+WSSSID=");
    strcat(cmd,wifi_ssid);
    strcat(cmd,"\r");
    ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
    PrivTaskDelay(2500);
    
    memset(cmd,0,sizeof(cmd));
    strcpy(cmd,"AT+WSKEY=OPEN,NONE,");
    strcat(cmd,wifi_pwd);
    strcat(cmd,"\r");
    ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
    PrivTaskDelay(2500);

    memset(cmd,0,sizeof(cmd));
    strcpy(cmd,"AT+WMODE=sta\r");
    ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
    PrivTaskDelay(2500);

    memset(cmd,0,sizeof(cmd));
    strcat(cmd,"AT+Z\r");
    ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
    PrivTaskDelay(10000);

    return 0;
}

/**
 * @description: disconnnect wifi from internet
 * @param adapter - wifi device pointer
 * @return success: EOK
 */
static int Hfa21SetDown(struct Adapter *adapter)
{
    Hfa21InitAtCmd(adapter->agent);

    ATOrderSend(adapter->agent, REPLY_TIME_OUT, NULL, "AT+FCLR\r");
    PrivTaskDelay(20000);

    return 0;
}

/**
 * @description: set wifi ip/gateway/netmask address(in sta mode)
 * @param adapter - wifi device pointer
 * @param ip - ip address
 * @param gateway - gateway address
 * @param netmask - netmask address
 * @return success: EOK, failure: ENOMEMORY
 */
static int Hfa21SetAddr(struct Adapter *adapter, const char *ip, const char *gateway, const char *netmask)
{
    #define HFA21_SET_ADDR_EXPRESSION        "+ok=%[^,],%[^,],%[^,],%[^,]\r"
    char *dhcp_mode =NULL;
    char *ip_str = NULL;
    char *gw_str = NULL;
    char *mask_str = NULL;

    dhcp_mode = (char *) PrivCalloc(1, 8);
    ip_str = (char *) PrivCalloc(1, 17);
    gw_str = (char *) PrivCalloc(1, 17);
    mask_str = (char *) PrivCalloc(1, 17);

    Hfa21InitAtCmd(adapter->agent);

    x_err_t result = EOK;

    ATReplyType reply = CreateATReply(64);
    if (NULL == reply) {
        printf("at_create_resp failed ! \n");
        result = ENOMEMORY;
        goto __exit;
    }

    ATOrderSend(adapter->agent, REPLY_TIME_OUT, NULL, "AT+WANN=%s,%s,%s,%s\r", "dhcp", ip, netmask, gateway);
    PrivTaskDelay(2500);

    ATOrderSend(adapter->agent, REPLY_TIME_OUT, reply, "AT+WANN\r");
    PrivTaskDelay(2500);

    ATOrderSend(adapter->agent, REPLY_TIME_OUT, NULL, "AT+Z\r");
    PrivTaskDelay(10000);

    const char * result_buf = GetReplyText(reply);

    char* str = strstr(result_buf, "+ok=");

    ParseATReply(str, HFA21_SET_ADDR_EXPRESSION, dhcp_mode,ip_str,mask_str,gw_str);
    printf("after configure:\n mode:%s\n ip:%s\n netmask:%s\n gateway:%s\n", dhcp_mode, ip_str, mask_str, gw_str);

__exit:
    if (reply) {
        DeleteATReply(reply);
    }

    return result;
}

/**
 * @description: ping
 * @param adapter - wifi device pointer
 * @param destination - domain name or ip address
 * @return success: EOK, failure: ENOMEMORY
 */
static int Hfa21Ping(struct Adapter *adapter, const char *destination)
{
    char *ping_result = NONE;
    char *dst = NONE;
    ping_result = (char *) PrivCalloc(1, 17);
    dst = (char *) PrivCalloc(1, 17);
    strcpy(dst, destination);
    strcat(dst, "\r");

    Hfa21InitAtCmd(adapter->agent);

    uint32 result = EOK;

    ATReplyType reply = CreateATReply(64);
    if (NULL == reply) {
        printf("at_create_resp failed ! \n");
        result = ENOMEMORY;
        goto __exit;
    }

    //ping baidu.com
    ATOrderSend(adapter->agent, REPLY_TIME_OUT, reply, "AT+PING=%s", dst);

    PrivTaskDelay(2500);

    ATOrderSend(adapter->agent, REPLY_TIME_OUT, NULL, "AT+Z\r");
    PrivTaskDelay(10000);

    const char * result_buf = GetReplyText(reply);

    char* str = strstr(result_buf, "+ok=");

    ParseATReply(str, "+ok=%s\r", ping_result);

    printf("ping www.baidu.com(36.152.44.95) result is:%s\n", ping_result);

__exit:
    if (reply) {
        DeleteATReply(reply);
    }

    return result;
}

/**
 * @description: display network configuration
 * @param adapter - wifi device pointer
 * @return success: EOK, failure: ENOMEMORY
 */
static int Hfa21Netstat(struct Adapter *adapter)
{
    #define HFA21_NETSTAT_RESP_SIZE         320
    #define HFA21_NETSTAT_TYPE_SIZE         10
    #define HFA21_NETSTAT_IPADDR_SIZE       17
    #define HFA21_WANN_EXPRESSION        "+ok=%[^,],%[^,],%[^,],%[^,]\r"
    #define HFA21_LANN_EXPRESSION        "+ok=%[^,],%[^,]\r"
    #define HFA21_WMODE_EXPRESSION       "+ok=%s\r"

    ATReplyType reply = NULL;
    struct ATAgent *agent = adapter->agent;
    uint32 result;
    char * result_buf = NULL;
    char * str = NULL;

    /* sta/ap */
    char *work_mode = NULL;
    /* dhcp/static */
    char *ip_mode = NULL;
    char *local_ipaddr = NULL;
    char *gateway = NULL;
    char *netmask = NULL;
    local_ipaddr = (char *) PrivCalloc(1, HFA21_NETSTAT_IPADDR_SIZE);
    gateway = (char *) PrivCalloc(1, HFA21_NETSTAT_IPADDR_SIZE);
    netmask = (char *) PrivCalloc(1, HFA21_NETSTAT_IPADDR_SIZE);
    work_mode = (char *) PrivCalloc(1, HFA21_NETSTAT_IPADDR_SIZE);
    ip_mode = (char *) PrivCalloc(1, HFA21_NETSTAT_IPADDR_SIZE);

    reply = CreateATReply(HFA21_NETSTAT_RESP_SIZE);
    if (reply == NULL) {
        result = ENOMEMORY;
        goto __exit;
    }

    ATOrderSend(agent, REPLY_TIME_OUT, NULL, "+++");
    PrivTaskDelay(100);

    ATOrderSend(agent, REPLY_TIME_OUT, NULL, "a");
    PrivTaskDelay(2500);

    ATOrderSend(agent, REPLY_TIME_OUT, reply, "AT+WMODE\r");
    PrivTaskDelay(2500);

    result_buf = GetReplyText(reply);

    str = strstr(result_buf, "+ok=");
    /* parse the third line of response data, get the network connection information */
    ParseATReply(str, HFA21_WMODE_EXPRESSION, work_mode);

    if (work_mode[0]=='S') {
        if (ATOrderSend(agent, REPLY_TIME_OUT, reply, "AT+WANN\r") < 0) {
            goto __exit;
        }

        PrivTaskDelay(2500);

        GetReplyText(reply);
        str = strstr(result_buf, "+ok=");
        /* parse the third line of response data, get the network connection information */
        ParseATReply(str, HFA21_WANN_EXPRESSION, ip_mode, local_ipaddr, netmask, gateway);
    } else {
        ATOrderSend(agent, REPLY_TIME_OUT, reply, "AT+LANN\r");
        PrivTaskDelay(2500);

        GetReplyText(reply);

        str = strstr(result_buf, "+ok=");
        /* parse the third line of response data, get the network connection information */
        ParseATReply(str, HFA21_LANN_EXPRESSION, local_ipaddr, netmask);
    }

    ATOrderSend(adapter->agent, REPLY_TIME_OUT, NULL, "AT+Z\r");
    PrivTaskDelay(10000);

    printf("work mode: %s\n", work_mode);
    if (work_mode[0]=='S')
        printf("ip mode: %s\nlocal ip: %s\nnetmask: %s\ngateway: %s\n", ip_mode, local_ipaddr, netmask, gateway);
    else
        printf("local ip: %s\nnetmask: %s\n", local_ipaddr, netmask);

    return EOK;

__exit:
    if (reply)
        DeleteATReply(reply);
    if (local_ipaddr)
        PrivFree(local_ipaddr);
    if (netmask)
        PrivFree(netmask);
    if (gateway)
        PrivFree(gateway);
    if (work_mode)
        PrivFree(work_mode);
}

static int Hfa21Connect(struct Adapter *adapter, enum NetRoleType net_role, const char *ip, const char *port, enum IpType ip_type)
{
    int result = EOK;
    ATReplyType reply = NONE;
    char cmd[LEN_PARA_BUF];
    struct ATAgent *agent = adapter->agent;

    reply = CreateATReply(64);
    if (reply == NONE) {
        printf("no memory for reply struct.");
        return ENOMEMORY;
    }

    Hfa21InitAtCmd(adapter->agent);

    memset(cmd,0,sizeof(cmd));
    strcpy(cmd,"AT+NETP=TCP,");
    if(net_role == CLIENT)
        strcat(cmd,"CLIENT,");
    else if(net_role == SERVER)
        strcat(cmd,"SERVER,");
    strcat(cmd,port);
    strcat(cmd,",");
    if(ip_type == IPV4)
        strcat(cmd,ip);
    else if(ip_type == IPV6)
    {
        
    }
    strcat(cmd,"\r");
    ATOrderSend(agent, REPLY_TIME_OUT, NULL, cmd);
    PrivTaskDelay(2500);

    ATOrderSend(adapter->agent, REPLY_TIME_OUT, NULL, "AT+Z\r");
    PrivTaskDelay(10000);

    adapter->net_role = net_role;

__exit:
    if (reply) {
        DeleteATReply(reply);
    }

    return result;
}

static int Hfa21Ioctl(struct Adapter *adapter, int cmd, void *args)
{
    if (OPE_INT != cmd) {
        printf("Hfa21Ioctl only support OPE_INT, do not support %d\n", cmd);
        return -1;
    }

    uint32_t baud_rate = *((uint32_t *)args);

    struct SerialDataCfg serial_cfg;
    memset(&serial_cfg, 0 ,sizeof(struct SerialDataCfg));
    serial_cfg.serial_baud_rate = baud_rate;
    serial_cfg.serial_data_bits = DATA_BITS_8;
    serial_cfg.serial_stop_bits = STOP_BITS_1;
    serial_cfg.serial_buffer_size = SERIAL_RB_BUFSZ;
    serial_cfg.serial_parity_mode = PARITY_NONE;
    serial_cfg.serial_bit_order = BIT_ORDER_LSB;
    serial_cfg.serial_invert_mode = NRZ_NORMAL;
#ifdef ADAPTER_HFA21_DRIVER_EXT_PORT
    serial_cfg.ext_uart_no = ADAPTER_HFA21_DRIVER_EXT_PORT;
    serial_cfg.port_configure = PORT_CFG_INIT;
#endif

    struct PrivIoctlCfg ioctl_cfg;
    ioctl_cfg.ioctl_driver_type = SERIAL_TYPE;
    ioctl_cfg.args = &serial_cfg;
    PrivIoctl(adapter->fd, OPE_INT, &ioctl_cfg);
    printf("Hfa21Ioctl success\n");
    return 0;
}

static const struct IpProtocolDone hfa21_done =
{
    .open = Hfa21Open,
    .close =  Hfa21Close,
    .ioctl = Hfa21Ioctl,
    .setup = Hfa21SetUp,
    .setdown = Hfa21SetDown,
    .setaddr = Hfa21SetAddr,
    .setdns = NULL,
    .setdhcp = NULL,
    .ping = Hfa21Ping,
    .netstat = Hfa21Netstat,
    .connect = Hfa21Connect,
    .send = Hfa21Send,
    .recv = Hfa21Receive,
    .disconnect = NULL,
};

/**
 * @description: Register wifi device hfa21
 * @return success: EOK, failure: ERROR
 */
AdapterProductInfoType Hfa21Attach(struct Adapter *adapter)
{
    struct AdapterProductInfo *product_info = PrivMalloc(sizeof(struct AdapterProductInfo));
    if (!product_info) {
        printf("Hfa21Attach Attach malloc product_info error\n");
        PrivFree(product_info);
        return NULL;
    }

    strcpy(product_info->model_name, ADAPTER_WIFI_HFA21);

    product_info->model_done = (void *)&hfa21_done;

    return product_info;
}