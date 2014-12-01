/******************** (C) COPYRIGHT 2014 FRDM **********************************
 * File Name          : uart.c
 * Author             : Bruce.zhu
 * Date First Issued  : 2014/02/23
 * Description        : This file contains the software implementation for the
 *                      uart dma HAL layer unit
 *******************************************************************************
 * History:
 * DATE       | VER   | AUTOR      | Description
 * 2014/02/23 | v1.0  | Bruce.zhu  | initial released
 *                                 | reference: low_power_dma_uart_demo.c
 *******************************************************************************/

#include "common.h"
#include "uart.h"
#include "nvic.h"

#include "MQX1.h"

/* MQX Lite include files */
#include "mqxlite.h"
#include "mqxlite_prv.h"
#include "IO_Map.h"



/********************************************************************/
/*
 * Initialize the uart for 8N1 operation, interrupts disabled, and
 * no hardware flow-control
 *
 * NOTE: Since the uarts are pinned out in multiple locations on most
 *       Kinetis devices, this driver does not enable uart pin functions.
 *       The desired pins should be enabled before calling this init function.
 *
 * Parameters:
 *  uartch      uart channel to initialize(UART1 or UART2)
 *  sysclk      uart module Clock in kHz(used to calculate baud)
 *  baud        uart baud rate
 */
void    uart_init(UART_MemMapPtr uartch, int sysclk, int baud)
{
    register uint16 sbr;
    uint8 temp;

    if (uartch == UART1_BASE_PTR)
        SIM_SCGC4 |= SIM_SCGC4_UART1_MASK;
    else if (uartch == UART2_BASE_PTR)
        SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;
    else
        return;

    /*
     * Make sure that the transmitter and receiver are disabled while we
     * change settings.
     */
    UART_C2_REG(uartch) &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK );

    /* Configure the uart for 8-bit mode, no parity */
    /* We need all default settings, so entire register is cleared */
    UART_C1_REG(uartch) = 0;

    /* Calculate baud settings */
    sbr = (uint16)((sysclk*1000)/(baud * 16));

    /* Save off the current value of the uartx_BDH except for the SBR field */
    temp = UART_BDH_REG(uartch) & ~(UART_BDH_SBR(0x1F));

    UART_BDH_REG(uartch) = temp |  UART_BDH_SBR(((sbr & 0x1F00) >> 8));
    UART_BDL_REG(uartch) = (uint8)(sbr & UART_BDL_SBR_MASK);

    /* Enable receiver and transmitter */
    UART_C2_REG(uartch) |= (UART_C2_TE_MASK | UART_C2_RE_MASK );

}


/**
  * @brief: init UART2 with RX interruput feature
  * @note:  PET22 --- UART0_TX
  *         PET23 --- UART0_RX
  *
  * @retval: none
  */
void    init_uart2(INT_ISR_FPTR isr_handle)
{
    /* SIM_SCGC4: UART0=1 */
    SIM_SCGC4 |= SIM_SCGC4_UART0_MASK;

    // PET22 -- UART0_TX
    // PET23 -- UART0_RX
    // ALT4
    /* PORTE_PCR22: ISF=0, MUX=2 */
    PORTE_PCR22 = (uint32_t)((PORTE_PCR22 & (uint32_t)~(uint32_t)(
                               PORT_PCR_ISF_MASK |
                               PORT_PCR_MUX(0x03)
                               )) | (uint32_t)(
                               PORT_PCR_MUX(0x04)
                               ));

    /* PORTE_PCR23: ISF=0, MUX=2 */
    PORTE_PCR23 = (uint32_t)((PORTE_PCR23 & (uint32_t)~(uint32_t)(
                               PORT_PCR_ISF_MASK |
                               PORT_PCR_MUX(0x03)
                               )) | (uint32_t)(
                               PORT_PCR_MUX(0x04)
                               ));

    //UART clock frequency will equal half the PLL frequency
    uart_init(UART2_BASE_PTR, 48000/2, 115200);

    if (isr_handle)
    {
        UART_PDD_EnableInterrupt(UART2_BASE_PTR, UART_C2_RIE_MASK);
        _int_install_isr(LDD_ivIndex_INT_UART2, isr_handle, NULL);
        // ENABLE UART2 RX interrupt
        enable_irq(14);
        set_irq_priority(14, 2);
    }

}


/**
  * @brief: used for UART1 and UART2
  * @param: uartch  UART1 --> UART1_BASE_PTR
  *                 UART2 --> UART2_BASE_PTR
  * @param: ch, the 8 bit char
  */
void    uart_send_data(UART_MemMapPtr uartch, uint8 ch)
{
    /* Wait until space is available in the FIFO */
    while(!(UART_S1_REG(uartch) & UART_S1_TDRE_MASK));
    /* Send the character */
    UART_D_REG(uartch) = ch;
}


