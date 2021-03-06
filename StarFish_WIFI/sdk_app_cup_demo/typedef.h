#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

typedef enum {
    CONNECT_ASSOC_POLICY_USER        = 0x0001,
    CONNECT_SEND_REASSOC             = 0x0002, 
    CONNECT_IGNORE_WPAx_GROUP_CIPHER = 0x0004,
    CONNECT_PROFILE_MATCH_DONE       = 0x0008,
    CONNECT_IGNORE_AAC_BEACON        = 0x0010,
    CONNECT_CSA_FOLLOW_BSS           = 0x0020,
    CONNECT_DO_WPA_OFFLOAD           = 0x0040,
    CONNECT_DO_NOT_DEAUTH            = 0x0080,
    CONNECT_WPS_FLAG                 = 0x0100,
    CONNECT_IGNORE_BSSID_HINT        = 0x0200,
    CONNECT_STAY_AWAKE               = 0x0400, 
    CONNECT_SAFE_MODE                = 0x0800,

    /* AP configuration flags */
    AP_NO_DISASSOC_UPON_DEAUTH       = 0x10000,
    AP_HOSTPAL_SUPPORT               = 0x20000
}ENUM_CONTROL_FLAG;

// Encryption mode
typedef enum {
    WLAN_CRYPT_NONE, 
    WLAN_CRYPT_WEP_CRYPT,
    WLAN_CRYPT_TKIP_CRYPT,
    WLAN_CRYPT_AES_CRYPT, 
    WLAN_CRYPT_BIP_CRYPT,
    WLAN_CRYPT_KTK_CRYPT,
    WLAN_CRYPT_INVALID
}ENUM_ENCRYTION_MODES;

typedef enum
{
    WLAN_AUTH_NONE, 
    WLAN_AUTH_WPA,
    WLAN_AUTH_WPA2, 
    WLAN_AUTH_WPA_PSK, 
    WLAN_AUTH_WPA2_PSK, 
    WLAN_AUTH_WPA_CCKM,
    WLAN_AUTH_WPA2_CCKM,
    WLAN_AUTH_WPA2_PSK_SHA256, 
    WLAN_AUTH_INVALID
}ENUM_AUTHENTICATION_MODES;

typedef struct {
    A_UINT32 len;
    A_UINT8 *pBuff;
}MEM_SHARE_t;

#endif
