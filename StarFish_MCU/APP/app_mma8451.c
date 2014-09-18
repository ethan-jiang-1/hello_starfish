/******************** (C) COPYRIGHT 2014 FRDM **********************************
 * File Name          : app_mma8451.c
 * Author             : Bruce.zhu
 * Date First Issued  : 2014/03/09
 * Description        : This file contains the software implementation for the
 *                      accel MMA8451 control unit
 *******************************************************************************
 * History:
 * DATE       | VER   | AUTOR      | Description
 * 2014/03/09 | v1.0  | Bruce.zhu  | initial released
 *******************************************************************************/

#include "app_mma8451.h"
#include "app_es201.h"

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

uint8_t mma8451_task_stack[MMA8451_TASK_STACK_SIZE];

static TDataState       g_DataState;
static LDD_TDeviceData *g_I2C_DeviceData = NULL;
static LWSEM_STRUCT     g_mma8451_int_sem;


static void MMA8451_INT1_isr_service(void* p_arg);

/**
  * @brief: init MMA8451 INT1 interrupt pin
  * @note:  PTA14
  *         PULL UP
  *         Interrupt on falling edge
  *
  */
void    init_PTA14_interrupt(void)
{
//    INT_ISR_FPTR isr;

//    // PTA14 -- MMA8451 INT1
//    // ALT1
//    /* PORTA_PCR14: ISF=0, MUX=1, IQRC=0x0A(1010), PE=1, PS=1 */
//    PORTA_PCR14 = 0x000A0103;
//    GPIOA_PDDR &= ~(1<<14);

//    isr = _int_install_isr(LDD_ivIndex_INT_PORTA, MMA8451_INT1_isr_service, NULL);
//    ASSERT_PARAM(isr != NULL);

//    // ENABLE PTA14 Falling interrupt
//    enable_irq(30);
//    set_irq_priority(30, 2);
}


static void mma8451_shake(void)
{
    byte            Data;
    LDD_TError      ret;

    // Determine source of interrupt by reading the system interrupt
    ret = ReadAccRegs(g_I2C_DeviceData,
                      &g_DataState,
                      INT_SOURCE_REG,
                      ACC_REG_SIZE,
                      &Data);
    if (ret)
    {
        // Set up Case statement here to service all of the possible interrupts
        if ((Data &0x04) == 0x04)
        {
            //Perform an Action since Motion Flag has been set
            //Read the Motion/Freefall Function to clear the interrupt
            ret = ReadAccRegs(g_I2C_DeviceData,
                              &g_DataState,
                              FF_MT_SRC_REG,
                              ACC_REG_SIZE,
                              &Data);
        }
    }

//    _lwsem_post(&g_mma8451_alarm_sem);

}


