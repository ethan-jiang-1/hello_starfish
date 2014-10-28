#ifndef _QCOM_WLAN_H__
#define _QCOM_WLAN_H__

typedef enum {
    QCOM_WLAN_DEV_MODE_STATION = 0,
    QCOM_WLAN_DEV_MODE_AP,
    QCOM_WLAN_DEV_MODE_INVALID
}QCOM_WLAN_DEV_MODE;

extern A_STATUS qcom_op_set_mode(A_UINT32 mode);
extern A_STATUS qcom_op_get_mode(A_UINT32 *pmode);

extern A_STATUS qcom_disconnect(void);
extern A_STATUS qcom_set_country_code(A_CHAR *pcountry_code);
extern A_STATUS qcom_get_country_code(A_CHAR *pcountry_code);

typedef enum {
    QCOM_11A_MODE = 0x1,
    QCOM_11B_MODE = 0x2,
    QCOM_11G_MODE = 0x3,
    QCOM_11N_MODE = 0x4,
    QCOM_HT40_MODE = 0x5,
} QCOM_WLAN_DEV_PHY_MODE;
extern A_STATUS qcom_set_phy_mode(A_UINT8 phymode);
extern A_STATUS qcom_get_phy_mode(A_UINT8 *pphymode);

//channel value start from 1
extern A_STATUS qcom_set_channel(A_UINT16 channel);
extern A_STATUS qcom_get_channel(A_UINT16 *pchannel);
extern A_STATUS qcom_get_channel_freq(A_UINT16 *pch_freq);

extern A_STATUS qcom_set_tx_power(A_UINT32 dbm);
extern A_STATUS qcom_get_tx_power(A_UINT32 *pdbm);

extern A_STATUS qcom_allow_aggr_set_tid(A_UINT16 tx_allow_aggr, A_UINT16 rx_allow_aggr);
extern A_STATUS qcom_allow_aggr_get_tid(A_UINT16 *ptx_allow_aggr, A_UINT16 *prx_allow_aggr);

extern A_STATUS qcom_set_connect_callback(void *callback);

extern A_STATUS qcom_get_ssid(A_CHAR *pssid);
extern A_STATUS qcom_get_state(A_UINT8 *pstate);

extern A_STATUS qcom_init_dev_context(void);

#endif
