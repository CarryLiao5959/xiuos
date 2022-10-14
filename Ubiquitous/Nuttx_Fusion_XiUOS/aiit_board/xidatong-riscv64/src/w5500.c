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
* @file w5500.c
* @brief w5500 driver based on simulated SPI
* @version 1.0
* @author AIIT XUOS Lab
* @date 2022-9-14
*/

#include "nuttx/arch.h"
#include "w5500.h"
#include "k210_gpio_common.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/
w5500_param_t w5500_param;

/****************************************************************************
 * Name: spi_read_byte
 * Description: Read one byte spi data returned
 * input: None
 * output: None
 * return:Read register data
 ****************************************************************************/
static uint8_t spi_read_byte(void)
{
    uint8_t i, dat = 0;
    SCLK_L();

    for(i = 0; i < 8; i++)
    {
        SCLK_H();
        dat <<= 1;
        dat |= k210_gpiohs_get_value(FPIOA_ENET_MISO);
        up_udelay(1);
        SCLK_L();
    }

    return dat;
}

/****************************************************************************
 * Name: spi_write_byte
 * Description: send 1 byte to spi
 * input: data
 * output: None
 * return: None
 ****************************************************************************/
static void spi_write_byte(uint8_t dat)
{
    uint8_t i;

    for(i = 0; i < 8; i++)
    {
        SCLK_L();

        if((dat << i) & 0x80)
        {
            MOSI_H();
        }
        else
        {
            MOSI_L();
        }

        SCLK_H();
    }

    SCLK_L();
}

/****************************************************************************
 * Name: spi_write_short
 * Description: send 2 bytes to spi
 * input: data
 * output: None
 * return: None
 ****************************************************************************/
static void spi_write_short(uint16_t dat)
{
    spi_write_byte((uint8_t)(dat / 256));
    spi_write_byte(dat);
}

/****************************************************************************
 * Name: w5500_write_byte
 *Description: Write 1 byte data to the specified address register through SPI
 *Input: reg: 16 bit register address, dat: data to be written
 * output: None
 * return: None
 ****************************************************************************/
static void w5500_write_byte(uint16_t reg, uint8_t dat)
{
    NCS_L();
    spi_write_short(reg);
    spi_write_byte(FDM1 | RWB_WRITE | COMMON_R);
    spi_write_byte(dat);
    NCS_H();
}

/****************************************************************************
 * Name: w5500_write_short
 * Description: Write 2 bytes data to the specified address register through SPI
 * Input: reg: 16 bit register address, dat: data to be written
 * output: None
 * return: None
 ****************************************************************************/
static void w5500_write_short(uint16_t reg, uint16_t dat)
{
    NCS_L();
    spi_write_short(reg);
    spi_write_byte(FDM2 | RWB_WRITE | COMMON_R);
    spi_write_short(dat);
    NCS_H();
}

/****************************************************************************
 * Name: w5500_write_bytes
 * Description: Write n bytes data to the specified address register through SPI
 * Input: reg: 16 bit register address, dat: data to be written,size:Length of data to be written
 * output: None
 * return: None
 ****************************************************************************/
static void w5500_write_bytes(uint16_t reg, uint8_t *dat, uint16_t size)
{
    uint16_t i;
    NCS_L();
    spi_write_short(reg);
    spi_write_byte(VDM | RWB_WRITE | COMMON_R);

    for(i = 0; i < size; i++)
    {
        spi_write_byte(*dat++);
    }

    NCS_H();
}

/****************************************************************************
 * Name: w5500_write_sock_byte
 * Description: Write 1 byte data to the specified port register through SPI
 * Input: sock: port number, reg: 16 bit register address, dat: data to be written
 * Output: None
 * return: None
 ****************************************************************************/
void w5500_write_sock_byte(socket_t sock, uint16_t reg, uint8_t dat)
{
    NCS_L();
    spi_write_short(reg);
    spi_write_byte(FDM1 | RWB_WRITE | (sock * 0x20 + 0x08));
    spi_write_byte(dat);
    NCS_H();
}

/****************************************************************************
 * Name: w5500_write_sock_short
 * Description: Write 2 bytes data to the specified port register through SPI
 * Input: sock: port number, reg: 16 bit register address, dat: data to be written
 * Output: None
 * return: None
 ****************************************************************************/
