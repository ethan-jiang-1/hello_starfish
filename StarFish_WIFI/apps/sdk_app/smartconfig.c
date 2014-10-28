#include "main.h"

#define LEN_MTU    (1600)
#define ETH_ALEN   (6)
#define SYNC_BYTES (4)

typedef struct {
	A_UINT8 version;
	A_UINT8 encrypt;
	A_UINT8 flags;
	A_UINT8 len;
}__attribute__((packed)) SMART_CONFIG_HEAD_t;

typedef struct {

    SMART_CONFIG_HEAD_t hearder;

	A_UINT8	 ap_ssid[32];	 //ssid
	A_UINT8	 ap_passkey[64]; //psk
	A_UINT8	 ap_index_key;
	A_UINT8	 ap_mode;
	A_UINT8	 ap_enath_mode;
	A_UINT8	 ap_crpty_type;
	A_UINT32 ip;
	A_UINT32 mask;
	A_UINT32 gw;
	A_UINT32 dns;

}__attribute__((packed)) SMART_CONFIG_DATA_t;

typedef struct {
    A_UINT16 frame_ctl;
    A_UINT16 duration_id;
    A_UINT8  addr1[ETH_ALEN];
    A_UINT8  addr2[ETH_ALEN];
    A_UINT8  payload[0];
}__attribute__((packed)) IEEE80211_HDR_2ADDR_t;

typedef enum {
    eSYNC,
    ePROC_DATA,
}ENUM_PROC_t;

typedef struct {
    A_BOOL  isReceived;
    A_UINT8 data;
} RECV_DATA_t;

static TX_TIMER      promic_timer;
static A_UINT8      *p_promiscRxBuf     = NULL;
static RECV_DATA_t  *p_recvData         = NULL;
static SMART_CONFIG_DATA_t *p_smartData = NULL;
static ENUM_PROC_t   ePacket_Proc       = eSYNC;
static A_UINT32      expected_index     = 0;