/**
  * @brief: used for UART1 and UART2
  *
  */
void    uart_send_string(UART_MemMapPtr uartch, uint8* p_str)
{
    uint32 i = 0;

    while (p_str[i])
    {
        uart_send_data(uartch, p_str[i]);
        i++;
    }
}


/**
  * @brief: used for UART0 UART1 and UART2
  * @param: uartch  UART0 --> UART0_BASE_PTR
  *                 UART1 --> UART1_BASE_PTR
  *                 UART2 --> UART2_BASE_PTR
  * @param: ch, the 8 bit char
  */
void    uart0_send_data(uint8 ch)
{
    while (UART0_PDD_GetTxCompleteStatus(UART0_BASE_PTR) == 0) {};

    UART0_PDD_PutChar8(UART0_BASE_PTR, (uint8_t)ch);

}


/**
  * @brief: used for UART1 and UART2
  *
  */
void    uart0_send_string(uint8* p_str)
{
    uint32 i = 0;

    while (p_str[i])
    {
        uart0_send_data(p_str[i]);
        i++;
    }
}


/**
  * @brief: init UART0
  *
  */
void    init_uart0(INT_ISR_FPTR isr_handle)
{
    /* SIM_SCGC4: UART0=1 */
    SIM_SCGC4 |= SIM_SCGC4_UART0_MASK;

    // PET20 -- UART0_TX
    // PET21 -- UART0_RX
    // ALT4
    /* PORTE_PCR20: ISF=0, MUX=2 */
//    PORTE_PCR20 = (uint32_t)((PORTE_PCR20 & (uint32_t)~(uint32_t)(
//                               PORT_PCR_ISF_MASK |
//                               PORT_PCR_MUX(0x03)
//                               )) | (uint32_t)(
//                               PORT_PCR_MUX(0x04)
//                               ));

//    /* PORTE_PCR21: ISF=0, MUX=2 */
//    PORTE_PCR21 = (uint32_t)((PORTE_PCR21 & (uint32_t)~(uint32_t)(
//                               PORT_PCR_ISF_MASK |
//                               PORT_PCR_MUX(0x03)
//                               )) | (uint32_t)(
//                               PORT_PCR_MUX(0x04)
//                               ));
PORTA_PCR1 = (uint32_t)((PORTA_PCR1 & (uint32_t)~(uint32_t)(
                PORT_PCR_ISF_MASK |
                PORT_PCR_MUX(0x05)
               )) | (uint32_t)(
                PORT_PCR_MUX(0x02)
               ));                                                  
  /* PORTA_PCR2: ISF=0,MUX=2 */
  PORTA_PCR2 = (uint32_t)((PORTA_PCR2 & (uint32_t)~(uint32_t)(
                PORT_PCR_ISF_MASK |
                PORT_PCR_MUX(0x05)
               )) | (uint32_t)(
                PORT_PCR_MUX(0x02)
               ));      

    UART0_PDD_EnableTransmitter(UART0_BASE_PTR, PDD_DISABLE); /* Disable transmitter. */
    UART0_PDD_EnableReceiver(UART0_BASE_PTR, PDD_DISABLE); /* Disable receiver. */

    UART0_C1 = 0x00U;                    /*  Set the C1 register */

    UART0_C3 = 0x00U;                    /*  Set the C3 register */

    UART0_S2 = 0x00U;                    /*  Set the S2 register */

    UART0_PDD_SetClockSource(UART0_BASE_PTR, UART0_PDD_PLL_FLL_CLOCK);
    /*
     * Set the baud rate register.
     * 115200
     */
    UART0_PDD_SetBaudRate(UART0_BASE_PTR, 52U);
    UART0_PDD_SetOversamplingRatio(UART0_BASE_PTR, 3U);
    UART0_PDD_EnableSamplingOnBothEdges(UART0_BASE_PTR, PDD_ENABLE);

    //------------------------- UART0 RX INTERRUPT ------------------------
    if (isr_handle)
    {
        // install isr function
        _int_install_isr(LDD_ivIndex_INT_UART0, isr_handle, NULL);

        // ENABLE UART0 RX interrupt
        enable_irq(12);
        set_irq_priority(12, 2);
        UART0_PDD_ClearInterruptFlags(UART0_BASE_PTR, UART0_C2_RIE_MASK);
        UART0_PDD_EnableInterrupt(UART0_BASE_PTR, UART0_C2_RIE_MASK);
    }

    // Enable transmitter
    UART0_PDD_EnableTransmitter(UART0_BASE_PTR, PDD_ENABLE);
    // Enable receiver
    UART0_PDD_EnableReceiver(UART0_BASE_PTR, PDD_ENABLE);

}