void w5500_write_sock_short(socket_t sock, uint16_t reg, uint16_t dat)
{
    NCS_L();
    spi_write_short(reg);
    spi_write_byte(FDM2 | RWB_WRITE | (sock * 0x20 + 0x08));
    spi_write_short(dat);
    NCS_H();
}

/****************************************************************************
 * Name: w5500_write_sock_long
 * Description: Write 4 bytes data to the specified port register through SPI
 * Input: sock: port number, reg: 16 bit register address, dat: 4 byte buffer pointers to be written
 * Output: None
 * return: None
 ****************************************************************************/
void w5500_write_sock_long(socket_t sock, uint16_t reg, uint8_t *dat)
{
    NCS_L();
    spi_write_short(reg);
    spi_write_byte(FDM4 | RWB_WRITE | (sock * 0x20 + 0x08));
    spi_write_byte(*dat++);
    spi_write_byte(*dat++);
    spi_write_byte(*dat++);
    spi_write_byte(*dat++);
    NCS_H();
}

/*******************************************************************************
*Function name: w5500_read_byte
*Description: Read 1 byte data of W5500 specified address register
*Input: reg: 16 bit register address
*Output: None
*Return : 1 byte data read from the register
*******************************************************************************/
uint8_t w5500_read_byte(uint16_t reg)
{
    uint8_t val;
    NCS_L();
    spi_write_short(reg);
    spi_write_byte(FDM1 | RWB_READ | COMMON_R);
    val = spi_read_byte();
    NCS_H();
    return val;
}

/*******************************************************************************
*Function name: w5500_read_sock_byte
*Description: Read 1 byte data of W5500 specified port register
*Input: sock: port number, reg: 16 bit register address
*Output: None
*Return: 1 byte data read from the register
*Description: None
*******************************************************************************/
uint8_t w5500_read_sock_byte(socket_t sock, uint16_t reg)
{
    uint8_t val;
    NCS_L();
    spi_write_short(reg);
    spi_write_byte(FDM1 | RWB_READ | (sock * 0x20 + 0x08));
    val = spi_read_byte();
    NCS_H();
    return val;
}

/*******************************************************************************
*Function name: w5500_read_sock_short
*Description: Read 2 bytes of W5500 specified port register
*Input: sock: port number, reg: 16 bit register address
*Output: None
*Return: read 2 bytes of data from the register (16 bits)
*******************************************************************************/
uint16_t w5500_read_sock_short(socket_t sock, uint16_t reg)
{
    uint16_t val;
    NCS_L();
    spi_write_short(reg);
    spi_write_byte(FDM2 | RWB_READ |(sock * 0x20 + 0x08));
    val = spi_read_byte();
    val *= 256;
    val |= spi_read_byte();
    NCS_H();
    return val;
}

/*******************************************************************************
*Function name: w5500_read_sock_bytes
*Description: Read data from W5500 receive data buffer
*Input: sock: port number, * dat: data saving buffer pointer
*Output: None
*Return: read data length
*******************************************************************************/
uint16_t w5500_read_sock_bytes(socket_t sock, uint8_t *dat)
{
    uint16_t recv_size, write_addr;
    uint16_t recv_addr;
    uint16_t i;
    uint8_t val;
    recv_size = w5500_read_sock_short(sock, W5500_SN_RX_RSR_REG);

    //no receive data
    if(recv_size == 0)
    {
        return 0;
    }

    if(recv_size > W5500_MAX_PACK_SIZE)
    {
        recv_size = W5500_MAX_PACK_SIZE;
    }

    recv_addr = w5500_read_sock_short(sock, W5500_SN_RX_RD_REG);
    write_addr = recv_addr;

    //calculate physical address
    recv_addr &= (SOCK_RECV_SIZE - 1);
    NCS_L();
    spi_write_short(recv_addr);
    spi_write_byte(VDM | RWB_READ | (sock * 0x20 + 0x18));

    if((recv_addr + recv_size) < SOCK_RECV_SIZE)
    {
        for(i = 0; i < recv_size; i++)
        {
            val = spi_read_byte();
            *dat = val;
            dat++;
        }
    }
    else
    {
        recv_addr = SOCK_RECV_SIZE - recv_addr;

        for(i = 0; i < recv_addr; i++)
        {
            val = spi_read_byte();
            *dat = val;
            dat++;
        }

        NCS_H();
        NCS_L();
        spi_write_short(0x00);
        spi_write_byte(VDM | RWB_READ | (sock * 0x20 + 0x18));

        for(; i < recv_size; i++)
        {
            val= spi_read_byte();
            *dat = val;
            dat++;
        }
    }

    NCS_H();
    write_addr += recv_size;

    w5500_write_sock_short(sock, W5500_SN_RX_RD_REG, write_addr);
    //start receive
    w5500_write_sock_byte(sock, W5500_SN_CR_REG, SN_CR_RECV);
    return recv_size;
}

