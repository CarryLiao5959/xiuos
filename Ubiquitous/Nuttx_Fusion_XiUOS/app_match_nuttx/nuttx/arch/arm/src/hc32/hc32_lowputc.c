/****************************************************************************
 * arch/arm/src/hc32/hc32_lowputc.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>

#include <arch/board/board.h>

#include "arm_internal.h"
#include "chip.h"

#include "hc32_rcc.h"
#include "hc32_gpio.h"
#include "hc32_uart.h"
#include "hc32_lowputc.h"
#include "hc32f4a0_usart.h"
#include "hc32_ddl.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* UART multiple processor ID definition */
#define UART_MASTER_STATION_ID          (0x20U)
#define UART_SLAVE_STATION_ID           (0x21U)

/* Ring buffer size */
#define IS_RING_BUFFER_EMPTY(x)         (0U == ((x)->u16UsedSize))

/* Multi-processor silence mode */
#define LP_UART_NORMAL_MODE          (0U)
#define LP_UART_SILENCE_MODE         (1U)

/**
 * @brief  Ring buffer structure definition
 */
typedef struct
{
    uint16_t u16Capacity;
    __IO uint16_t u16UsedSize;
    uint16_t u16InIdx;
    uint16_t u16OutIdx;
    uint8_t  au8Buf[50];
} stc_ring_buffer_t;

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static uint8_t m_u8UartSilenceMode = LP_UART_NORMAL_MODE;

static stc_ring_buffer_t m_stcRingBuf = {
    .u16InIdx = 0U,
    .u16OutIdx = 0U,
    .u16UsedSize = 0U,
    .u16Capacity = sizeof (m_stcRingBuf.au8Buf),
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * @brief  Set silence mode.
 * @param  [in] u8Mode                      Silence mode
 *         This parameter can be one of the following values:
 *           @arg LP_UART_SILENCE_MODE:  UART silence mode
 *           @arg LP_UART_NORMAL_MODE:   UART normal mode
 * @retval None
 */
static void UsartSetSilenceMode(uint8_t u8Mode)
{
    m_u8UartSilenceMode = u8Mode;
}

/**
 * @brief  Get silence mode.
 * @param  [in] None
 * @retval Returned value can be one of the following values:
 *           @arg LP_UART_SILENCE_MODE:  UART silence mode
 *           @arg LP_UART_NORMAL_MODE:   UART normal mode
 */
static uint8_t UsartGetSilenceMode(void)
{
    return m_u8UartSilenceMode;
}

/**
 * @brief  Instal IRQ handler.
 * @param  [in] pstcConfig      Pointer to struct @ref stc_irq_signin_config_t
 * @param  [in] u32Priority     Interrupt priority
 * @retval None
 */
static void InstalIrqHandler(const stc_irq_signin_config_t *pstcConfig,
                                    uint32_t u32Priority)
{
    if (NULL != pstcConfig)
    {
        (void)INTC_IrqSignIn(pstcConfig);
        NVIC_ClearPendingIRQ(pstcConfig->enIRQn);
        NVIC_SetPriority(pstcConfig->enIRQn, u32Priority);
        NVIC_EnableIRQ(pstcConfig->enIRQn);
    }
}


/**
 * @brief  Write ring buffer.
 * @param  [in] pstcBuffer              Pointer to a @ref stc_ring_buffer_t structure
 * @param  [in] pu8Data                 Pointer to data buffer to read
 * @retval An en_result_t enumeration value:
 *           - Ok: Write success.
 *           - ErrorNotReady: Buffer is empty.
 */
static en_result_t RingBufRead(stc_ring_buffer_t *pstcBuffer, uint8_t *pu8Data)
{
    en_result_t enRet = Ok;

    if (pstcBuffer->u16UsedSize == 0U)
    {
        enRet = ErrorNotReady;
    }
    else
    {
        *pu8Data = pstcBuffer->au8Buf[pstcBuffer->u16OutIdx++];
        pstcBuffer->u16OutIdx %= pstcBuffer->u16Capacity;
        pstcBuffer->u16UsedSize--;
    }

    return enRet;
}

/**
 * @brief  UART TX Empty IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_TxEmpty_IrqCallback(void)
{
    uint8_t u8Data = 0U;
    en_flag_status_t enFlag = USART_GetStatus(LP_UNIT, USART_FLAG_TXE);
    en_functional_state_t enState = USART_GetFuncState(LP_UNIT, USART_INT_TXE);

    if ((Set == enFlag) && (Enable == enState))
    {
        USART_SendId(LP_UNIT, UART_SLAVE_STATION_ID);

        while (Reset == USART_GetStatus(LP_UNIT, USART_FLAG_TC))   /* Wait Tx data register empty */
        {
        }

        if (Ok == RingBufRead(&m_stcRingBuf, &u8Data))
        {
            USART_SendData(LP_UNIT, (uint16_t)u8Data);
        }

        if (IS_RING_BUFFER_EMPTY(&m_stcRingBuf))
        {
            USART_FuncCmd(LP_UNIT, USART_INT_TXE, Disable);
            USART_FuncCmd(LP_UNIT, USART_INT_TC, Enable);
        }
    }
}

