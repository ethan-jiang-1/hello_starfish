#include "main.h"
#include "qcom_uart.h"
#include "tx_api.h"

extern TX_QUEUE queue_share_uart2wifi;
extern TX_QUEUE queue_share_wifi2uart;

static A_INT32 uart_fd_com = -1;

void UART_Init(void) 
{
    qcom_uart_para com_uart_cfg;

    com_uart_cfg.BaudRate    = 115200;
    com_uart_cfg.number      = 8;
    com_uart_cfg.StopBits    = 1;
    com_uart_cfg.parity      = 0;
    com_uart_cfg.FlowControl = 0;

    qcom_uart_init();

    uart_fd_com = qcom_uart_open((A_CHAR *)"UART1");

    qcom_set_uart_config((A_CHAR *)"UART1", &com_uart_cfg);
}

void UART_SendTask(A_UINT32 arg) 
{
    MEM_SHARE_t shareMem;

    while (1) {

        UINT status = tx_queue_receive(&queue_share_wifi2uart, 
                                       &shareMem, 
                                       TX_WAIT_FOREVER);

        if (TX_SUCCESS == status) {

            while (shareMem.len > 0) {
                shareMem.len = qcom_uart_write(uart_fd_com, (A_CHAR *)shareMem.pBuff, &shareMem.len); 
            }

            qcom_mem_free(shareMem.pBuff);
        } 
    }
}

void UART_RecvTask(A_UINT32 arg) 
{
    INT fd_act;
    q_fd_set fd_set;
    A_CHAR uartRxBuf[LEN_UART_BUFF] = { 0 };
    A_UINT32 uartLen;
    MEM_SHARE_t shareMem;

    struct timeval uartTmo;
    uartTmo.tv_sec  = 3;
    uartTmo.tv_usec = 0;

    while (1) {

        FD_ZERO(&fd_set);
        FD_SET(uart_fd_com, &fd_set);

        fd_act = qcom_select((uart_fd_com + 1), &fd_set, NULL, NULL, &uartTmo);

        if (fd_act > 0) {

            if (FD_ISSET(uart_fd_com, &fd_set)) {

                // Read the data from uart
                uartLen = LEN_UART_BUFF;
                uartLen = qcom_uart_read(uart_fd_com, uartRxBuf, &uartLen);

                // Alloc the memory
                shareMem.len   = uartLen;
                shareMem.pBuff = qcom_mem_alloc(uartLen);

                if (shareMem.pBuff != NULL) {
                    A_MEMCPY(shareMem.pBuff, uartRxBuf, uartLen);
                    tx_queue_send(&queue_share_uart2wifi, &shareMem, TX_WAIT_FOREVER);
                }
            }
        } 
    }
}
