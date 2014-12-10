

#include "app_wifi.h"


#include "app_trace.h"
#include <string.h>
#include <stdlib.h>

#include "app_rgb_led.h"


#define WIFI_MSG_NUMBER        64
#define WIFI_MSG_SIZE          1
static uint_32  g_uart0_rx_queue[sizeof(LWMSGQ_STRUCT)/sizeof(uint_32) + WIFI_MSG_NUMBER* WIFI_MSG_SIZE];

#define WIFI_RECV_BUF_SIZE     512  /*This is maxium buffer size*/
static uint_8  g_wifi_com_recv_buf[WIFI_RECV_BUF_SIZE];

static es201_dev_info_t         g_es201_gps_dev;

LWSEM_STRUCT                    g_mma8451_alarm_sem;
static LWSEM_STRUCT             g_at_cmd_send_sem;



///---------------------------------------------
#define GPS_NO_SIGNAL_CMD       "+GPSINFO: GPS NO SINGAL"
#define GPS_HEADER_CMD          "+GPSINFO: "
#define ES201_GPS_POWER_STATE   "+GPS: "
#define ES201_POWER_ON_CMD      "IIII"
#define ES201_OK_AT_CMD         "OK"
#define ES201_ERROR_AT_CMD      "ERROR"

#define ES201_AT_TEST_CMD       "AT"
#define ES201_GPS_INFO_AT_CMD   "AT+GPSINFO?"
#define OPEN_GPS_POWER          "AT+GPS=1"
#define GET_GPS_POWER_STATE     "AT+GPS?"
#define ES201_AT_CSQ_CMD        "AT+CSQ"
//////////////////////WIFI CMD
#define WIFI_TAG_HEADER				"%B1Q"
#define WIFI_IMAGE_TAG_HEADER							"%B1Q,300,"	
#define WIFI_IMAGE_TAG_LEN								(sizeof(WIFI_IMAGE_TAG_HEADER) - 1)
#define WIFI_IMAGE_CMD_HEADER_LEN						(sizeof("%B1Q,300,1000,1,0123456789"))

///---------------------------------------------
// sms send
#define ES201_AT_CIMI           "AT+CIMI"
#define ES201_AT_CSMS           "AT+CSMS=1"
#define ES201_AT_CNMI           "AT+CNMI=2,2"
#define ES201_AT_CMGF           "AT+CMGF=1"
#define ES201_AT_SEND_SMS       "AT+CMGS="
#define ES201_SMS_SEND_IND      ">"
// sms receive
#define ES201_RECV_NEW_MSG      "+CMT: \""

// lock phone number
#define LOCK_PHONE_NUMBER       "13616212725"


static void uart0_irq_handler(void* p_arg);
static void send_at_command(char* p_cmd);

bool verifyCheckSum( dword dwTargetCheckSum, uint_8 *pData, dword nDataLen )
{

		dword dwCheckSum = 0;
	while (nDataLen--)
	{
		dwCheckSum+=*pData++;
	}
	return dwCheckSum == dwTargetCheckSum;
}

static int ParseChecksum(uint_8 * pData, int dwLen)
	{

		int dwMulti = 1;
		int dwCheckSum = 0;
		int i=0;
		for(i=dwLen; i>0; --i)
		{
			dwCheckSum += ((*(pData + i - 1) - '0') * dwMulti);
			dwMulti *= 10;
		}

		return dwCheckSum;
	}
/**
*--------------------------------------------------------------
  * WIFI picture cmd:
	* %B1Q,ID,length, terminate flag, check sum,binary data.
	* length means how many byte of binary data
	*	terminate flag means it is the last frame image or not.
	* %B1Q,IMAGE_ID(3000),1000,1,0015609326......  
  *--------------------------------------------------------------
  */
