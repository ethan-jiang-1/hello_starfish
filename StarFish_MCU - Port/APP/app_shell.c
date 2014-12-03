/******************** (C) COPYRIGHT 2014 FRDM **********************************
 * File Name          : app_shell.c
 * Author             : Bruce.zhu
 * Date First Issued  : 2014/02/21
 * Description        : This file contains the software implementation for the
 *                      shell control unit
 *******************************************************************************
 * History:
 * DATE       | VER   | AUTOR      | Description
 * 2014/02/21 | v1.0  | Bruce.zhu  | initial released
 *******************************************************************************/

#include "app_shell.h"
#include "app_trace.h"

#include "MQX1.h"

/* MQX Lite include files */
#include "mqxlite.h"
#include "mqxlite_prv.h"
#include "mutex.h"
#include "lwmsgq.h"

#include "app_trace.h"
#include <string.h>
#include <stdlib.h>


uint8_t shell_task_stack[SHELL_TASK_STACK_SIZE];

#define  SHELL_HEADER       "app_"

#define CMD_BUF_SIZE        32

#define SHELL_MSG_NUMBER    64
#define SHELL_MSG_SIZE      1
static uint_32  g_uart2_rx_queue[sizeof(LWMSGQ_STRUCT)/sizeof(uint_32) + SHELL_MSG_NUMBER* SHELL_MSG_SIZE];

static CPU_INT16S cmd_list(CPU_INT16U i, CPU_CHAR  **p_p, SHELL_OUT_FNCT p_f, SHELL_CMD_PARAM* p);
static CPU_INT16S get_mqxlite_status(CPU_INT16U i, CPU_CHAR  **p_p, SHELL_OUT_FNCT p_f, SHELL_CMD_PARAM* p);



static SHELL_CMD App_ShellAppCmdTbl[] =
{
    // show all the commands on the console
    {SHELL_HEADER"list"         ,cmd_list               },
    // get mqx-lite task status
    {SHELL_HEADER"top"          ,get_mqxlite_status     },

	{0                          ,0                      }
};


/**
  * show all support commands
  *
  */
static CPU_INT16S cmd_list(CPU_INT16U i, CPU_CHAR  **p_p, SHELL_OUT_FNCT p_f, SHELL_CMD_PARAM* p)
{
	SHELL_MODULE_CMD* p_module_cmd;
	SHELL_CMD*        p_cmd;
	uint32_t          cmd_pos = 0;

	APP_TRACE("------------ support commands ------------\r\n");
	p_module_cmd = Shell_ModuleCmdUsedPoolPtr;
	while(p_module_cmd != 0)
	{
		cmd_pos = 0;
		p_cmd = p_module_cmd->CmdTblPtr;
		while(p_cmd[cmd_pos].Name)
		{
			APP_TRACE("%s\r\n", p_cmd[cmd_pos].Name);
			cmd_pos++;
		}

		p_module_cmd = p_module_cmd->NextModuleCmdPtr;
	}

	return 0;
}


static int get_task_stack_usage
(
    _task_id task_id,
    _mem_size_ptr stack_size_ptr,
    _mem_size_ptr stack_used_ptr
)
{
    TD_STRUCT_PTR td_ptr;
    _mqx_uint_ptr stack_ptr;
    _mqx_uint_ptr stack_base;

    td_ptr = (TD_STRUCT_PTR)_task_get_td(task_id);

    if (td_ptr == NULL)
    {
        return(-1);
    }

    stack_base = (_mqx_uint_ptr)td_ptr->STACK_BASE;
    stack_ptr = (_mqx_uint_ptr)td_ptr->STACK_LIMIT;
    while (stack_ptr < stack_base)
    {
        if (*stack_ptr != MQX_STACK_MONITOR_VALUE)
        {
            break;
        }
        ++stack_ptr;
    }

    *stack_used_ptr = (_mqx_uint)((uchar_ptr)td_ptr->STACK_BASE -
                    (uchar_ptr)stack_ptr);
    *stack_size_ptr = (_mqx_uint)((uchar_ptr)td_ptr->STACK_BASE -
                    (uchar_ptr)td_ptr->STACK_LIMIT);

    return(0);
}



