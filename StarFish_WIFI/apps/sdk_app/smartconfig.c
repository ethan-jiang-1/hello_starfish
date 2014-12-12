#include "main.h"

#define LEN_MTU       (1600)
#define ETH_ALEN      (6)

#define MAX_SSID_LEN  (32)
#define MAX_KEY_LEN   (64)
#define MAX_TOTAL_LEN (2 + MAX_SSID_LEN + MAX_KEY_LEN + 16)

typedef struct  {
    A_UINT16 frame_ctl;
    A_UINT16 duration_id;
    A_UINT8 addr1[ETH_ALEN];
    A_UINT8 addr2[ETH_ALEN];
    A_UINT8 addr3[ETH_ALEN];
    A_UINT16 seq_ctl;
    A_UINT8 addr4[ETH_ALEN];
    A_UINT8 payload[0];
} __attribute__((packed)) IEEE80211_HDR_4ADDR_t; 

typedef enum {
    eHEADER,
    ePROC_DATA,
}ENUM_PROC_t;

typedef struct {
    A_BOOL  isReceived;
    A_UINT8 data;
} RAW_RECV_DATA_t;

static qcom_timer_t         promisc_timer;
static A_UINT8             *p_promisc_rx_buf = NULL;
static RAW_RECV_DATA_t     *p_raw_recv_data  = NULL;
static ENUM_PROC_t          e_packet_proc    = eHEADER;


extern void CLICMDS_STAConnect2(A_CHAR *pSSID, A_CHAR *pKey);