static int  do_wifi_image_cmd(uint_8* pData)
{
//"%B1Q,300,1000,1,0123456789,"
	wifi_image_info info;
	char szBuf[64] = {0};
	char szDataLen[5] = {0};
	char szEndFlag[2] = {0};
	char szCheckSum[11] = {0};
	memcpy(szDataLen, pData + 9, 4);
	memcpy(szEndFlag, pData + 14, 1);
	memcpy(szCheckSum, pData + 16, 10);
	info.m_lDataLen = atoi(szDataLen);
	info.m_lEndFlag = atoi(szEndFlag);
	info.m_dwCheckSum =ParseChecksum((uint_8*)(szCheckSum), strlen(szCheckSum));
	if(verifyCheckSum(info.m_dwCheckSum, (uint_8 *)(pData+WIFI_IMAGE_CMD_HEADER_LEN), 
			info.m_lDataLen))
	{
		
		/*start to record spi*/
		APP_TRACE((const char*)g_wifi_com_recv_buf[WIFI_IMAGE_CMD_HEADER_LEN]);
		snprintf(szBuf, sizeof(szBuf)-1, "%%B1P,0,0:1");
    uart0_send_string((uint8_t*)szBuf);
		if(info.m_lEndFlag==1)
		{
			/*send all data stored in flash to eink*/
			
		}
		
	}

	return 1;
	
}
static int  do_wifi_xxx_cmd(char* p_str)
{
    char* delims = ",";
    char* p_result;
    es201_gps_handle v;
    int   i,length, lastframe= 0;

    // first, get es201 device instance
    v = get_es201_gps_instance();

    // 1. length
    p_result = strtok(p_str, (char*)delims);
    if (p_result)
    {
        length = atoi(p_result);
    }
    else
    {
        return -1;
    }

    // 2. last frame flag
    p_result = strtok( NULL, (char*)delims);
    if (p_result)
    {
        lastframe = atoi(p_result);
    }
    else
    {
			
        return -2;
    }

    // 3. altitude
    p_result = strtok( NULL, (char*)delims);
    if (p_result)
    {
        v->altitude = atof(p_result);
    }
    else
    {
        v->gps_signal = GPS_POWER_ON;
        return -3;
    }

    // 4. satellite number, no care about it
    p_result = strtok( NULL, (char*)delims);
    if (p_result)
    {
        v->satellite_num = atoi(p_result);
    }
    else
    {
        v->gps_signal = GPS_POWER_ON;
        return -4;
    }

    // 5. speed
    p_result = strtok( NULL, (char*)delims);
    if (p_result)
    {
        v->speed = atof(p_result);
    }
    else
    {
        v->gps_signal = GPS_POWER_ON;
        return -5;
    }

    // 6. no care about it
    p_result = strtok( NULL, (char*)delims);
    if (p_result)
    {
    }
    else
    {
        v->gps_signal = GPS_POWER_ON;
        return -6;
    }

    // 7. UTC time
    p_result = strtok( NULL, (char*)delims);
    if (p_result)
    {
        snprintf(v->utc_time, sizeof(v->utc_time), "%s", p_result);
    }
    else
    {
        v->gps_signal = GPS_POWER_ON;
        return -7;
    }

    // 8. GPS fix information
    p_result = strtok(NULL, (char*)delims);
    if (p_result)
    {
        i = atoi(p_result) + GPS_POWER_OFF;
        if (IS_GPS_STATE_TYPE(i))
        {
            v->gps_signal = (gps_state)i;
        }
        else
        {
            v->gps_signal = GPS_POWER_ON;
            return -8;
        }
    }
    else
    {
        v->gps_signal = GPS_POWER_ON;
        return -9;
    }

    return 0;

}

/*
 * @brief: deal with a new sms message
 *
 * +CMT: "+8613616212725","","14/03/17,20:53:58+32"
 * ggggggggggg
 */
static int  do_sms_cmd(char* p_str)
{
    char* delims = "\"\r\n";
    char* p_result;
    char  phone_number_buffer[16];

    p_result = p_str + strlen(ES201_RECV_NEW_MSG);
    memcpy(phone_number_buffer, p_result, 14);
    phone_number_buffer[14] = 0;

    APP_TRACE("phone number is: %s\r\n", phone_number_buffer);

    // 1. Don't care about the front part of the sms message
    p_result = strtok(p_str, (char*)delims);
    if (!p_result)
    {
        APP_TRACE("The sms message is wrong!\r\n");
        DUMP_DATA((uint8_t*)p_str, strlen(p_str));
    }

    // 2. the sms message
    p_result = strtok(NULL, (char*)delims);
    if (p_result)
    {
        APP_TRACE("The sms message: %s\r\n", p_result);
    }

    return 0;
}