static void show_mqxlite_task_information(void)
{
    KERNEL_DATA_STRUCT_PTR  kernel_data;
    TD_STRUCT_PTR           td_ptr;
    _mqx_uint               size;
    int                     i = 1;
    _mem_size               stack_size;
    _mem_size               stack_used;

	APP_TRACE(" %-4s %-16s%5s%6s%11s\r\n", "NUM", "TASK", "PRIO", "ID", "Stack used");
    APP_TRACE("--------------------------------------------------\r\n");

    _GET_KERNEL_DATA(kernel_data);

    td_ptr = (TD_STRUCT_PTR)((uchar_ptr)kernel_data->TD_LIST.NEXT -
             FIELD_OFFSET(TD_STRUCT,TD_LIST_INFO));
    size   = _QUEUE_GET_SIZE(&kernel_data->TD_LIST);

    while (size && td_ptr)
    {
        get_task_stack_usage(td_ptr->TASK_ID, &stack_size, &stack_used);

        // num name priority id [stack size,stack used]
        APP_TRACE(" [%2d] %-16s%5d%6d [%4d,%4d]\r\n",
                    i++,
                    td_ptr->TASK_TEMPLATE_PTR->TASK_NAME,
                    td_ptr->TASK_TEMPLATE_PTR->TASK_PRIORITY,
                    td_ptr->TASK_ID,
                    stack_size,
                    stack_used
                  );

        size--;
        td_ptr = (TD_STRUCT_PTR)((uchar_ptr)(td_ptr->TD_LIST_INFO.NEXT) -
                 FIELD_OFFSET(TD_STRUCT,TD_LIST_INFO));
    }

}


static CPU_INT16S get_mqxlite_status(   CPU_INT16U argc,
										        CPU_CHAR  **argv,
										        SHELL_OUT_FNCT pout_fnct,
										        SHELL_CMD_PARAM* pcmd_param)
{
	switch (argc)
	{
		case 1:
			show_mqxlite_task_information();
			break;

        case 2:
            if (strcmp("-h", argv[1]) == 0)
            {
            APP_TRACE("top usage:\r\n"
                	  "     get MQX-lite run information\r\n"
                	  "-h   get help information\r\n");
            }
            break;

        default:
            break;
	}

    return 0;
}


void    app_cmd_tbl_add (void)
{
	SHELL_ERR err;

	APP_TRACE("Adding app shell command table ... ");
	Shell_CmdTblAdd("app", App_ShellAppCmdTbl, &err);
	if (err == SHELL_ERR_NONE) {
		APP_TRACE("done.\n\r");
	} else {
		APP_TRACE("failed.\n\r");
	}
}



static void exec_cmd(uint8_t* p_buf)
{
	SHELL_ERR shell_err;
	uint8_t   cmd_buf[CMD_BUF_SIZE + 10];

	snprintf((char*)cmd_buf, sizeof(cmd_buf), "%s%s", SHELL_HEADER, p_buf);
	Shell_Exec((CPU_CHAR*)cmd_buf, (SHELL_OUT_FNCT)0, (SHELL_CMD_PARAM*)0, &shell_err);
	switch(shell_err)
	{
		case SHELL_ERR_NONE:
			break;
		case SHELL_ERR_NULL_PTR:
			APP_TRACE("Error, NULL pointer passed.");
			break;
		case SHELL_ERR_CMD_NOT_FOUND:
			APP_TRACE("Error, command NOT found.");
			break;
		case SHELL_ERR_CMD_SEARCH:
			APP_TRACE("Error, searching command.");
			break;
		case SHELL_ERR_ARG_TBL_FULL:
			APP_TRACE("Error, too many arguments.");
			break;
		case SHELL_ERR_CMD_EXEC:
			APP_TRACE("Error, executing command.");
			break;
		default:
			break;
	}

}