/**
 * @brief  UART TX Complete IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_TxComplete_IrqCallback(void)
{
    en_flag_status_t enFlag = USART_GetStatus(LP_UNIT, USART_FLAG_TC);
    en_functional_state_t enState = USART_GetFuncState(LP_UNIT, USART_INT_TC);

    if ((Set == enFlag) && (Enable == enState))
    {
        /* Disable TX function */
        USART_FuncCmd(LP_UNIT, (USART_TX | USART_RX | USART_INT_TC), Disable);

        /* Enable RX function */
        USART_FuncCmd(LP_UNIT, (USART_RX | USART_INT_RX), Enable);
    }
}

/**
 * @brief  Write ring buffer.
 * @param  [in] pstcBuffer              Pointer to a @ref stc_ring_buffer_t structure
 * @param  [in] u8Data                  Data to write
 * @retval An en_result_t enumeration value:
 *           - Ok: Write success.
 *           - ErrorBufferFull: Buffer is full.
 */
static en_result_t write_ring_buf(stc_ring_buffer_t *pstcBuffer, uint8_t u8Data)
{
    en_result_t enRet = Ok;

    if (pstcBuffer->u16UsedSize >= pstcBuffer->u16Capacity)
    {
        enRet = ErrorBufferFull;
    }
    else
    {
        pstcBuffer->au8Buf[pstcBuffer->u16InIdx++] = u8Data;
        pstcBuffer->u16InIdx %= pstcBuffer->u16Capacity;
        pstcBuffer->u16UsedSize++;
    }

    return enRet;
}

/**
 * @brief  UART RX IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_Rx_IrqCallback(void)
{
    uint8_t u8RxData;
    en_flag_status_t enFlag = USART_GetStatus(LP_UNIT, USART_FLAG_RXNE);
    en_functional_state_t enState = USART_GetFuncState(LP_UNIT, USART_INT_RX);

    if ((Set == enFlag) && (Enable == enState))
    {
        u8RxData = (uint8_t)USART_RecData(LP_UNIT);

        if ((Reset == USART_GetStatus(LP_UNIT, USART_FLAG_MPB)) &&
            (LP_UART_NORMAL_MODE == UsartGetSilenceMode()))
        {
            write_ring_buf(&m_stcRingBuf, u8RxData);
        }
        else
        {
            if (UART_MASTER_STATION_ID != u8RxData)
            {
                USART_SilenceCmd(LP_UNIT, Enable);
                UsartSetSilenceMode(LP_UART_SILENCE_MODE);
            }
            else
            {
                UsartSetSilenceMode(LP_UART_NORMAL_MODE);
            }
        }
    }
}

/**
 * @brief  UART RX Error IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_RxErr_IrqCallback(void)
{
    USART_ClearStatus(LP_UNIT, (USART_CLEAR_FLAG_FE | USART_CLEAR_FLAG_PE | USART_CLEAR_FLAG_ORE));
}

void hc32_unlock(void)
{
    /* Unlock GPIO register: PSPCR, PCCR, PINAER, PCRxy, PFSRxy */
    GPIO_Unlock();
    /* Unlock PWC register: FCG0 */
    PWC_FCG0_Unlock();
    /* Unlock PWC, CLK, PVD registers, @ref PWC_REG_Write_Unlock_Code for details */
    PWC_Unlock(PWC_UNLOCK_CODE_0);
    /* Unlock SRAM register: WTCR */
    SRAM_WTCR_Unlock();
    /* Unlock SRAM register: CKCR */
    // SRAM_CKCR_Unlock();
    /* Unlock all EFM registers */
    EFM_Unlock();
}

void hc32_lock(void)
{
    /* Lock GPIO register: PSPCR, PCCR, PINAER, PCRxy, PFSRxy */
    GPIO_Lock();
    /* Lock PWC register: FCG0 */
    PWC_FCG0_Lock();
    /* Lock PWC, CLK, PVD registers, @ref PWC_REG_Write_Unlock_Code for details */
    PWC_Lock(PWC_UNLOCK_CODE_0);
    /* Lock SRAM register: WTCR */
    SRAM_WTCR_Lock();
    /* Lock SRAM register: CKCR */
    // SRAM_CKCR_Lock();
    /* Lock EFM OTP write protect registers */
    // EFM_OTP_WP_Lock();
    /* Lock EFM register: FWMC */
    // EFM_FWMC_Lock();
    /* Lock all EFM registers */
    EFM_Lock();
}