int dispatchMessage(uint_8* pData)
{
	
	return 1;
}
bool IsImage()
{
	if(0 == memcmp(g_wifi_com_recv_buf, WIFI_IMAGE_TAG_HEADER, WIFI_IMAGE_TAG_LEN))
		return TRUE;
	else 
		return FALSE;
}
/**
  * @brief: deal with the recviced data
  * @param: length, reception character length
  * @retval: none
  * @note:
  * AT command example:
  * 1) GPS data packetage information
  *
  */
void    do_cmd_wifidata(uint8_t length)
{
    char*               p;
    int                 ret;
    es201_gps_handle    v;
    int                 state;

    // first, in order to deal with the string
    // we need add '0' at the end of string
    g_wifi_com_recv_buf[length] = 0;

    APP_TRACE((const char*)g_wifi_com_recv_buf);
    /*
		* Image has higher priority than the others message, so handle it firstly.
		*/
		 if(IsImage())
		 {
			 do_wifi_image_cmd(g_wifi_com_recv_buf);
			 
		 }
		 /*if it is not image related message(photo image and firmware image),
		 * and then this message must belong to normal case:
		 * %B1Q,ID, Parameter1, parmater2
		 * Is it necessary to use terminate flag?
		 */
		 p = strstr((const char*)g_wifi_com_recv_buf, WIFI_TAG_HEADER);
		 if(p)
		 {
			 ret=dispatchMessage(p);
		 }
		 /////////////////////////////
		
//		if(p)
//		{
//			 // skip to GPS data content
//       // p += strlen(GPS_HEADER_CMD);
//        ret = do_wifi_image_cmd(p);
//        if (ret == 0)
//        {
//            APP_TRACE("%.5f, %.5f, %.5f, %.5f, %d, %s, %d\r\n",
//                      v->longitude,
//                      v->latitude,
//                      v->altitude,
//                      v->speed,
//                      v->satellite_num,
//                      v->utc_time,
//                      (int)v->gps_signal);
//        }
//        else
//        {
//            APP_TRACE("get GPS signal error[%d]\r\n", ret);
//            APP_TRACE("%s", g_wifi_com_recv_buf);
//        }

//        return;
//		}
//	
//	//end of wifi parsing
//    v = get_es201_gps_instance();

//    // GPS no signal
//    // +GPSINFO: GPS NO SINGAL
//    p = strstr((const char*)g_wifi_com_recv_buf, GPS_NO_SIGNAL_CMD);
//    if (p)
//    {
//        v->gps_signal = GPS_POWER_ON;
//        return;
//    }


//    // send sms ind
//    p = strstr((const char*)g_wifi_com_recv_buf, ES201_SMS_SEND_IND);
//    if (p)
//    {
//        _lwsem_post(&g_at_cmd_send_sem);
//        return;
//    }

//    /*
//     * @brief: receive a new sms message
//     *
//     * +CMT: "+8613616212725","","14/03/17,20:53:58+32"
//     * ggggggggggg
//     */
//    p = strstr((const char*)g_wifi_com_recv_buf, ES201_RECV_NEW_MSG);
//    if (p)
//    {
//        do_sms_cmd((char*)g_wifi_com_recv_buf);
//    }

//    // ES201 power on information
//    // IIII....
//    p = strstr((const char*)g_wifi_com_recv_buf, ES201_POWER_ON_CMD);
//    if (p)
//    {
//        v->gps_signal = GPS_POWER_OFF;
//        v->gsm_state  = GSM_POWER_ON;

//        return;
//    }

//    // ES201 power state information
//    // +GPS: 1      ---> GPS is power on
//    // +GPS: 0      ---> GPS is power off
//    p = strstr((const char*)g_wifi_com_recv_buf, ES201_GPS_POWER_STATE);
//    if (p)
//    {
//        p += strlen(ES201_GPS_POWER_STATE);
//        state = atoi(p);
//        if (state == 1)
//            v->gps_signal = GPS_POWER_ON;
//        else
//            v->gps_signal = GPS_POWER_OFF;

//        return;
//    }

//    // normal AT cmd information
//    // OK
//    p = strstr((const char*)g_wifi_com_recv_buf, ES201_OK_AT_CMD);
//    if (p)
//    {
//        v->gsm_state = GSM_STATE_OK;
//        return;
//    }

//    // ERROR
//    p = strstr((const char*)g_wifi_com_recv_buf, ES201_ERROR_AT_CMD);
//    if (p)
//    {
//        v->gsm_state = GSM_STATE_ERROR;
//        return;
//    }

//    APP_TRACE("\r\n* ES201 data not handle:\r\n");
//    DUMP_DATA(g_wifi_com_recv_buf, length);

}