void    shell_task(uint32_t task_init_data)
{
    uint8_t         cmd_buf[CMD_BUF_SIZE];
    uint8_t         cmd_len = 0;
    uint8_t         cmd;
    uint8_t         active_flag = 0;
    _mqx_uint       ret;
    _mqx_max_type   msg;

    // create recv msg queue
    ret = _lwmsgq_init(g_uart2_rx_queue, SHELL_MSG_NUMBER, SHELL_MSG_SIZE);
    ASSERT_PARAM(MQX_OK == ret);

    Shell_Init();
    app_cmd_tbl_add();

    _time_delay_ticks(40);

    APP_TRACE("\r\n>>>>> Type Enter key to active shell console. <<<<<");
    APP_TRACE("\r\n>>>>>   Type CTRL+C to exit shell console.    <<<<<\r\n");

    while (1)
    {
        ret = _lwmsgq_receive(g_uart2_rx_queue, &msg, LWMSGQ_RECEIVE_BLOCK_ON_EMPTY, 0, NULL);
        ASSERT_PARAM(MQX_OK == ret);

        cmd = (uint8_t)msg;

        // check shell module active
        if(active_flag == 0)
        {
            if (cmd == 0x0d)
            {
                APP_TRACE("\r\nFRDM>");
                active_flag = 1;
            }
            else
            {
                APP_TRACE("Type Enter key to active this shell console...\r\n");
            }

            continue;
        }

    	switch (cmd)
        {
        	// backspace
            case '\b':
                if(cmd_len > 0)
                {
                    cmd_len--;
                    APP_TRACE("%c%c%c", '\b', ' ', '\b');
                }
                break;

            // CTRL + C ----> exit shell console
            case 0x03:
                active_flag = 0;
                APP_TRACE("\r\nExit shell console.\r\n");
                cmd_len = 0;
                break;

        	case 0x0d:
                if(cmd_len == 0)
                {
                    APP_TRACE("\r\nFRDM>");
                    break;
                }
                else
                {
                    cmd_buf[cmd_len] = 0;
                    APP_TRACE("\r\n");

                    // exec cmd
                    exec_cmd(cmd_buf);

                    APP_TRACE("\r\nFRDM>");
                    cmd_len = 0;
                }
                break;

        	default:
                if (cmd_len >= 64)
                {
                    APP_TRACE("\r\nError, cmd is too long.\r\n");
                    APP_TRACE("FRDM>");
                    cmd_len = 0;
                }
                else
                {
                    cmd_buf[cmd_len++] = cmd;
                    APP_TRACE("%c", cmd);
                }
                break;
        }
    }
}


//void    uart0_irq_handler(void* p_arg)
//{
//    _mqx_uint       ret;
//    _mqx_max_type   msg;

//    if (UART0_PDD_ReadInterruptStatusReg(UART0_BASE_PTR) & UART0_S1_RDRF_MASK)
//    {
//        msg = UART0_PDD_GetChar8(UART0_BASE_PTR);

//        ret = _lwmsgq_send(g_uart2_rx_queue, &msg, 0);
//        ASSERT_PARAM(MQX_OK == ret);
//    }
//}

void    uart1_irq_handler(void* p_arg)
{
    _mqx_uint       ret;
    _mqx_max_type   msg;

    if (UART_PDD_ReadInterruptStatusReg(UART1_BASE_PTR) & UART_S1_RDRF_MASK)
    {
        msg = UART_PDD_GetChar8(UART1_BASE_PTR);

        ret = _lwmsgq_send(g_uart2_rx_queue, &msg, 0);
        ASSERT_PARAM(MQX_OK == ret);
    }
}