/*******************************************************************************
*Function name: w5500_write_sock_bytes
*Description: Write data to the data sending buffer of W5500
*Input: sock: port number, dat: data storage buffer pointer, size: length of data to be written
*Output: None
*Return: None
*******************************************************************************/
void w5500_write_sock_bytes(socket_t sock, uint8_t *dat, uint16_t size)
{
    uint16_t recv_addr, write_addr;
    uint16_t i;

    // if udp mode, set ip and port
    if(w5500_read_sock_byte(sock, W5500_SN_MR_REG) != SOCK_UDP)
    {
        w5500_write_sock_long(sock, W5500_SN_DIPR_REG, w5500_param.udp_ip);
        w5500_write_sock_short(sock, W5500_SN_DPORTR_REG, w5500_param.udp_port);
    }

    recv_addr = w5500_read_sock_short(sock, W5500_SN_TX_WR_REG);
    write_addr = recv_addr;
    recv_addr &= (SOCK_SEND_SIZE - 1);

    NCS_L();
    spi_write_short(recv_addr);
    spi_write_byte(VDM | RWB_WRITE | (sock * 0x20 + 0x10));

    if((recv_addr + size) < SOCK_SEND_SIZE)
    {
        for(i = 0; i < size; i++)
        {
            spi_write_byte(*dat++);
        }
    }
    else
    {
        recv_addr = SOCK_SEND_SIZE - recv_addr;

        for(i = 0; i < recv_addr; i++)
        {
            spi_write_byte(*dat++);
        }

        NCS_H();
        NCS_L();

        spi_write_short(0x00);
        spi_write_byte(VDM | RWB_WRITE | (sock * 0x20 + 0x10));

        for(; i < size; i++)
        {
            spi_write_byte(*dat++);
        }
    }

    NCS_H();
    write_addr += size;

    w5500_write_sock_short(sock, W5500_SN_TX_WR_REG, write_addr);
    w5500_write_sock_byte(sock, W5500_SN_CR_REG, SN_CR_SEND);
}

/*******************************************************************************
*Function name: w5500_reset
*Description: Hardware reset W5500
*Input: None
*Output: None
*Return value: None
*Note: The reset pin of the W5500 can be encircled only when the low level is at least 500us
*******************************************************************************/
void w5500_reset(void)
{
    uint8_t dat = 0;

    RST_L();
    RST_H();

    //wait connect ok
    while((dat & LINK) == 0)
    {
        up_mdelay(500);
        dat = w5500_read_byte(W5500_PHYCFGR_REG);
    }
}

/*******************************************************************************
*Function name: w5500_config_init
*Description: Initialize W5500 register functions
*Input: None
*Output: None
*Return value: None
*Note: Before using W5500, initialize W5500
*******************************************************************************/
void w5500_config_init(void)
{
    uint8_t i = 0;

    //software reset, set 1 and auto clear 0
    w5500_write_byte(W5500_MR_REG, MR_RST);
    up_mdelay(100);

    w5500_write_bytes(W5500_GAR_REG, w5500_param.gw_addr, 4);
    w5500_write_bytes(W5500_SUBR_REG, w5500_param.ip_mask, 4);
    w5500_write_bytes(W5500_SHAR_REG, w5500_param.mac_addr, 6);
    w5500_write_bytes(W5500_SIPR_REG, w5500_param.ip_addr, 4);

    //set socket rx and tx memory size 2k
    for(i = 0; i < 8; i++)
    {
        w5500_write_sock_byte(i, W5500_SN_RXBUF_SIZE_REG, 0x02);
        w5500_write_sock_byte(i, W5500_SN_TXBUF_SIZE_REG, 0x02);
    }

    //set retry time 200ms (0x07d0 = 2000)
    w5500_write_short(W5500_RTR_REG, 0x07d0);

    //retry time with 8, when exceed it, produce overtime interrupt, set W5500_SN_IR_REG(TIMEOUT)
    w5500_write_byte(W5500_RCR_REG, 8);
}

