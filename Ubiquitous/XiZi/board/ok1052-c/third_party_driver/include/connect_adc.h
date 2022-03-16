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
* @file connect_adc.h
* @brief define imxrt1052-baord adc function and struct
* @version 1.1
* @author AIIT XUOS Lab
* @date 2021-12-28
*/

#ifndef CONNECT_ADC_H
#define CONNECT_ADC_H

#include <device.h>

struct Imrt1052HwAdc
{
    void *ADCx;
    uint8 adc_channel;
};

int Imrt1052HwAdcInit(void);

#endif
