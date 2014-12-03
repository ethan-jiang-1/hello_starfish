/******************** (C) COPYRIGHT 2014 FRDM **********************************
 * File Name          : uart.c
 * Author             : Bruce.zhu
 * Date First Issued  : 2014/02/21
 * Description        : This file contains the software implementation for the
 *                      uart dma route unit
 *******************************************************************************
 * History:
 * DATE       | VER   | AUTOR      | Description
 * 2014/02/23 | v1.0  | Bruce.zhu  | initial released
 *                                 | reference: low_power_dma_uart_demo.c
 *******************************************************************************/

#ifndef _UART0_H_
#define _UART0_H_

/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
/* Include inherited beans */

#include "PE_LDD.h"
#include "UART0_PDD.h"
#include "UART_PDD.h"
#include "Cpu.h"


void    init_uart0(INT_ISR_FPTR isr_handle);
void    init_uart1(INT_ISR_FPTR isr_handle);

void    uart_send_data(UART_MemMapPtr uartch, uint8 ch);
void    uart_send_string(UART_MemMapPtr uartch, uint8* p_str);

void    uart0_send_data(uint8 ch);
void    uart0_send_string(uint8* p_str);










#endif /* _UART0_H_ */

