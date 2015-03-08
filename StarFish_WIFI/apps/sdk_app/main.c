#include <qcom_mem.h>
#include <qcom_cli.h>
#include "main.h"

#define APP_POOL_SIZE                 ( 6 * 512 )

#define WIFI_THREAD_STACK_SIZE   ( 4 * 512 )
static TX_BYTE_POOL pool_main;
extern void WIFI_Task(A_UINT32 arg);
static TX_THREAD wifi_thread;

//#define WIFI_SEND_THREAD_STACK_SIZE   ( 1 * 512 )
//#define UART_RECV_THREAD_STACK_SIZE   ( 1 * 512 )
//#define UART_SEND_THREAD_STACK_SIZE   ( 1 * 512 )

//#define WIFI2UART_QUEUE_SIZE  (20)
//#define UART2WIFI_QUEUE_SIZE  (20)

extern void user_pre_init(void);
//extern void WIFI_RecvTask(A_UINT32 arg);
//extern void WIFI_SendTask(A_UINT32 arg);

extern void UART_Init(void); 
//extern void UART_RecvTask(A_UINT32 arg);
//extern void UART_SendTask(A_UINT32 arg); 
extern A_INT32 CLICMDS_StartTelnetDaemon(void);

extern TX_QUEUE queue_share;
extern SYS_CONFIG_t sys_config;
/*
TX_QUEUE queue_share_uart2wifi;
TX_QUEUE queue_share_wifi2uart;



static TX_THREAD wifi2uart_recv_thread;
static TX_THREAD wifi2uart_send_thread;
static TX_THREAD uart2wifi_recv_thread;
static TX_THREAD uart2wifi_send_thread;
*/
static const A_UINT8 auchCRCHi[] =
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

const A_UINT8 auchCRCLo[] =
{
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

A_UINT16 MAIN_CalcCRC16(A_UINT8 *puchMsg, A_UINT16 len)
{
    A_UINT8 uchCRCHi = 0xFF;
    A_UINT8 uchCRCLo = 0xFF;
    A_UINT8 uIndex;

    while (len--) {
        uIndex   = uchCRCHi ^ *puchMsg++;
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
        uchCRCLo = auchCRCLo[uIndex];
    }

    return (uchCRCHi | uchCRCLo << 8);
}

A_UINT32 _inet_addr(A_CHAR *str)
{
    A_UINT32 ipaddr;
    A_UINT32 data[4];

    int ret = A_SSCANF(str, "%3d.%3d.%3d.%3d", data, data + 1, data + 2, data + 3);

    if (ret < 0) 
        return (0);
    else 
        ipaddr = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];

    return (ipaddr);
}

A_STATUS config_ip_static(A_CHAR *pIpAddr, 
                          A_CHAR *pNetMask, 
                          A_CHAR *pGateway)
{
    A_UINT32 address;
    A_UINT32 submask;
    A_UINT32 gateway;
    A_STATUS status = 0;

    address = _inet_addr(pIpAddr);
    submask = _inet_addr(pNetMask);
    gateway = _inet_addr(pGateway);

    A_PRINTF("ConfigIpstatic ipAddress = 0x%x\n", address);

    status = qcom_ip_address_set(address, submask, gateway);

    if (A_OK == status) {
        A_PRINTF("AP: IP = %s submask = %s gateway = %s\n", pIpAddr, pNetMask, pGateway);
        return (A_OK);
    }
    else {
        A_PRINTF("Set AP address submask gateway fail\n");
        return (A_ERROR);
    }
}

A_STATUS config_dhcp_pool(A_CHAR *pStartaddr, 
                          A_CHAR *pEndaddr, 
                          A_UINT32 leasetime)
{
    A_UINT32 startaddr, endaddr;
    A_STATUS status;

    startaddr = _inet_addr(pStartaddr);
    endaddr   = _inet_addr(pEndaddr);

    status = qcom_dhcps_enable(TRUE);

    if (A_OK == status) 
        A_PRINTF("Enable DHCP success\n");
    else 
        A_PRINTF("Enable DHCP sucess\n");

    status = qcom_dhcps_set_pool(startaddr, endaddr, leasetime);

    if (A_OK == status) 
        A_PRINTF("Set dhcp pool %s ~ %s leasetime = %d\n", pStartaddr, pEndaddr, leasetime);
    else 
        A_PRINTF("Set dhcp pool fail\n");

    return status;
}

