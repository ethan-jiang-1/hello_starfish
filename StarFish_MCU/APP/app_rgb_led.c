/******************** (C) COPYRIGHT 2014 FRDM **********************************
 * File Name          : app_rgb_led.c
 * Author             : Bruce.zhu
 * Date First Issued  : 2014/03/13
 * Description        : This file contains the software implementation for the
 *                      RGB LED control unit
 *******************************************************************************
 * History:
 * DATE       | VER   | AUTOR      | Description
 * 2014/03/13 | v1.0  | Bruce.zhu  | initial released
 *******************************************************************************/
#include "app_rgb_led.h"

#include "MQX1.h"
#include "nvic.h"

/* MQX Lite include files */
#include "mqxlite.h"
#include "mqxlite_prv.h"
#include "mutex.h"
#include "lwmsgq.h"

#include "app_trace.h"
#include <string.h>
#include <stdlib.h>

uint8_t         rgb_led_task_stack[RGB_LED_TASK_STACK_SIZE];
static uint32_t g_rgb_led_state;


/*
 * PTB18 ---> RED LED
 * PTB19 ---> GREEN LED
 * PTD1  ---> BLUE LED
 */
static void init_rgb_led(void)
{
    // green led
    PORTB_PCR19 |= (1 << 8);
    GPIOB_PSOR |= (1<<19);
    GPIOB_PDDR |= (1<<19);

    // red led
    PORTB_PCR18 |= (1 << 8);
    GPIOB_PSOR |= (1<<18);
    GPIOB_PDDR |= (1<<18);

    // blue led
    PORTD_PCR1 |= (1 << 8);
    GPIOD_PSOR |= (1<<1);
    GPIOD_PDDR |= (1<<1);
}


/**
  * @brief: set RGB LED
  * new_state: rgb_led_state
  *            RGB_GREEN_LED
  *            RGB_RED_LED
  *            RGB_BLUE_LED
  */
void    set_rgb_led_state(uint32_t new_state)
{
    g_rgb_led_state = new_state;
}


void    app_rgb_led_task(uint32_t task_init_data)
{
    uint32_t temp = 0;

    APP_TRACE("RGB LED_task start...\r\n");

    init_rgb_led();

    for (;;)
    {
        _time_delay_ticks(200);

        temp = g_rgb_led_state;
        // check green led
        if (temp & RGB_GREEN_LED)
        {
            // GREEN LED ON
            GPIOB_PCOR |= (1<<19);
        }

        // check red led
        if (temp & RGB_RED_LED)
        {
            // RED LED ON
            GPIOB_PCOR |= (1<<18);
        }

        // check blue led
        if (temp & RGB_BLUE_LED)
        {
            // BLUE LED ON
            GPIOD_PCOR |= (1<<1);
        }

        _time_delay_ticks(10);

        // check green led
        if (temp & RGB_GREEN_LED)
        {
            // GREEN LED OFF
            GPIOB_PSOR |= (1<<19);
        }

        // check red led
        if (temp & RGB_RED_LED)
        {
            // RED LED OFF
            GPIOB_PSOR |= (1<<18);
        }

        // check blue led
        if (temp & RGB_BLUE_LED)
        {
            // BLUE LED OFF
            GPIOD_PSOR |= (1<<1);
        }
    }
}






