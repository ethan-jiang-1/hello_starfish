#include "main.h"
#include "qcom_uart.h"

extern TX_QUEUE queue_share_uart2wifi;
extern TX_QUEUE queue_share_wifi2uart;
extern SYS_CONFIG_t sys_config;

extern void MAIN_ConfigWIFI();

static int socket_client_1 = 0;
static int socket_client_2 = 0;
static int socket_client_3 = 0;
static int socket_client_4 = 0;
extern A_INT32 uart_mcu_com ;
extern A_INT32 uart_fd_com;
#define MAX_CLIENTS (4)

void GetImageFromCloud()
{
 // CLIOTA_Tftp(cli, ip_addr, file_name, partition_index,test_flag);

 
}

void PostDataToCloud()
{

}
void WIFI_Task(A_UINT32 arg)
{
	//A_UINT32 len=5;
	A_PRINTF("start wifi task\r\n");
	 MAIN_ConfigWIFI();
	//A_CHAR * uartstring= "hallo";
	//A_CHAR * uartstring2="warld";
	while(1) {
		//qcom_uart_write(uart_mcu_com,uartstring,&len);
		//qcom_uart_write(uart_fd_com,uartstring2,&len);
		qcom_thread_msleep(1000);
		}
	

}
void WIFI_SendTask(A_UINT32 arg)
{
    MEM_SHARE_t shareMem;

    while (1) {

        UINT status = tx_queue_receive(&queue_share_uart2wifi, 
                                       &shareMem, 
                                       TX_WAIT_FOREVER);

        if (TX_SUCCESS == status) {

            if (socket_client_1 > 0) {
                if(qcom_send(socket_client_1, (A_CHAR *)shareMem.pBuff, shareMem.len, 0) <= 0) {
                    qcom_close(socket_client_1);
                    socket_client_1 = 0; 
                }
            }

            if (socket_client_2 > 0) {
                if(qcom_send(socket_client_2, (A_CHAR *)shareMem.pBuff, shareMem.len, 0) <= 0) {
                    qcom_close(socket_client_2);
                    socket_client_2 = 0; 
                }
            }

            if (socket_client_3 > 0) {
                if(qcom_send(socket_client_3, (A_CHAR *)shareMem.pBuff, shareMem.len, 0) <= 0) {
                    qcom_close(socket_client_3);
                    socket_client_3 = 0; 
                }
            }

            if (socket_client_4 > 0) {
                if(qcom_send(socket_client_4, (A_CHAR *)shareMem.pBuff, shareMem.len, 0) <= 0) {
                    qcom_close(socket_client_4);
                    socket_client_4 = 0; 
                }
            }

            qcom_mem_free(shareMem.pBuff);
        } 
    }
}

static void NotifyUart(A_CHAR *recvBuf, int nRecv)
{
    MEM_SHARE_t shareMem;

    // Alloc the memory
    shareMem.len   = nRecv;
    shareMem.pBuff = qcom_mem_alloc(nRecv);

    if (shareMem.pBuff != NULL) {
        A_MEMCPY(shareMem.pBuff, recvBuf, nRecv);
        tx_queue_send(&queue_share_wifi2uart, &shareMem, TX_WAIT_FOREVER);
    }
}