A_STATUS MAIN_StartAPMode()
{
    A_STATUS status = A_OK;
    A_CHAR ssid[LEN_SSID] = {0};
    A_UINT8 macAddr[6] = {0};

    // Set the security
    status = qcom_sec_set_auth_mode(sys_config.authMode);
    if(A_OK != status) {
        A_PRINTF("Fail to set the auth mode\r\n");
        return status;
    }

    status = qcom_sec_set_encrypt_mode(sys_config.encyptMode);
    if(A_OK != status) {
        A_PRINTF("Fail to set the encrypt mode\r\n");
        return status;
    }

    status = qcom_sec_set_passphrase(sys_config.apKey);
    if (A_OK != status) {
        A_PRINTF("Fail to set the passphrase\r\n");
        return status;
    }

    status = qcom_mac_get((A_UINT8 *)&macAddr);
    if (A_OK != status) {
        A_PRINTF("Fail to get the MAC\r\n");
        return status;
    }

    sprintf(ssid, "%s-%02X%02X%02X", sys_config.apSSID, macAddr[3], macAddr[4], macAddr[5]);

    status = qcom_ap_start(ssid);
    if (A_OK != status) {
        A_PRINTF("Start AP failed\n");
        return status;
    }

    /* config AP IP */
    status = config_ip_static(sys_config.ipAddressAP, 
                              sys_config.netMaskAP, 
                              sys_config.gatewayAddressAP);
    if (A_OK != status) {
        return status;
    }

    /* start DHCP server */
    status = config_dhcp_pool(sys_config.dhcpPoolLow, 
                              sys_config.dhcpPoolHigh, 
                              sys_config.dhcpLeastTime);
    if (A_OK != status) {
        return status;
    }

    return (A_OK);
}