es201_gps_handle get_es201_gps_instance(void)
{
    return &g_es201_gps_dev;
}


void    send_sms_alarm_command(char* p_phone)
{
    _mqx_uint           ret;
    char                vl_buf[64];
    es201_gps_handle    v;
    uint8_t             i;

    v = get_es201_gps_instance();

    for (i = 0; i < 5; i++)
    {
        send_at_command("AT");
        _time_delay_ticks(200);
    }

    snprintf(vl_buf, sizeof vl_buf, "AT+CMGS=\"%s\"", p_phone);
    send_at_command(vl_buf);

    // wait '>'
    ret = _lwsem_wait_ticks(&g_at_cmd_send_sem, 200*3);
    if (ret != MQX_OK)
    {
        APP_TRACE("SMS wait '>' error!\r\n");
//        set_rgb_led_state(RGB_GREEN_LED | RGB_RED_LED);
    }
    else
    {
        snprintf(vl_buf, sizeof vl_buf, "!!!alarm: %.5f,%.5f,%.2f",
                                        v->longitude,
                                        v->latitude,
                                        v->speed);

        // OK, send the alarm message
        uart0_send_string((uint8_t*)vl_buf);
        // CTRL-Z
        uart0_send_data(0x1A);
    }

}


void    send_sms_alarm_message(void)
{
    es201_gps_handle        v;
    _mqx_uint               current_ticks;
    KERNEL_DATA_STRUCT_PTR  kernel_data;

    v = get_es201_gps_instance();

    _GET_KERNEL_DATA(kernel_data);

    current_ticks = kernel_data->TIME.TICKS[0];
    if (current_ticks - v->ticks >= 200*60*3)   // 3min
    {
        v->alarm = 0;
        v->ticks = current_ticks;
    }

    if (v->alarm == 0)
    {
        v->alarm = 1;
        // TODO : phone number need to be saved in flash
        send_sms_alarm_command("13616212725");
        _time_delay_ticks(200);
    }

}

void    get_gps_information(void)
{
    es201_gps_handle    v;

    v = get_es201_gps_instance();

    // update gps information
    if (v->gps_signal == GPS_POWER_ON || v->alarm)
    {
        send_at_command(ES201_GPS_INFO_AT_CMD);
    }

}

static void send_at_command(char* p_cmd)
{
    uart0_send_string((uint8*)p_cmd);
    uart0_send_data(0x0D);
}


/**
  * @brief: sending AT command to communication with ES201
  * 1. ES201 power off : blue led
  * 2. ES201 power on  : red led
  * 3. ES201 GPS signal: green led
  *
  */