static A_BOOL SMARTCONFIG_IsSmartConfigPacket(IEEE80211_HDR_4ADDR_t *pPacket) {

    if ((pPacket->addr3[0] == 0x01) && (pPacket->addr3[1] == 0x00) && (pPacket->addr3[2] == 0x5e) &&
        ((pPacket->addr3[3] & 0x80) == 0) && (pPacket->addr3[5] == 0xFE)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

A_BOOL SMARTCONFIG_IsAllDataReceived(A_UINT8 len) 
{
    A_UINT8 i = 0;

    for (i=0; i<len; i++) {

        if (!(p_raw_recv_data[i].isReceived)) {
            return FALSE;
        }
    }

    return TRUE;
}

 void SMARTCONFIG_Proc() 
{
    A_CHAR cSSID[MAX_SSID_LEN] = {0};
    A_CHAR cKey[MAX_KEY_LEN]   = {0};

	A_UINT32 ip;
	A_UINT32 mask;
	A_UINT32 gateway;
	A_UINT32 dns;

    A_INT32 i     = 0;
    A_INT32 index = 2;

    // Acquire the SSID
    for (i=0; i<MAX_SSID_LEN; i++) {
        if (p_raw_recv_data[i + index].data != '\0') {
            cSSID[i] = p_raw_recv_data[i + index].data;
        }
        else {
            cSSID[i] = '\0';
            break;
        }
    }

    // Acquire the Key
    index += i + 1;

    for (i=0; i<MAX_KEY_LEN; i++) {
        if (p_raw_recv_data[i + index].data != '\0') {
            cKey[i] = p_raw_recv_data[i + index].data;
        }
        else {
            cKey[i] = '\0';
            break;
        }
    }

    index += i + 1;

    // Acquire IP
    ip = (p_raw_recv_data[index + 0].data << 24) | (p_raw_recv_data[index + 1].data << 16) | 
        (p_raw_recv_data[index + 2].data << 8) | p_raw_recv_data[index + 3].data;

    index += 4;

    // Acquire subnet mask
    mask = (p_raw_recv_data[index + 0].data << 24) | (p_raw_recv_data[index + 1].data << 16) | 
        (p_raw_recv_data[index + 2].data << 8) | p_raw_recv_data[index + 3].data;

    index += 4;

    // Acquire gateway
    gateway = (p_raw_recv_data[index + 0].data << 24) | (p_raw_recv_data[index + 1].data << 16) | 
        (p_raw_recv_data[index + 2].data << 8) | p_raw_recv_data[index + 3].data;

    index += 4;

    // Acquire dns
    dns = (p_raw_recv_data[index + 0].data << 24) | (p_raw_recv_data[index + 1].data << 16) | 
        (p_raw_recv_data[index + 2].data << 8) | p_raw_recv_data[index + 3].data;

    A_PRINTF("\r\n");

    A_PRINTF("SSID :%s\n",   cSSID);
    A_PRINTF("key  :%s\n",   cKey);
    A_PRINTF("ip   :%x\n",   ip);
    A_PRINTF("mask :%x\n",   mask);
    A_PRINTF("gw   :%x\n",   gateway);
    A_PRINTF("dns  :%x\n",   dns);

    A_PRINTF("\r\n");

    qcom_promiscuous_enable(0);

    CLICMDS_STAConnect2(cSSID, cKey);
}

static void SMARTCONFIG_ResetProc()
{
    e_packet_proc = eHEADER;

    memset(p_raw_recv_data, 0, MAX_TOTAL_LEN * sizeof(RAW_RECV_DATA_t));
}

static A_UINT8 SMARTCONFIG_CheckSum(A_UINT8 *buf, int len)
{
    A_INT32 i        = 0;
    A_UINT8 checksum = 0;

    for (i=0; i<len; i++) 
        checksum += buf[i];

    return (~checksum + 2);
}

static void SMARTCONFIG_PromiscuosCallback(A_UINT8 *buf, int length) 
{
    IEEE80211_HDR_4ADDR_t *pPacket = (IEEE80211_HDR_4ADDR_t *)buf;

    static A_UINT8 totalLen = 0;

    if (SMARTCONFIG_IsSmartConfigPacket(pPacket)) {

        A_UINT8 index = pPacket->addr3[3];
        A_UINT8 value = pPacket->addr3[4];

        A_PRINTF("index = %d, value = %d\r\n", index, value);

        switch (e_packet_proc) {

        case eHEADER:
            if (index < 2) {

                p_raw_recv_data[index].isReceived = TRUE;
                p_raw_recv_data[index].data       = value;

                if (p_raw_recv_data[0].isReceived && p_raw_recv_data[1].isReceived) {

                    if (SMARTCONFIG_CheckSum(&p_raw_recv_data[0].data, 1) == p_raw_recv_data[1].data) {

                        e_packet_proc = ePROC_DATA;
                        totalLen     = p_raw_recv_data[0].data;

                        A_PRINTF("Smart Config Len = %d\r\n", totalLen);

                        qcom_timer_stop(&promisc_timer);
                        qcom_timer_start(&promisc_timer);
                    }
                }
            } 
            break;

        case ePROC_DATA:
            if (index < totalLen) {

                p_raw_recv_data[index].isReceived = TRUE;
                p_raw_recv_data[index].data       = value;

                if (SMARTCONFIG_IsAllDataReceived(totalLen)) {

                    A_PRINTF("All Packets received, index = %d\r\n", index);
                    SMARTCONFIG_Proc();

                    SMARTCONFIG_ResetProc(); 
                    return;
                }
            }
            else { // Error Handle
                qcom_timer_stop(&promisc_timer);
                SMARTCONFIG_ResetProc();
            }
            break;
        }
    }
}

static void SMARTCONFIG_RevPacketTimerHandler(unsigned int alarm, void *data) 
{
    A_PRINTF("SMARTCONFIG_RevPacketTimer: Timeout!\r\n");

    qcom_thread_msleep(200);
    SMARTCONFIG_ResetProc(); 
}

A_STATUS SMARTCONFIG_SetPromiscuousEnable(A_UINT8 enabled) 
{
    // In order to optimize memory
    if (p_promisc_rx_buf == NULL) 
        p_promisc_rx_buf = (A_UINT8 *)qcom_mem_alloc(LEN_MTU);

    if (p_promisc_rx_buf == NULL) {
        A_PRINTF("SMARTCONFIG_SetPromicuousEnable: Failed to alloc p_promiscRxBuf!\r\n");
        return A_ERROR;
    }

    // The raw receive data acquire from the dest mac
    if (p_raw_recv_data == NULL) 
        p_raw_recv_data = (RAW_RECV_DATA_t *)qcom_mem_alloc(MAX_TOTAL_LEN * sizeof(RAW_RECV_DATA_t));

    if (p_raw_recv_data == NULL) {
        A_PRINTF("SMARTCONFIG_SetPromicuousEnable: Failed to alloc p_recvData!\r\n");
        return A_ERROR;
    }

    // The promisc timer to monitor the smart config packet receive procedure
    qcom_timer_init(&promisc_timer, SMARTCONFIG_RevPacketTimerHandler, NULL, 5 * 1000, ONESHOT);

    SMARTCONFIG_ResetProc();

    qcom_disconnect();

    qcom_set_promiscuous_rx_cb(p_promisc_rx_buf, SMARTCONFIG_PromiscuosCallback);

    return qcom_promiscuous_enable(enabled); 
}