A_STATUS get_ipconfig()
{
    A_UINT8 macAddr[6] = {0};
    A_UINT32 ipAddress;
    A_UINT32 submask;
    A_UINT32 gateway;
    A_UINT32 dns;
    A_STATUS status;

    status = qcom_mac_get((A_UINT8 *)&macAddr);

    if (A_ERROR == status) {
        A_PRINTF("Fail to get the MAC\r\n");
        return (A_ERROR);
    }

    status = qcom_ip_address_get(&ipAddress, &submask, &gateway);

    if (A_ERROR == status || ipAddress == 0) {
        A_PRINTF("Fail to get the IP\r\n");
        return (A_ERROR);
    }

    status = qcom_dns_server_address_get(&dns);

    if (A_ERROR == status) {
        A_PRINTF("Fail to get the dns server\r\n");
        return (A_ERROR);
    }

    A_PRINTF("MAC addr = %x:%x:%x:%x:%x:%x\n",
             macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

    A_PRINTF("IP:%d.%d.%d.%d\n", 
             (ipAddress) >> 24 & 0xFF, 
             (ipAddress) >> 16 & 0xFF, 
             (ipAddress) >> 8 & 0xFF, 
             (ipAddress) & 0xFF);

    A_PRINTF("Mask:%d.%d.%d.%d\n", 
             (submask) >> 24 & 0xFF, 
             (submask) >> 16 & 0xFF, 
             (submask) >> 8 & 0xFF, 
             (submask) & 0xFF);

    A_PRINTF("Gateway:%d.%d.%d.%d\n", 
             (gateway) >> 24 & 0xFF, 
             (gateway) >> 16 & 0xFF, 
             (gateway) >> 8 & 0xFF, 
             (gateway) & 0xFF);

    A_PRINTF("DNS:%d.%d.%d.%d\n", 
             (dns) >> 24 & 0xFF, 
             (dns) >> 16 & 0xFF, 
             (dns) >> 8 & 0xFF, 
             (dns) & 0xFF);

    return (A_OK);
}

A_STATUS MAIN_StartStationMode(A_CHAR *ssid, A_CHAR *passwd)
{
    A_STATUS status = A_OK;
    A_INT32 retryCount = 0;
    A_UINT8 wifiState = 0;

    status = qcom_sec_set_passphrase(passwd);
    if (A_OK != status) {
        A_PRINTF("Fail to set password\r\n");
        return status;
    }

    status = qcom_sta_connect_with_scan(ssid);
    if (A_OK != status) {
        A_PRINTF("Fail to connected with AP\r\n");
        return status;
    }

    if (!sys_config.dhcpEnable) {

        // Set the static IP 
        A_UINT32 address;
        A_UINT32 submask;
        A_UINT32 gateway;

        address = _inet_addr(sys_config.ipAddressStation);
        submask = _inet_addr(sys_config.netMaskStation);
        gateway = _inet_addr(sys_config.gatewayAddressStation);

        qcom_ip_address_set(address, submask, gateway);
    }
    else {
        qcom_dhcpc_enable(TRUE);

        // Check wifi connection status
        do {
            qcom_thread_msleep(1000);
            qcom_get_state(&wifiState);

            retryCount ++;

            if (retryCount > 10) {

                A_PRINTF("Fail to connected with AP and get the current state:%d\r\n", wifiState);

                return (A_ERROR);
            }

        } while(4 != wifiState);

        retryCount = 0;

        // Check ip address
        while (1) {

            status = get_ipconfig();

            if (A_OK == status) {
                A_PRINTF("Get ip success\n");
                break;
            }

            if (5 != retryCount) {
                retryCount ++;
                qcom_thread_msleep(3000);
            }
            else {
                break;
            }
        }
    }

    return (status);
}


void MAIN_ConfigWIFI()
{
    A_STATUS status;

    if (sys_config.mode == eStation) {

        A_PRINTF("Start Station Mode\n");

        status = MAIN_StartStationMode(sys_config.staSSID, sys_config.staKey);

        if (A_ERROR == status) {

            status = MAIN_StartAPMode();

            if (A_ERROR == status) {
                qcom_sys_reset();
            }
        }
    }
    else {

        A_PRINTF("Start AP mode zga\n");

        status = MAIN_StartAPMode();

        if (A_ERROR == status) {
            qcom_sys_reset();
        }
    }
}

void MAIN_StartAppDaemon()
{
  A_INT32 i;
  A_CHAR *wifiThreadMemPtr;
  tx_byte_pool_create(&pool_main, 
                        "APP pool", 
                        TX_POOL_CREATE_DYNAMIC, 
                        APP_POOL_SIZE);
    // Alloc the WIFI thread stack
    tx_byte_allocate(&pool_main, 
                     (VOID **)&wifiThreadMemPtr, 
                     WIFI_THREAD_STACK_SIZE, 
                     TX_NO_WAIT);

    tx_thread_create(&wifi_thread, 
                     "WIFI Thread", 
                     (VOID (*)(ULONG))WIFI_Task,
                     i, 
                     wifiThreadMemPtr, 
                     WIFI_THREAD_STACK_SIZE, 
                     16, 
                     16, 
                     4, 
                     TX_AUTO_START);
/*
    A_INT32 i;

    UCHAR *qMemWifi2UartPtr;
    UCHAR *qMemUart2WifiPtr;

    A_CHAR *wifiRecvThreadMemPtr;
    A_CHAR *wifiSendThreadMemPtr;

    A_CHAR *uartRecvThreadMemPtr;
    A_CHAR *uartSendThreadMemPtr;

    tx_byte_pool_create(&pool_main, 
                        "APP pool", 
                        TX_POOL_CREATE_DYNAMIC, 
                        APP_POOL_SIZE);

	// Alloc a WIFI 2 UART queue
	tx_byte_allocate(&pool_main, 
                     (VOID **)&qMemWifi2UartPtr, 
                     WIFI2UART_QUEUE_SIZE * sizeof(MEM_SHARE_t), 
                     TX_NO_WAIT);

	tx_queue_create(&queue_share_wifi2uart, 
                    "WIFI 2 UART Q", 
                    sizeof(MEM_SHARE_t) / sizeof(A_UINT32), 
                    qMemWifi2UartPtr, 
                    WIFI2UART_QUEUE_SIZE * sizeof(MEM_SHARE_t));

	// Alloc a UART 2 WIFI queue
	tx_byte_allocate(&pool_main, 
                     (VOID **)&qMemUart2WifiPtr, 
                     UART2WIFI_QUEUE_SIZE * sizeof(MEM_SHARE_t), 
                     TX_NO_WAIT);

	tx_queue_create(&queue_share_uart2wifi, 
                    "UART 2 WIFI Q", 
                    sizeof(MEM_SHARE_t) / sizeof(A_UINT32), 
                    qMemUart2WifiPtr, 
                    UART2WIFI_QUEUE_SIZE * sizeof(MEM_SHARE_t));

    // Alloc the WIFI 2 UART thread stack
    tx_byte_allocate(&pool_main, 
                     (VOID **)&wifiRecvThreadMemPtr, 
                     WIFI_RECV_THREAD_STACK_SIZE, 
                     TX_NO_WAIT);

    tx_thread_create(&wifi2uart_recv_thread, 
                     "WIFI Recv Thread", 
                     (VOID (*)(ULONG))WIFI_RecvTask,
                     i, 
                     wifiRecvThreadMemPtr, 
                     WIFI_RECV_THREAD_STACK_SIZE, 
                     16, 
                     16, 
                     4, 
                     TX_AUTO_START);  

    tx_byte_allocate(&pool_main, 
                     (VOID **)&wifiSendThreadMemPtr, 
                     WIFI_SEND_THREAD_STACK_SIZE, 
                     TX_NO_WAIT);

    tx_thread_create(&wifi2uart_send_thread, 
                     "WIFI Send Thread", 
                     (VOID (*)(ULONG))WIFI_SendTask,
                     i, 
                     wifiSendThreadMemPtr, 
                     WIFI_SEND_THREAD_STACK_SIZE, 
                     16, 
                     16, 
                     4, 
                     TX_AUTO_START);  

    // Alloc the UART 2 WIFI thread stack
    tx_byte_allocate(&pool_main, 
                     (VOID **)&uartRecvThreadMemPtr, 
                     UART_RECV_THREAD_STACK_SIZE, 
                     TX_NO_WAIT);

    tx_thread_create(&uart2wifi_recv_thread, 
                     "UART Recv Thread", 
                     (VOID (*)(ULONG))UART_RecvTask,
                     i, 
                     uartRecvThreadMemPtr, 
                     UART_RECV_THREAD_STACK_SIZE, 
                     16, 
                     16, 
                     4, 
                     TX_AUTO_START);  

    tx_byte_allocate(&pool_main, 
                     (VOID **)&uartSendThreadMemPtr, 
                     UART_SEND_THREAD_STACK_SIZE, 
                     TX_NO_WAIT);

    tx_thread_create(&uart2wifi_send_thread, 
                     "UART Send Thread", 
                     (VOID (*)(ULONG))UART_SendTask,
                     i, 
                     uartSendThreadMemPtr, 
                     UART_SEND_THREAD_STACK_SIZE, 
                     16, 
                     16, 
                     4, 
                     TX_AUTO_START); 
*/
}


void user_main(void)
{
    user_pre_init();

    console_setup();

    // Load the configurtation for NVRAM
    NVRAM_RestoreConfig();

    UART_Init();

    MAIN_StartAppDaemon();

    CLICMDS_StartTelnetDaemon();
}
