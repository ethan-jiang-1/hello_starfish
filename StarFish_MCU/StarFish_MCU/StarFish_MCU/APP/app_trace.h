/******************** (C) COPYRIGHT 2014 FRDM **********************************
 * File Name          : app_trace.h
 * Author             : Bruce.zhu
 * Date First Issued  : 2014/02/21
 * Description        : This file contains the software implementation for the
 *                      trace unit
 *******************************************************************************
 * History:
 * DATE       | VER   | AUTOR      | Description
 * 2014/03/02 | v1.0  | Bruce.zhu  | initial released
 *******************************************************************************/

#ifndef _APP_TRACE_H_
#define _APP_TRACE_H_

#include "uart.h"


/*---------------------
               USE UART2 TO DEUBG
                     ---------------------*/

/* task must call this */
#define APP_TRACE(...)          app_traceInfo_output(__VA_ARGS__)
/* ISR call */
#define TRACE(...)              traceInfo_output(__VA_ARGS__)
/* ASSERT */
#define ASSERT_PARAM(expr)      ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* dump data */
#define DUMP_DATA(str, len)     DumpData(str, len)

void    trace_init(void);
void    app_traceInfo_output(const char* fmt, ...);
void    traceInfo_output(const char* fmt, ...);
void    assert_failed(uint8_t* file, uint32_t line);
void    DumpData(uint8_t* pbData, uint32_t ulDataLen);





#endif /* _APP_TRACE_H_ */