void    app_es201_send_task(uint32_t task_init_data)
{
    _mqx_uint           ret;
    es201_gps_handle    v;
    int                 i;

    APP_TRACE("app_es201_send_task...\r\n");

   // set_rgb_led_state(RGB_BLUE_LED);
    v = get_es201_gps_instance();

    // delay 5 seconds...
    _time_delay_ticks(200*5);

    // send AT command to test with ES201
    for (i = 0; i < 5; i++)
    {
        // AT
        send_at_command(ES201_AT_TEST_CMD);
        _time_delay_ticks(200);
    }

    // get GSM network signal
    send_at_command(ES201_AT_CSQ_CMD);
    _time_delay_ticks(200);

    if (v->gsm_state < GSM_POWER_ON)
    {
        APP_TRACE("ES201 no response, please power on...\r\n");

        // wait 30 seconds
        for (i = 0; i < 30; i++)
        {
            _time_delay_ticks(200);
            if (v->gsm_state >= GSM_POWER_ON)
            {
                APP_TRACE("ES201 POWER ON!\r\n");

                // set RGB to purple
            //    set_rgb_led_state(RGB_RED_LED | RGB_BLUE_LED);
                // delay 7 seconds to wait ES201 ready
                _time_delay_ticks(200*7);
                break;
            }
        }

        if (i == 20)
        {
            // system error, send task return
            APP_TRACE("!!! ES201 POWER ON ERROR !!!\r\n");
            return;
        }
    }
    else
    {
        APP_TRACE("ES201 IS POWER ON ALREADY!\r\n");
    }

    // -------------------- set SMS direct show --------------------
    _time_delay_ticks(200);
    send_at_command(ES201_AT_CIMI);

    _time_delay_ticks(200);
    send_at_command(ES201_AT_CNMI);

    _time_delay_ticks(200);
    send_at_command(ES201_AT_CMGF);
    _time_delay_ticks(200);

    // ---------------------------- GPS ----------------------------
 //   set_rgb_led_state(RGB_RED_LED);

    // check GPS state
    for (i = 0; i < 5; i++)
    {
        if (v->gps_signal >= GPS_POWER_ON)
        {
            break;
        }

        send_at_command(GET_GPS_POWER_STATE);
        _time_delay_ticks(200);
    }

    if (i == 5)
    {
        APP_TRACE("open GPS power...\r\n");
        send_at_command(OPEN_GPS_POWER);
        _time_delay_ticks(200);
    }

    APP_TRACE("ES201 is ready to monitoring\r\n");
    for (;;)
    {
        ret = _lwsem_wait_ticks(&g_mma8451_alarm_sem, 200*3);
        switch (ret)
        {
            case MQX_OK:                    // send sms
                send_sms_alarm_message();
                break;
            case MQX_LWSEM_WAIT_TIMEOUT:    // check GPS signal
                get_gps_information();
                break;
        }
    }

}


void    app_wifi_control_task(uint32_t task_init_data)
{
    _mqx_uint           ret;
    _mqx_max_type       msg = 0;
    uint8_t             pos = 0;
    es201_gps_handle    v;
    _task_id            id;

    // create recv msg queue
    ret = _lwmsgq_init(g_uart0_rx_queue, WIFI_MSG_NUMBER, WIFI_MSG_SIZE);
    ASSERT_PARAM(MQX_OK == ret);

//    // create send AT command sem
//    ret = _lwsem_create(&g_at_cmd_send_sem, 0);
//    ASSERT_PARAM(MQX_OK == ret);

//    ret = _lwsem_create(&g_mma8451_alarm_sem, 0);
//    ASSERT_PARAM(MQX_OK == ret);

    // init UART0 to communication with ES201
    init_uart0(uart0_irq_handler);

    // clear es201 device instance
    v = get_es201_gps_instance();
    memset(v, 0x00, sizeof(es201_dev_info_t));

    // start ES201 at command send task
   // id = _task_create_at(0, ES201_SEND_TASK, 0, es201_send_task_stack, ES201_SEND_TASK_STACK_SIZE);
    ASSERT_PARAM(MQX_NULL_TASK_ID != id);

    APP_TRACE("app_es201_control_task start...\r\n");

    for (;;)
    {
        /* wait 5*5=25ms
				Adopt the easiest and stupid way to receive one complete message based on time out mechansim instead of finding "terminating" symbol.
			*/
        ret = _lwmsgq_receive(g_uart0_rx_queue, &msg, LWMSGQ_RECEIVE_BLOCK_ON_EMPTY, 5, NULL);
        switch (ret)
        {
            case MQX_OK:
                if (pos < WIFI_RECV_BUF_SIZE - 1)
                {// save data
                    g_wifi_com_recv_buf[pos++] = (uint8_t)msg;
                }
                else
                {
                    APP_TRACE("error: buffer full\r\n");
                    // data error, clear the buffer
                    pos = 0;
                }
                break;

            case LWMSGQ_TIMEOUT:
                if (pos)
                {
                    do_cmd_wifidata(pos);
                    // clear buffer, to reload next data reception
                    pos = 0;
                }
                break;

            default:
                APP_TRACE("g_uart0_rx_queue recv error: 0x%x\r\n", ret);
                break;

        }
    }
}



static void uart0_irq_handler(void* p_arg)
{
    _mqx_uint       ret;
    _mqx_max_type   msg;

    if (UART0_PDD_ReadInterruptStatusReg(UART0_BASE_PTR) & UART0_S1_RDRF_MASK)
    {
        msg = UART0_PDD_GetChar8(UART0_BASE_PTR);

        ret = _lwmsgq_send(g_uart0_rx_queue, &msg, 0);
        ASSERT_PARAM(MQX_OK == ret);
    }
}






