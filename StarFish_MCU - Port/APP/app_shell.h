/******************** (C) COPYRIGHT 2014 FRDM **********************************
 * File Name          : app_shell.h
 * Author             : Bruce.zhu
 * Date First Issued  : 2014/02/21
 * Description        : This file contains the software implementation for the
 *                      shell control unit
 *******************************************************************************
 * History:
 * DATE       | VER   | AUTOR      | Description
 * 2014/02/21 | v1.0  | Bruce.zhu  | initial released
 *******************************************************************************/

#ifndef _APP_SHELL_H_
#define _APP_SHELL_H_

#include "shell.h"
#include "psp_prv.h"


void    uart1_irq_handler(void* p_arg);
void    shell_task(uint32_t task_init_data);



#endif /* _APP_SHELL_H_ */