static A_BOOL SMARTCONFIG_IsSmartConfigPacket(IEEE80211_HDR_2ADDR_t *pPacket) {

    if ((pPacket->addr1[0] == 0x01) && (pPacket->addr1[1] == 0x00) && (pPacket->addr1[2] == 0x5e) &&
        ((pPacket->addr1[3] & 0x80) == 0) && (pPacket->addr1[5] == 0xFE)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

A_BOOL SMARTCONFIG_IsAllDataReceived() 
{
    int i = 0;

    for (i=0; i<sizeof(SMART_CONFIG_DATA_t); i++) {

        if (!(p_recvData[i].isReceived)) {
            return FALSE;
        }
    }

    return TRUE;
}

 void SMARTCONFIG_Proc() 
{
    A_UINT8 *pData         = (A_UINT8 *)p_smartData;
    RECV_DATA_t *pRecvData = (RECV_DATA_t *)p_recvData;
    int i = 0;

    for (i=0; i<sizeof(SMART_CONFIG_DATA_t); i++) {

        *pData = pRecvData->data;

        pData ++;
        pRecvData ++;
    }

    A_PRINTF("\r\n");

    A_PRINTF("version       :%x\n",   p_smartData->hearder.version);
    A_PRINTF("encrypt       :%x\n",   p_smartData->hearder.encrypt);
    A_PRINTF("flags         :%x\n",   p_smartData->hearder.flags);
    A_PRINTF("len           :%x\n",   p_smartData->hearder.len);
    A_PRINTF("ssid          :%s\n",   p_smartData->ap_ssid);
    A_PRINTF("psk           :%s\n",   p_smartData->ap_passkey);
    A_PRINTF("key_index     :%x\n",   p_smartData->ap_index_key);
    A_PRINTF("ap_mode       :%x\n",   p_smartData->ap_mode);
    A_PRINTF("ap_enath_mode :%x\n",   p_smartData->ap_enath_mode);
    A_PRINTF("ap_crpty_type :%x\n",   p_smartData->ap_crpty_type);
    A_PRINTF("ip            :%x\n",   p_smartData->ip);
    A_PRINTF("mask          :%x\n",   p_smartData->mask);
    A_PRINTF("gw            :%x\n",   p_smartData->gw);
    A_PRINTF("dns           :%x\n",   p_smartData->dns);

    A_PRINTF("\r\n");
}

static void SMARTCONFIG_ResetProc()
{
    ePacket_Proc   = eSYNC;
    expected_index = 0; 

    memset(p_recvData, 0, sizeof(SMART_CONFIG_DATA_t) * sizeof(RECV_DATA_t));
}

static void SMARTCONFIG_PromiscuosCallback(unsigned char *buf, int length) 
{
    IEEE80211_HDR_2ADDR_t *pPacket = (IEEE80211_HDR_2ADDR_t *)buf;

    if (SMARTCONFIG_IsSmartConfigPacket(pPacket)) {

        A_UINT8 index = pPacket->addr1[3];
        A_UINT8 value = pPacket->addr1[4];

        A_PRINTF("index = %d, value = %d\r\n", index, value);

        switch (ePacket_Proc) {

        // The UDP packet maybe out of order in
        // some cases, however we still need to
        // check it is really smart config data
        case eSYNC:

            if (index == expected_index) {

                // The expected index have been receive,
                // Deactivate the receiver timer
                //tx_timer_deactivate(&promic_timer);

                p_recvData[index].isReceived = TRUE;
                p_recvData[index].data       = value;

                expected_index++;

                if (expected_index >= SYNC_BYTES) {
                    ePacket_Proc = ePROC_DATA;
                    A_PRINTF("Found the sync!\r\n");
                }

                // Start the timer again to monitor
                // the receive procedure, if the next
                // packet is not received in 0.5s,
                // we reset the received procedure
               // tx_timer_activate(&promic_timer);
            } else {
                expected_index = 0;
            }
            break;

        case ePROC_DATA:

            //tx_timer_deactivate(&promic_timer);

            if (index < sizeof(SMART_CONFIG_DATA_t)) {

                p_recvData[index].isReceived = TRUE;
                p_recvData[index].data       = value;

                if (SMARTCONFIG_IsAllDataReceived()) {

                    A_PRINTF("All Packets received, index = %d\r\n", index);
                    SMARTCONFIG_Proc();

                    SMARTCONFIG_ResetProc(); 
                    return;
                }

                //tx_timer_activate(&promic_timer); 
            }
            else {
                SMARTCONFIG_ResetProc(); 
            }
            break;
        }
    }
}

void SMARTCONFIG_RevPacketTimer(ULONG id) 
{
    tx_timer_deactivate(&promic_timer);

    A_PRINTF("SMARTCONFIG_RevPacketTimer: Timeout!\r\n");

    SMARTCONFIG_ResetProc(); 
}

A_STATUS SMARTCONFIG_SetPromicuousEnable(A_UINT8 enabled) 
{
    //UINT status;

    // Inorder to optimize memory
    if (p_promiscRxBuf == NULL) 
        p_promiscRxBuf = (A_UINT8 *)qcom_mem_alloc(LEN_MTU);

    if (p_promiscRxBuf == NULL) {
        A_PRINTF("SMARTCONFIG_SetPromicuousEnable: Failed to alloc p_promiscRxBuf!\r\n");
        return A_ERROR;
    }

    if (p_recvData == NULL) 
        p_recvData = (RECV_DATA_t *)qcom_mem_alloc(
            sizeof(SMART_CONFIG_DATA_t) * sizeof(RECV_DATA_t));

    if (p_recvData == NULL) {
        A_PRINTF("SMARTCONFIG_SetPromicuousEnable: Failed to alloc p_recvData!\r\n");
        return A_ERROR;
    }

    if (p_smartData == NULL) 
        p_smartData = (SMART_CONFIG_DATA_t *)qcom_mem_alloc(
            sizeof(SMART_CONFIG_DATA_t) );

    if (p_smartData == NULL) {
        A_PRINTF("SMARTCONFIG_SetPromicuousEnable: Failed to alloc p_smartData!\r\n");
        return A_ERROR;
    }

    SMARTCONFIG_ResetProc();

    qcom_disconnect();

    //status = tx_timer_create(&promic_timer, "promic_timer",
    //                         SMARTCONFIG_RevPacketTimer, 0x1, 1, 50,
    //                         TX_NO_ACTIVATE);

    //if (status == TX_SUCCESS) {

        qcom_set_promiscuous_rx_cb(p_promiscRxBuf, SMARTCONFIG_PromiscuosCallback);

        return qcom_promiscuous_enable(enabled);

    //} else {
    //    A_PRINTF("SMARTCONFIG_SetPromicuousEnable: Failed to create timer!\r\n");
    //    return A_ERROR;
    //}

    return;
}