/*******************************************************************************
*Function name: Detect_Gateway
*Description: Check the gateway server
*Input: None
*Output: None
*Return value: TRUE (0xFF) for success, FALSE (0x00) for failure
*******************************************************************************/
uint8_t w5500_detect_gateway(void)
{
    uint8_t ip_addr[4] = {w5500_param.ip_addr[0] + 1, w5500_param.ip_addr[1] + 1, w5500_param.ip_addr[2] + 1, w5500_param.ip_addr[3] + 1};

    //check gateway and get gateway phyiscal address
    w5500_write_sock_long(0, W5500_SN_DIPR_REG, ip_addr);
    w5500_write_sock_byte(0, W5500_SN_MR_REG, SN_MR_TCP);
    w5500_write_sock_byte(0, W5500_SN_CR_REG, SN_CR_OPEN);
    up_mdelay(5);

    if(w5500_read_sock_byte(0, W5500_SN_SR_REG) != SOCK_INIT)
    {
        w5500_write_sock_byte(0, W5500_SN_CR_REG, SN_CR_CLOSE);
        return FALSE;
    }

    //set socket connection mode
    w5500_write_sock_byte(0, W5500_SN_CR_REG, SN_CR_CONNECT);

    do
    {
        uint8_t val = 0;
        //read socket0 interrupt register
        val = w5500_read_sock_byte(0, W5500_SN_IR_REG);

        if(val != 0)
        {
            w5500_write_sock_byte(0, W5500_SN_IR_REG, val);
        }

        up_mdelay(5);

        if((val & IR_TIMEOUT) == IR_TIMEOUT)
        {
            return FALSE;
        }
        else if(w5500_read_sock_byte(0, W5500_SN_DHAR_REG) != 0xff)
        {
            //close socket
            w5500_write_sock_byte(0, W5500_SN_CR_REG, SN_CR_CLOSE);
            return TRUE;
        }
    } while(1);
    return TRUE;
}

/*******************************************************************************
*Function name: w5500_socket_init
*Description: Specify Socket (0~7) initialization
*Input: sock: port to be initialized
*Output: None
*Return value: None
*******************************************************************************/
void w5500_socket_init(socket_t sock)
{
    //max partition bytes = 30
    w5500_write_sock_short(0, W5500_SN_MSSR_REG, 30);

    switch(sock)
    {
        case 0:
            w5500_write_sock_short(0, W5500_SN_PORT_REG, w5500_param.sock.local_port);
            w5500_write_sock_short(0, W5500_SN_DPORTR_REG, w5500_param.sock.dst_port);
            w5500_write_sock_long(0, W5500_SN_DIPR_REG, w5500_param.sock.dst_ip);
            break;

        default:
            break;
    }
}

/*******************************************************************************
*Function name: w5500_socket_connect
*Description: Set the specified Socket (0~7) as the client to connect with the remote server
*Input: sock: port to be set
*Output: None
*Return value: TRUE (0xFF) for success, FALSE (0x00) for failure
*******************************************************************************/
uint8_t w5500_socket_connect(socket_t sock)
{
    w5500_write_sock_byte(sock, W5500_SN_MR_REG, SN_MR_TCP);
    w5500_write_sock_byte(sock, W5500_SN_CR_REG, SN_CR_OPEN);
    up_mdelay(5);

    if(w5500_read_sock_byte(sock, W5500_SN_SR_REG) != SOCK_INIT)
    {
        w5500_write_sock_byte(sock, W5500_SN_CR_REG, SN_CR_CLOSE);
        return FALSE;
    }

    w5500_write_sock_byte(sock, W5500_SN_CR_REG, SN_CR_CONNECT);
    return TRUE;
}