void    app_mma8451_control_task(uint32_t task_init_data)
{
    LDD_TError      ret;
    byte            Data;
    float           ths;
    _mqx_uint       mqx_ret;

    APP_TRACE("MMA8451 control task start...\r\n");

    // wait MMA8451 start...
    _time_delay_ticks(200*30);
    APP_TRACE("init MMA8451...\r\n");
    g_I2C_DeviceData = I2C_Init(&g_DataState);

    /*
     * read MMA8451 ID
     */
    ret = ReadAccRegs(g_I2C_DeviceData,
                      &g_DataState,
                      WHO_AM_I,
                      ACC_REG_SIZE,
                      &Data);
    if (ret)
    {
        APP_TRACE("MMA8451 ID: 0x%X\r\n", Data);
    }
    else
    {
        APP_TRACE("read MMA8451 ID error\r\n");
    }

    /*
     * Put the device into Standby Mode
     * Register 0x2A CTRL_REG1
     */
    // Set the device in 100 Hz ODR, Standby
    // 0001 1000
    // 0x18
    Data = 0x18;
    ret = WriteAccRegs(g_I2C_DeviceData, &g_DataState, CTRL_REG_1, ACC_REG_SIZE, &Data);
    if (!ret)
    {
        APP_TRACE("write MMA8451 [0x%.2X] error\r\n", CTRL_REG_1);
    }

    // read the 0x2A register again
    Data = 0;
    ret = ReadAccRegs(g_I2C_DeviceData,
                      &g_DataState,
                      CTRL_REG_1,
                      ACC_REG_SIZE,
                      &Data);
    if (Data != 0x18)
    {
        APP_TRACE("MMA8451 [0x%.2X] error[0x%.2X != 0x18]\r\n", CTRL_REG_1, Data);
    }

    /*
     * Configuring the MMA8451 for Motion Detection
     * FF/MT Config
     */
    // Enable Latch, Motion, X-axis, Y-axis
    // 1101 1000
    Data = 0xD8;
    ret = WriteAccRegs(g_I2C_DeviceData, &g_DataState, FF_MT_CFG_REG, ACC_REG_SIZE, &Data);
    if (ret == FALSE)
    {
        APP_TRACE("MMA8451 [0x%.2X] error\r\n", FF_MT_CFG_REG);
    }
    // read it and check the write value
    Data = 0;
    ret = ReadAccRegs(g_I2C_DeviceData,
                      &g_DataState,
                      FF_MT_CFG_REG,
                      ACC_REG_SIZE,
                      &Data);
    if (Data != 0xD8)
    {
        APP_TRACE("MMA8451 [0x%.2X] error [0x%.2X != 0xD8]\r\n", FF_MT_CFG_REG, Data);
    }

    /*
     * Setting the Threshold for Motion Detection
     * FF_MT_THS
     */
    //Set Threshold to counts
    ths = 0.3;
    Data = (byte)(ths/0.063);
    ret = WriteAccRegs(g_I2C_DeviceData, &g_DataState, FT_MT_THS_REG, ACC_REG_SIZE, &Data);
    if (ret == FALSE)
    {
        APP_TRACE("Setting the Threshold for Motion Detection error\r\n");
    }
    ret = ReadAccRegs(g_I2C_DeviceData,
                      &g_DataState,
                      FT_MT_THS_REG,
                      ACC_REG_SIZE,
                      &Data);
    if (ret)
    {
        APP_TRACE("Threshold [0x%.2X] = 0x%.2X[%d]\r\n", FT_MT_THS_REG, Data, Data);
    }

    /*
     * Setting theDebounce Counter
     * 100Hz = 0.01
     * 0x0A*0.01 = 0.1(100ms)
     */
    Data = 0x05;
    ret = WriteAccRegs(g_I2C_DeviceData, &g_DataState, FF_MT_COUNT_REG, ACC_REG_SIZE, &Data);
    if (ret == FALSE)
    {
        APP_TRACE("Setting [0x%.2X] error\r\n", FF_MT_COUNT_REG);
    }
    ret = ReadAccRegs(g_I2C_DeviceData,
                      &g_DataState,
                      FF_MT_COUNT_REG,
                      ACC_REG_SIZE,
                      &Data);
    if (Data != 0x0A)
    {
        APP_TRACE("MMA8451 [0x%.2X] = 0x%.2X != 0x0A\r\n", FF_MT_COUNT_REG, Data);
    }

    /*
    ** Enable the Freefall/Motion interrupt and route it to INT1.
    */
    Data = INT_EN_FF_MT_MASK;
    ret = WriteAccRegs(g_I2C_DeviceData, &g_DataState, CTRL_REG_4, ACC_REG_SIZE, &Data);
    if (ret == FALSE)
    {
        APP_TRACE("Motion interrupt error\r\n");
    }
    ret = ReadAccRegs(g_I2C_DeviceData,
                      &g_DataState,
                      CTRL_REG_4,
                      ACC_REG_SIZE,
                      &Data);
    if (Data != INT_EN_FF_MT_MASK)
    {
        APP_TRACE("MMA8451 [0x%.2X] = 0x%.2X != 0x0A\r\n", CTRL_REG_4, Data);
    }

    Data = INT_CFG_FF_MT_MASK;
    ret = WriteAccRegs(g_I2C_DeviceData, &g_DataState, CTRL_REG_5, ACC_REG_SIZE, &Data);
    if (ret == FALSE)
    {
        APP_TRACE("route it to INT1 error\r\n");
    }
    ret = ReadAccRegs(g_I2C_DeviceData,
                      &g_DataState,
                      CTRL_REG_5,
                      ACC_REG_SIZE,
                      &Data);
    if (Data != INT_CFG_FF_MT_MASK)
    {
        APP_TRACE("MMA8451 [0x%.2X] = 0x%.2X != 0x0A\r\n", CTRL_REG_5, Data);
    }

    // Put the device in Active Mode
    ret = ReadAccRegs(g_I2C_DeviceData,
                      &g_DataState,
                      CTRL_REG_1,
                      ACC_REG_SIZE,
                      &Data);
    if (ret)
    {
        Data |= 0x01;
        ret = WriteAccRegs(g_I2C_DeviceData, &g_DataState, CTRL_REG_1, ACC_REG_SIZE, &Data);
        if (!ret)
        {
            APP_TRACE("write MMA8451 [0x%.2X] error\r\n", CTRL_REG_1);
        }
    }
    else
    {
        APP_TRACE("Put the device in Active Mode error\r\n", CTRL_REG_5, Data);
    }

    APP_TRACE("MMA8451 Configuring Motion Detection OK!\r\n");

 //   init_PTA14_interrupt();

    mqx_ret = _lwsem_create(&g_mma8451_int_sem, 0);
    ASSERT_PARAM(MQX_OK == mqx_ret);

    // task begin
    for (;;)
    {
        mqx_ret = _lwsem_wait(&g_mma8451_int_sem);
        ASSERT_PARAM(MQX_OK == mqx_ret);

        APP_TRACE("MMA8451 shake!\r\n");
        mma8451_shake();
    }
}


static void MMA8451_INT1_isr_service(void* p_arg)
{
    // PTA14
    if (PORTA_ISFR & ((uint32_t)(1<<14)))
    {
        // writte 1 to the flag to clear the interrupt flag
        PORTA_ISFR = (uint32_t)(1<<14);
        _lwsem_post(&g_mma8451_int_sem);
    }
}




