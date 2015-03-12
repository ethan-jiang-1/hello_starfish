#include "main.h"
#include "qcom_uart.h"
#include "tx_api.h"

extern TX_QUEUE queue_share_uart2wifi;
extern TX_QUEUE queue_share_wifi2uart;

A_INT32 uart_fd_com = -1;
A_INT32 uart_mcu_com = -1;
#define WriteLen 6;
A_CHAR uartRxBuf[LEN_UART_BUFF] = { 0 };
extern void Uart_Rx_Pin_Set(int pin0,int pin1);
extern void Uart_Tx_Pin_Set(int pin0,int pin1);
void Uart_Write_Block(A_INT32 fd, A_CHAR* buff, A_UINT32 len);
A_CHAR* Uart_Read_Block(A_INT32 fd);
void UART_Init(void) 
{
    qcom_uart_para com_uart_cfg;

    com_uart_cfg.BaudRate    = 115200;
    com_uart_cfg.number      = 8;
    com_uart_cfg.StopBits    = 1;
    com_uart_cfg.parity      = 0;
    com_uart_cfg.FlowControl = 0;
	Uart_Rx_Pin_Set(10,6);
	Uart_Tx_Pin_Set(11,7);
    qcom_uart_init();

    uart_fd_com = qcom_uart_open((A_CHAR *)"UART0");
	

    qcom_set_uart_config((A_CHAR *)"UART0", &com_uart_cfg);
    
    uart_mcu_com = qcom_uart_open((A_CHAR *)"UART1");
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

void Uart_Write_Block(A_INT32 fd, A_CHAR* buff, A_UINT32 len)
{
			A_UINT32 nSendMCU=WriteLen;
			A_UINT32 nPos=0;
			A_UINT32 nResult=0;
			if(len<nSendMCU)
				{
					nSendMCU=len;
				}
			while (len > 0)
				 {
                 	nResult=qcom_uart_write(fd, &buff[nPos], &nSendMCU); 
					A_PRINTF("nSendMCU:%d,nResult:%d\r\n",nSendMCU,nResult);
					nPos+=nSendMCU;
					len=len-nSendMCU;
					A_PRINTF("len:%d\r\n",len);
					if(len>nSendMCU)
						{
							nSendMCU=WriteLen;
						}
					else
						{
						nSendMCU=len;
						}
						if(nSendMCU==0) //it means HW buf full
							{
								
							}
					//qcom_thread_msleep(500);
		
            		}

}

A_CHAR* Uart_Read_Block(A_INT32 fd )
{
/*
	q_fd_set fd_set;
    A_UINT32 uartLen;
	 A_INT32 fd_act=0;
	A_INT32  nLeft=0;
    struct timeval uartTmo;
    uartTmo.tv_sec  = 5;
    uartTmo.tv_usec = 0;
*/
	  A_UINT32 uartLen;
	A_INT32  nLeft=0;

	A_PRINTF("enter read call\r\n");
while(1)
{
	A_PRINTF("enter while\r\n");
	memset(uartRxBuf,0,uartLen);
	 uartLen = LEN_UART_BUFF;
     nLeft = qcom_uart_read(fd, uartRxBuf, &uartLen);
	 A_PRINTF("uartLen:%d,nLeft:%d\r\n",uartLen,nLeft);
	 A_PRINTF("uartRxBuf:%s\r\n",uartRxBuf);
	 qcom_thread_msleep(5000);

}
/*
while(1)
{
		FD_ZERO(&fd_set);
        FD_SET(fd, &fd_set);
		memset(uartRxBuf,0,uartLen);
        fd_act = qcom_select((fd+ 1), &fd_set, NULL, NULL, &uartTmo);

        if (fd_act > 0) {

            if (FD_ISSET(fd, &fd_set)) {

                // Read the data from uart
                uartLen = LEN_UART_BUFF;
                nLeft = qcom_uart_read(fd, uartRxBuf, &uartLen);
		A_PRINTF("uartLen:%d,nLeft:%d\r\n",uartLen,nLeft);
		A_PRINTF("uartRxBuf:%s\r\n",uartRxBuf);

            }
			break;
        }
		else
		{
			A_PRINTF("receive timeout\r\n");	
		} 
}
*/
		A_PRINTF("exit read call\r\n");
		return uartRxBuf;
}
