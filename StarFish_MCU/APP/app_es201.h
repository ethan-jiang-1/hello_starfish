/******************** (C) COPYRIGHT 2014 FRDM **********************************
 * File Name          : app_es201.c
 * Author             : Bruce.zhu
 * Date First Issued  : 2014/03/03
 * Description        : This file contains the software implementation for the
 *                      es201 module control unit
 *******************************************************************************
 * History:
 * DATE       | VER   | AUTOR      | Description
 * 2014/03/03 | v1.0  | Bruce.zhu  | initial released
 *******************************************************************************/

#ifndef _APP_ES201_H_
#define _APP_ES201_H_

#include "uart.h"
#include "MQX1.h"

/* MQX Lite include files */
#include "mqxlite.h"
#include "mqxlite_prv.h"
#include "mutex.h"
#include "lwmsgq.h"
#include "lwtimer.h"
#include "psp_prv.h"



typedef enum
{
    GPS_POWER_OFF,          /* 0 GPS in power off(default)  */
    GPS_POWER_ON,           /* 1 GPS is on, but no signal   */
    GPS_2D_FIX,             /* 2 2D fix                     */
    GPS_3D_FIX              /* 3 3D fix                     */
}gps_state;

#define IS_GPS_STATE_TYPE(TYPE)    (((TYPE) == GPS_POWER_OFF) || \
                                    ((TYPE) == GPS_POWER_ON)  || \
                                    ((TYPE) == GPS_2D_FIX)  || \
                                    ((TYPE) == GPS_3D_FIX))

typedef enum
{
    GSM_POWER_OFF,          /* 0 ES201 power off(default)   */
    GSM_POWER_ON,           /* 1 ES201 power on             */
    GSM_SENDING_AT_CMD,     /* 2 sending AT command         */
    GSM_STATE_IDLE,         /* 3 ES201 in idle state        */
    GSM_STATE_OK,           /* 4 OK                         */
    GSM_STATE_ERROR         /* 5 ERROR                      */
}gsm_state;


typedef struct
{
    // GPS related information
    gps_state   gps_signal;         // GPS signal state
    float       longitude;          // longitude, 122.07391
    float       latitude;           // latitude, 37.52375
    float       altitude;           // altitude, 53.87
    float       speed;              // device speed
    int         satellite_num;      // satellite number
    char        utc_time[16];       // 140215220304

    // GSM state
    gsm_state   gsm_state;

    // alarm state
    char        alarm;

    // ticks
    uint_64     ticks;
}es201_dev_info_t;


typedef es201_dev_info_t *es201_gps_handle;


extern LWSEM_STRUCT             g_mma8451_alarm_sem;



void    app_es201_control_task(uint32_t task_init_data);
void    app_es201_send_task(uint32_t task_init_data);

es201_gps_handle get_es201_gps_instance(void);



#endif /* _APP_ES201_H_ */

