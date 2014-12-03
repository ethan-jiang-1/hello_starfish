/******************** (C) COPYRIGHT 2014 FRDM **********************************
 * File Name          : app_trace.c
 * Author             : Bruce.zhu
 * Date First Issued  : 2014/02/21
 * Description        : This file contains the software implementation for the
 *                      trace unit
 *******************************************************************************
 * History:
 * DATE       | VER   | AUTOR      | Description
 * 2014/03/02 | v1.0  | Bruce.zhu  | initial released
 *******************************************************************************/

#include "app_trace.h"
#include "app_shell.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "MQX1.h"

/* MQX Lite include files */
#include "mqxlite.h"
#include "mqxlite_prv.h"
#include "mutex.h"


static uint8_t      isr_output_msg[64];
static uint8_t      output_msg[128];
static MUTEX_STRUCT g_trace_mutex;


/**
  * @brief: USE UART2 to output debug message
  *
  */
void app_traceInfo_output(const char* fmt, ...)
{
    va_list     ap;
    _mqx_uint   err;

    err = _mutex_lock(&g_trace_mutex);
    ASSERT_PARAM(MQX_EOK == err);

    va_start(ap, fmt);
    vsnprintf((char*)output_msg, sizeof(output_msg), fmt, ap);
    va_end(ap);

    //uart0_send_string(output_msg);
		uart_send_string(UART1_BASE_PTR, output_msg);
    err = _mutex_unlock(&g_trace_mutex);
    ASSERT_PARAM(MQX_EOK == err);
}


/**
  * Displays isr debug messages on the console
  * Note: This function should be called from isr
  */
void traceInfo_output(const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsnprintf((char*)isr_output_msg, sizeof(isr_output_msg), fmt, ap);
    va_end(ap);

   // uart0_send_string(isr_output_msg);
		uart_send_string(UART1_BASE_PTR, isr_output_msg);
}




/**
  * Displays a hex dump on the debug console (16 bytes per line)
  * \param pbData     Pointer to dump data
  * \param ulDataLen  Length of data dump
  */
void    DumpData(uint8_t* pbData, uint32_t ulDataLen)
{
	uint8_t  vl_buf[16];
	uint32_t ulIdx = 0;
	char     word[16];
	uint8_t  i = 0;

	for(ulIdx = 0; ulIdx < ulDataLen; ++ulIdx)
	{
		if(0 == (ulIdx % 16) && ulIdx)
		{
			for (i = 0; i < 16; i++)
			{
				//uart0_send_data(word[i]);
				uart_send_data(UART1_BASE_PTR, word[i]);
			}
			//uart0_send_string("\r\n");
			uart_send_string(UART1_BASE_PTR, "\r\n");
		}

		if (pbData[ulIdx] > 31 && pbData[ulIdx] < 127)
		{
			word[ulIdx%16] = pbData[ulIdx];
		}
		else
		{
			word[ulIdx%16] = '.';
		}

		snprintf((char*)vl_buf, sizeof(vl_buf), "%02X ", pbData[ulIdx]);
		//uart0_send_string(vl_buf);
		uart_send_string(UART1_BASE_PTR, vl_buf);
	}

	if ((ulIdx % 16) == 0)
		ulIdx = 16;
	else
		ulIdx = ulIdx % 16;

	for (i = 0; i < ulIdx; i++)
	{
		//uart0_send_data(word[i]);
		uart_send_data(UART1_BASE_PTR, word[i]);
	}

//	uart0_send_string("\r\n");
	uart_send_string(UART1_BASE_PTR, "\r\n");
}


/**
  * @brief: UART2 must be init before call this function
  *
  */
void trace_init(void)
{
    _mqx_uint err;

    err = _mutex_init(&g_trace_mutex, NULL);
    ASSERT_PARAM(err == MQX_EOK);

   // init_uart0(uart0_irq_handler);
	init_uart1(uart1_irq_handler);
}



void assert_failed(uint8_t* file, uint32_t line)
{
    TRACE("==> [%d] - [%s]\r\n", line, file);

    /* Infinite loop */
    while (1)
    {
    }
}