void hc32_print_portinit(void)
{
    GPIO_SetFunc(BSP_PRINTF_PORT, BSP_PRINTF_PIN, BSP_PRINTF_PORT_FUNC, PIN_SUBFUNC_DISABLE);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: arm_lowputc
 *
 * Description:
 *   Output one byte on the serial console
 *
 ****************************************************************************/

void arm_lowputc(char ch)
{
    while(Set != USART_GetStatus(LP_UNIT, USART_FLAG_TXE));
    USART_SendData(LP_UNIT, ch);
}


/****************************************************************************
 * Name: hc32_lowsetup
 *
 * Description:
 *   This performs basic initialization of the USART used for the serial
 *   console.  Its purpose is to get the console output available as soon
 *   as possible.
 *
 ****************************************************************************/

void hc32_lowsetup(void)
{
    stc_irq_signin_config_t stcIrqSigninCfg;
    const stc_usart_multiprocessor_init_t stcUartMultiProcessorInit = {
        .u32Baudrate = 115200UL,
        .u32BitDirection = USART_LSB,
        .u32StopBit = USART_STOPBIT_1BIT,
        .u32DataWidth = USART_DATA_LENGTH_8BIT,
        .u32ClkMode = USART_INTERNCLK_NONE_OUTPUT,
        .u32PclkDiv = USART_PCLK_DIV64,
        .u32OversamplingBits = USART_OVERSAMPLING_8BIT,
        .u32NoiseFilterState = USART_NOISE_FILTER_DISABLE,
        .u32SbDetectPolarity = USART_SB_DETECT_FALLING,
    };

    hc32_unlock();

    hc32_clk_config();

    /* Initialize UART for debug print function. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, hc32_print_portinit);

    /* Configure USART RX/TX pin. */
    GPIO_SetFunc(LP_RX_PORT, LP_RX_PIN, LP_RX_GPIO_FUNC, PIN_SUBFUNC_DISABLE);
    GPIO_SetFunc(LP_TX_PORT, LP_TX_PIN, LP_TX_GPIO_FUNC, PIN_SUBFUNC_DISABLE);

    hc32_lock();

    /* Enable peripheral clock */
    PWC_Fcg3PeriphClockCmd(LP_FUNCTION_CLK_GATE, Enable);

    /* Set silence mode */
    UsartSetSilenceMode(LP_UART_SILENCE_MODE);

    /* Initialize UART function. */
    (void)USART_MultiProcessorInit(LP_UNIT, &stcUartMultiProcessorInit);

    /* Register error IRQ handler && configure NVIC. */
    stcIrqSigninCfg.enIRQn = LP_UNIT_ERR_INT_IRQn;
    stcIrqSigninCfg.enIntSrc = LP_UNIT_ERR_INT_SRC;
    stcIrqSigninCfg.pfnCallback = &USART_RxErr_IrqCallback;
    InstalIrqHandler(&stcIrqSigninCfg, DDL_IRQ_PRIORITY_DEFAULT);

    /* Register RX IRQ handler && configure NVIC. */
    stcIrqSigninCfg.enIRQn = LP_UNIT_RX_INT_IRQn;
    stcIrqSigninCfg.enIntSrc = LP_UNIT_RX_INT_SRC;
    stcIrqSigninCfg.pfnCallback = &USART_Rx_IrqCallback;
    InstalIrqHandler(&stcIrqSigninCfg, DDL_IRQ_PRIORITY_00);

    /* Register TX IRQ handler && configure NVIC. */
    stcIrqSigninCfg.enIRQn = LP_UNIT_TX_INT_IRQn;
    stcIrqSigninCfg.enIntSrc = LP_UNIT_TX_INT_SRC;
    stcIrqSigninCfg.pfnCallback = &USART_TxEmpty_IrqCallback;
    InstalIrqHandler(&stcIrqSigninCfg, DDL_IRQ_PRIORITY_DEFAULT);

    /* Register TC IRQ handler && configure NVIC. */
    stcIrqSigninCfg.enIRQn = LP_UNIT_TCI_INT_IRQn;
    stcIrqSigninCfg.enIntSrc = LP_UNIT_TCI_INT_SRC;
    stcIrqSigninCfg.pfnCallback = &USART_TxComplete_IrqCallback;
    InstalIrqHandler(&stcIrqSigninCfg, DDL_IRQ_PRIORITY_DEFAULT);
}

