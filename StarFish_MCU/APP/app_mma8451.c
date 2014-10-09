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
#include "acos.h"

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
    INT_ISR_FPTR isr;

    // PTA14 -- MMA8451 INT1
    // ALT1
    /* PORTA_PCR14: ISF=0, MUX=1, IQRC=0x0A(1010), PE=1, PS=1 */
		/*zga: in kl25, this pin is connected from chip to mcu*/
    PORTA_PCR14 = 0x000A0103;
    GPIOA_PDDR &= ~(1<<14);

    isr = _int_install_isr(LDD_ivIndex_INT_PORTA, MMA8451_INT1_isr_service, NULL);
    ASSERT_PARAM(isr != NULL);

    // ENABLE PTA14 Falling interrupt
    enable_irq(30);
    set_irq_priority(30, 2);
}

/*
+1000 --> 75 degree
-1000 --> 105 degree=180-75.
+3999--->angle 0
+34---> angle 90 
*/
uint_8 angle_calculation(int accel_z, byte sign_bit)
{
	byte angle=0;
	accel_z=abs(accel_z);
	if (accel_z>3999)
			accel_z=3999;
	for(angle=0;angle<89;angle++)
	{
		if(accel_z<=acos_array[angle]&&accel_z>acos_array[angle+1])
		{
			if (sign_bit==1)
			return (angle+1);
			else
				return (180-angle);
		}
	}
		/*if angle=89, jump the above loop. so it is must be 90.*/
			if(angle==89)
				return (angle+1);
			return -1; // return error code.
	
}

static void mma8451_getdata(void)
{
    byte            Data;
    LDD_TError      ret;
		byte Color[192];
		signed short accel_x, accel_y, accel_z=0;
		int i,j=0;
		uint_8 sign_bit=0;
		uint_8 angle=0;
    // Determine source of interrupt by reading the system interrupt
    ret = ReadAccRegs(g_I2C_DeviceData,
                      &g_DataState,
                      INT_SOURCE_REG,
                      ACC_REG_SIZE,
                      &Data);
    if (ret)
    {
			// APP_TRACE("interrupt source: 0x%X\r\n", Data);
        // Set up Case statement here to service all of the possible interrupts
        if ((Data &SRC_FIFO_MASK) == 0x40)
        {
	
					ret = ReadAccRegs(g_I2C_DeviceData, &g_DataState, OUT_X_MSB, 192 * ACC_REG_SIZE, Color);  // Read x,y,z acceleration data.

					for(i=0;i<32;i++)
					{
						j=6*i;
					accel_x   = Color[j+1] | (Color[j+0]<<8);
					accel_x >>= 2;
					
					accel_y   = Color[j+3] | (Color[j+2]<<8);
					accel_y >>= 2;
					
					
					accel_z   = Color[j+5] | (Color[j+4]<<8);
					accel_z >>= 2;
					if(accel_z>=0)
						sign_bit=1;
					else
						sign_bit=0;
					angle=angle_calculation(accel_z,sign_bit);	
					APP_TRACE("tilt %d,y=%05d,z=%05d\r\n", angle, accel_y, accel_z);
					}
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
    _time_delay_ticks(2*30);
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

   /*write 2g mode ---XYZ_DATA_CFG*/
		Data=0x0;
		WriteAccRegs(g_I2C_DeviceData, &g_DataState, XYZ_DATA_CFG, ACC_REG_SIZE, &Data);
	 
		/* Set F_Mode to Circular, Set the Watermark Value into the F_SETUP register
		IIC_RegWrite(F_SETUP_REG, F_MODE0_MASK + WATERMARK_VAL);
		F_SETUP_REG=0x09,circle mode.
		F_MODE0_MASK=0100_0000;
		WATERMARK_VAL=32, 0010_0000 
		*/
		Data=0x60;
		WriteAccRegs(g_I2C_DeviceData, &g_DataState, 0x09, ACC_REG_SIZE, &Data);
		
		 /*
    ** Enable the fifo interrupt and route it to INT1.
    */
    Data = INT_EN_FIFO_MASK;//INT_EN_FF_MT_MASK;
    ret = WriteAccRegs(g_I2C_DeviceData, &g_DataState, CTRL_REG_4, ACC_REG_SIZE, &Data);
    if (ret == FALSE)
    {
        //APP_TRACE("Motion interrupt error\r\n");
			APP_TRACE("Data ready interrupt error\r\n");
    }
    ret = ReadAccRegs(g_I2C_DeviceData,
                      &g_DataState,
                      CTRL_REG_4,
                      ACC_REG_SIZE,
                      &Data);
    if (Data != INT_EN_FIFO_MASK)//INT_EN_FF_MT_MASK)
    {
        APP_TRACE("MMA8451 [0x%.2X] = 0x%.2X != 0x0A\r\n", CTRL_REG_4, Data);
    }

    Data = INT_CFG_FIFO_MASK;//INT_CFG_FF_MT_MASK;
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
    if (Data != INT_CFG_FIFO_MASK)
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

    init_PTA14_interrupt();

    mqx_ret = _lwsem_create(&g_mma8451_int_sem, 0);
    ASSERT_PARAM(MQX_OK == mqx_ret);

    // task begin
    for (;;)
    {
        mqx_ret = _lwsem_wait(&g_mma8451_int_sem);
        ASSERT_PARAM(MQX_OK == mqx_ret);
				mma8451_getdata();
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




