#ifndef _MAIN_H_
#define _MAIN_H_

#include <base.h>
#include <qcom_system.h>
#include <qcom_common.h>
#include <qcom_network.h>
#include <qcom_wlan.h>
#include <qcom_ap.h>
#include <qcom_scan.h>
#include <qcom_timer.h>
#include <qcom_sec.h>
#include <qcom_sta.h>
#include <qcom_nvram.h>
#include <qcom_mem.h>
#include <qcom_time.h>
#include <select_api.h>
#include <socket_api.h>
#include "typedef.h"

#define SW_VERSION "v0.1"

#define SUCCESS           (0) 
#define LEN_SSID          (32)
#define LEN_PASSWORD      (64)
#define LEN_IP_ADDRESS    (16)
#define LEN_UART_BUFF     (256)
#define CONFIG_NVRAM_ADDR (0x7E000)
//#define CONFIG_NVRAM_ADDR (0x80000)
#define ERASEROMRANGE     (4096)
#define TCP_RX_BUF_LEN    (512)

typedef enum {
    eAP,
    eStation
}ENUM_MODE;

typedef struct {

    ENUM_MODE mode;
    A_INT32 port;

    // AP mode
    A_CHAR apSSID[LEN_SSID];
    A_CHAR apKey[LEN_PASSWORD];

    A_UINT32 authMode;
    A_UINT32 encyptMode;

    A_CHAR ipAddressAP[LEN_IP_ADDRESS];
    A_CHAR netMaskAP[LEN_IP_ADDRESS];
    A_CHAR gatewayAddressAP[LEN_IP_ADDRESS];

    A_CHAR dhcpPoolLow[LEN_IP_ADDRESS];
    A_CHAR dhcpPoolHigh[LEN_IP_ADDRESS];

    A_INT32 dhcpLeastTime;

    // Station Mode
    A_CHAR staSSID[LEN_SSID];
    A_CHAR staKey[LEN_PASSWORD];

    A_BOOL dhcpEnable;

    A_CHAR ipAddressStation[LEN_IP_ADDRESS];
    A_CHAR netMaskStation[LEN_IP_ADDRESS];
    A_CHAR gatewayAddressStation[LEN_IP_ADDRESS];
    // test mode
    A_CHAR staSSID1[LEN_SSID];
    A_CHAR staKey1[LEN_PASSWORD];

    A_BOOL dhcpEnable1;

    A_CHAR ipAddressStation1[LEN_IP_ADDRESS];
    A_CHAR netMaskStation1[LEN_IP_ADDRESS];
    A_CHAR gatewayAddressStation1[LEN_IP_ADDRESS];
	//end test mode
    A_UINT16 crc16;

}__attribute__((packed)) SYS_CONFIG_t;


A_UINT16 MAIN_CalcCRC16(A_UINT8 *puchMsg, A_UINT16 len);

A_STATUS NVRAM_RestoreConfig();
A_STATUS NVRAM_SaveSettings(SYS_CONFIG_t *pConfig);
void NVRAM_LoadDefaultSettings(SYS_CONFIG_t *pConfig);

void TASKMGR_Del(void);
void TASKMGR_Yield();
void TASKMGR_Exit();
void TASKMGR_KillAll(void);

#endif