void WIFI_RecvTask(A_UINT32 arg)
{
    int socketClient = -1;
    int socketServer = -1;

    struct sockaddr_in sockAddr = {0};
    struct timeval tmo;

    int nRecv;
    int ret = -1;
    int fd = 0; 
    int addrLen;

    q_fd_set sockSet;
    struct sockaddr_in clientAddr;

    int port;
    int ipAddr;

    A_CHAR recvBuf[TCP_RX_BUF_LEN] = { 0 };

    MAIN_ConfigWIFI();

    socketServer = qcom_socket(AF_INET, SOCK_STREAM, 0);
    if (socketServer <= 0)
        goto DONE;

    // Init the socket info
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sockAddr.sin_port        = htons(sys_config.port);
    sockAddr.sin_family      = AF_INET;

    ret = qcom_bind(socketServer, 
                    (struct sockaddr *)&sockAddr, 
                    sizeof(struct sockaddr_in));

    if (ret != 0) {
        A_PRINTF("Failed to bind the socket\n");
        goto DONE;
    }

    ret = qcom_listen(socketServer, 10);

    if (ret != 0) {
        A_PRINTF("Failed to listen socket\n");
        goto DONE;
    }

    A_PRINTF("Tcp server is listenning on port %d\n", sys_config.port);

    while (1) {

        /* init fd_set */
        FD_ZERO(&sockSet);
        FD_SET(socketServer, &sockSet);

        // Add new socket
        if (socket_client_1 > 0) {
            FD_SET(socket_client_1, &sockSet);
        }

        if (socket_client_2 > 0) {
            FD_SET(socket_client_2, &sockSet);
        }

        if (socket_client_3 > 0) {
            FD_SET(socket_client_3, &sockSet);
        }

        if (socket_client_4 > 0) {
            FD_SET(socket_client_4, &sockSet);
        }

        tmo.tv_sec  = 5;
        tmo.tv_usec = 0;
        fd = qcom_select(MAX_CLIENTS + 1, &sockSet, NULL, NULL, &tmo);

        if (fd != 0) {

            if (socket_client_1 > 0) {

                if (FD_ISSET(socket_client_1, &sockSet)) {

                    nRecv = qcom_recv(socket_client_1, recvBuf, TCP_RX_BUF_LEN, 0);

                    if (nRecv > 0) {
                        A_PRINTF("Client1 recv recvBuf %s\n", recvBuf);
                        NotifyUart(recvBuf, nRecv);
                    }
                    else {
                        A_PRINTF("Close client 1\n");
                        qcom_close(socket_client_1);
                        socket_client_1 = 0;
                    }
                }
            }

            if (socket_client_2 > 0) {

                if (FD_ISSET(socket_client_2, &sockSet)) {

                    nRecv = qcom_recv(socket_client_2, recvBuf, TCP_RX_BUF_LEN, 0);

                    if (nRecv > 0) {
                        A_PRINTF("Client2 recv recvBuf %s\n", recvBuf);
                        NotifyUart(recvBuf, nRecv);
                    }
                    else {
                        A_PRINTF("Close client 2\n");
                        qcom_close(socket_client_2);
                        socket_client_2 = 0;
                    }
                }
            }

            if (socket_client_3 > 0) {

                if (FD_ISSET(socket_client_3, &sockSet)) {

                    nRecv = qcom_recv(socket_client_3, recvBuf, TCP_RX_BUF_LEN, 0);

                    if (nRecv > 0) {
                        A_PRINTF("Client3 recv recvBuf %s\n", recvBuf);
                        NotifyUart(recvBuf, nRecv);
                    }
                    else {
                        A_PRINTF("close client 3\n");
                        qcom_close(socket_client_3);
                        socket_client_3 = 0;
                    }
                }
            }

            if (socket_client_4 > 0) {

                if (FD_ISSET(socket_client_4, &sockSet)) {

                    nRecv = qcom_recv(socket_client_4, recvBuf, TCP_RX_BUF_LEN, 0);

                    if (nRecv > 0) {
                        A_PRINTF("Client4 recv recvBuf %s\n", recvBuf);
                        NotifyUart(recvBuf, nRecv);
                    }
                    else {
                        A_PRINTF("Close client 4\n");
                        qcom_close(socket_client_4);
                        socket_client_4 = 0;
                    }
                }
            }

            // Processing request coming connection
            if (FD_ISSET(socketServer, &sockSet)) {

                socketClient = qcom_accept(socketServer, 
                                          (struct sockaddr *)&clientAddr, 
                                          &addrLen);

                ipAddr = ntohl(clientAddr.sin_addr.s_addr);
                port   = ntohs(clientAddr.sin_port);

                A_PRINTF("Accept connection from %d.%d.%d.%d:%d\n",
                         (ipAddr) >> 24 & 0xFF, 
                         (ipAddr) >> 16 & 0xFF, 
                         (ipAddr) >> 8 & 0xFF, 
                         (ipAddr) & 0xFF,
                          port);

                A_PRINTF("Socket Client1 = %d Client2 = %d Client3 = %d Client4 = %d\n",
                       socket_client_1, socket_client_2, socket_client_3, socket_client_4);

                // Find the availbale client
                if (socket_client_1 <= 0) {
                    socket_client_1 = socketClient;
                    A_PRINTF("Create Client 1 = %d\n", socket_client_1);
                }
                else if (socket_client_2 <= 0) {
                    socket_client_2 = socketClient;
                    A_PRINTF("Create Client 2 = %d\n", socket_client_2);
                }
                else if (!(socket_client_3 > 0)) {
                    socket_client_3 = socketClient;
                    A_PRINTF("Create Client 3 = %d\n", socket_client_3);
                }
                else if (!(socket_client_4 > 0)) {
                    socket_client_4 = socketClient;
                    A_PRINTF("Create Client 4 = %d\n", socket_client_4);
                }

                if ((socket_client_1 > 0) && 
                    (socket_client_2 > 0) && 
                    (socket_client_3 > 0) && 
                    (socket_client_4 > 0)) {

                    A_PRINTF("Client is enough \n");
                    qcom_close(socketClient);
                }
            }
        }
    }

DONE:
    // Close TCP sockets
    if (socketClient > 0)
        qcom_close(socketClient);

    if (socketServer > 0)
        qcom_close(socketServer);
}