/*******************************************************************************
*Function name: w5500_socket_listen
*Description: Set the specified Socket (0~7) as the server to wait for the connection of the remote host
*Input: sock: port to be set
*Output: None
*Return value: TRUE (0xFF) for success, FALSE (0x00) for failure
*******************************************************************************/
uint8_t w5500_socket_listen(socket_t sock)
{
    w5500_write_sock_byte(sock, W5500_SN_MR_REG, SN_MR_TCP);
    w5500_write_sock_byte(sock, W5500_SN_CR_REG, SN_CR_OPEN);
    up_mdelay(5);

    if(w5500_read_sock_byte(sock, W5500_SN_SR_REG) != SOCK_INIT)
    {
        w5500_write_sock_byte(sock, W5500_SN_CR_REG, SN_CR_CLOSE);
        return FALSE;
    }

    w5500_write_sock_byte(sock, W5500_SN_CR_REG, SN_CR_LISTEN);
    up_mdelay(5);

    if(w5500_read_sock_byte(sock, W5500_SN_SR_REG) != SOCK_LISTEN)
    {
        w5500_write_sock_byte(sock, W5500_SN_CR_REG, SN_CR_CLOSE);
        return FALSE;
    }

    return TRUE;
}

/*******************************************************************************
*Function name: w5500_socket_set_udp
*Description: Set the specified Socket (0~7) to UDP mode
*Input: sock: port to be set
*Output: None
*Return value: TRUE (0xFF) for success, FALSE (0x00) for failure
*******************************************************************************/
uint8_t w5500_socket_set_udp(socket_t sock)
{
    w5500_write_sock_byte(sock, W5500_SN_MR_REG, SN_MR_UDP);
    w5500_write_sock_byte(sock, W5500_SN_CR_REG, SN_CR_OPEN);
    up_mdelay(5);

    if(w5500_read_sock_byte(sock, W5500_SN_SR_REG) != SOCK_UDP)
    {
        w5500_write_sock_byte(sock, W5500_SN_CR_REG, SN_CR_CLOSE);
        return FALSE;
    }
    return TRUE;
}

/*******************************************************************************
*Function name: w5500_irq_process
*Description: W5500 interrupt handler framework
*Input: None
*Output: None
*Return value: None
*Description: None
*******************************************************************************/
void w5500_irq_process(void)
{
    uint8_t ir_flag, sn_flag;
    ir_flag = w5500_read_byte(W5500_SIR_REG);
    do
    {   
        //handle socket0 event
        if((ir_flag & S0_INT) == S0_INT)
        {
            sn_flag = w5500_read_sock_byte(0, W5500_SN_IR_REG);
            w5500_write_sock_byte(0, W5500_SN_IR_REG, sn_flag);

            if(sn_flag & IR_CON)
            {
                //socket connection finished
                w5500_param.sock.flag |= SOCK_FLAG_CONN;
            }

            if(sn_flag & IR_DISCON)
            {
                //disconnect state
                w5500_write_sock_byte(0, W5500_SN_CR_REG, SN_CR_CLOSE);
                w5500_socket_init(0);
                w5500_param.sock.flag = 0;
            }

            if(sn_flag & IR_SEND_OK)
            {
                //send one package ok
                w5500_param.sock.state |= SOCK_STAT_SEND;
            }

            if(sn_flag & IR_RECV)
            {
                w5500_param.sock.state |= SOCK_STAT_RECV;
            }

            if(sn_flag & IR_TIMEOUT)
            {
                //close socket, connection failed
                w5500_write_sock_byte(0, W5500_SN_CR_REG, SN_CR_CLOSE);
                w5500_param.sock.flag = 0;
            }
        }
        ir_flag = w5500_read_byte(W5500_SIR_REG);
    }while(ir_flag);
}

/*******************************************************************************
*Function name: w5500_load_param
*Description: load param to  w5500_param
*Input: src: source param
*Output: None
*Return value: None
*******************************************************************************/
void w5500_load_param(w5500_param_t *src)
{
    w5500_param_t *dest = &w5500_param;
    memcpy(dest->ip_addr, src->ip_addr, sizeof(src->ip_addr));
    memcpy(dest->ip_mask, src->ip_mask, sizeof(src->ip_mask));
    memcpy(dest->gw_addr, src->gw_addr, sizeof(src->gw_addr));
    memcpy(dest->mac_addr, src->mac_addr, sizeof(src->mac_addr));
    memcpy(dest->sock.dst_ip, src->sock.dst_ip, sizeof(src->sock.dst_ip));
    dest->sock.local_port = src->sock.local_port;
    dest->sock.dst_port = src->sock.dst_port;
    dest->sock.mode = src->sock.mode;
}
