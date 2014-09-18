/******************** (C) COPYRIGHT 2014 FRDM **********************************
 * File Name          : app_rgb_led.h
 * Author             : Bruce.zhu
 * Date First Issued  : 2014/03/13
 * Description        : This file contains the software implementation for the
 *                      RGB LED control unit
 *******************************************************************************
 * History:
 * DATE       | VER   | AUTOR      | Description
 * 2014/03/13 | v1.0  | Bruce.zhu  | initial released
 *******************************************************************************/
#ifndef _APP_RGB_LED_H_
#define _APP_RGB_LED_H_

#include "PE_Types.h"



typedef enum
{
    RGB_GREEN_LED   = 0x01,
    RGB_RED_LED     = 0x02,
    RGB_BLUE_LED    = 0x04
}rgb_led_state;



void    set_rgb_led_state(uint32_t state);

void    app_rgb_led_task(uint32_t task_init_data);






#endif /* _APP_RGB_LED_H_ */

