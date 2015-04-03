/** ###################################################################
**     Filename    : mqx_tasks.c
**     Project     : ProcessorExpert
**     Processor   : MKL26Z128VLH4
**     Component   : Events
**     Version     : Driver 01.00
**     Compiler    : ARM C/C++ Compiler
**     Date/Time   : 2012-09-06, 13:09, # CodeGen: 9
**     Abstract    :
**         This is user's event module.
**         Put your event handler code here.
**     Settings    :
**     Contents    :
**         Init_Task   - void Init_Task(uint32_t task_init_data);
**         TSS_Trigger - void TSS_Trigger(uint32_t task_init_data);
**         ColorTask   - void ColorTask(uint32_t task_init_data);
**
** ###################################################################*/
/* MODULE mqx_tasks */

#include "Cpu.h"
#include "Events.h"
#include "mqx_tasks.h"

/* User includes (#include below this line is not maintained by Processor Expert) */
#include <cstdlib>
#include "Common.h"
#include "MMA845xQ.h"
#include "app_trace.h"
#include "app_mma8451.h"
/////////////////////////zga add
#include "lptmr.h"
#include "adc.h"
#include "smc.h"
#include "DMA1.H"
#include "nvic.h"
///////////zga dma test
#include "MKL_spi.h"
#include "w25x32.h"
#include "eink_display.h"
#include "partition.h"

LDD_TDeviceData *I2C_DeviceData = NULL;
LDD_TDeviceData *PWMTimerRGB_DeviceData = NULL;
LDD_TDeviceData *PeriodicTimer = NULL;
//////add for lptmgr
extern volatile  uint8_t Measured;
extern LWSEM_STRUCT     g_lptmr_int_sem;
extern volatile bool bSysActive;
extern volatile bool bVLPSMode;
extern void enablemma8451standby();
extern void enablemma8451detect();
extern void enablemma8451running();
//////////////
void SetSysStatus(uint_8 sys);
////////////////dma end test
/////////////////end for lptmgr
volatile bool BlinkFlag = FALSE;
TDataState DataState;
static void show_version_information(void)
{
    char vl_buf[64];

    APP_TRACE("\r\n====================================================\r\n");
    snprintf((char*)vl_buf, sizeof(vl_buf), "* MQXlite RTOS1    : [%s]", MQX_LITE_VERSION);
    APP_TRACE("%-51s*\r\n", vl_buf);
    snprintf((char*)vl_buf, sizeof(vl_buf), "* Firmware Version: [%s]", "1.0.0.0");
    APP_TRACE("%-51s*\r\n", vl_buf);
    snprintf((char*)vl_buf, sizeof(vl_buf), "* Hardware Version: [%s]", "FRDM_REV_E");
    APP_TRACE("%-51s*\r\n", vl_buf);
    snprintf((char*)vl_buf, sizeof(vl_buf), "* Created Date    : %s", _mqx_date);
    APP_TRACE("%-51s*\r\n", vl_buf);
    snprintf((char*)vl_buf, sizeof(vl_buf), "*");
    APP_TRACE("%-51s*\r\n", vl_buf);
    snprintf((char*)vl_buf, sizeof(vl_buf), "* (C) COPYRIGHT 2014 FRDM KL25Z");
    APP_TRACE("%-51s*\r\n", vl_buf);
    APP_TRACE("====================================================\r\n\r\n");

}


/*
** ===================================================================
**     Event       :  Init_Task (module mqx_tasks)
**
**     Component   :  Task1 [MQXLite_task]
**     Description :
**         MQX task routine. The routine is generated into mqx_tasks.c
**         file.
**     Parameters  :
**         NAME            - DESCRIPTION
**         task_init_data  - 
**     Returns     : Nothing
** ===================================================================
*/
uint_8 sysStatus=0;
extern uint16_t MeasuredValues[ADC_CHANNELS_COUNT];
uint_8 GetTemperature()
{
	return 1;
}
bool GetLightON()
{
	return FALSE;
}

bool GetTouchON()
{
	return TRUE;
}
uint_8 GetSysStatus()
{
	return sysStatus;
}
void SetSysStatus(uint_8 sys)
{
	sysStatus=sys;
}
int  get_rect_data( int len)
{
	
	unsigned int i,j=0;
	
	for(i = 0; i < len; i++)
	{  
		  exchange_buff[i] = 0x00;	//局部刷黑
	}
	
	return len;
	
}

void Init_Task(uint32_t task_init_data)
{
		int tester=0;
	//uint_8 sys=0;
	bool bInitOpen=FALSE;
	bool bInitStill=FALSE;
	bool bInitVLPS=FALSE;
	int i=0;
	int spiid=0;
	
	uint8_t BufferW[10];
	uint8_t BufferR[10];
	MQX_TICK_STRUCT ttt;
	 _mqx_uint       mqx_ret;
	  trace_init();
			Lptmr_Init(5000, LPOCLK);		
		Lptmr_Start();
	 show_version_information();
	////init spi flash
	spiInit(SPI0_BASE_PTR , Master);                                    /* init spi*/
  spiid = flash_read_id();
	flash_write_status(BLOCK_PROTECTION_LOCK_DOWN_NULL);
	/////////////end spi flash
	
	/*init eink*/
	
	eink_init(); //初始化操作
			
			while(0)
			{
// only for the first boot up time
  eink_display_full(0xff); //全屏刷白
  eink_display_full(0x00); //全屏刷黑
  eink_display_full(0xff); //全屏刷白
//end of boot up time.

 //在起点为（120,200）的位置刷出(120*157)大小的黑框 
 //请注意之前必须全屏刷白才有效果
 { 
		  struct display_rect rect;
		
		  rect.x = 120;
		  rect.y = 200;
		  rect.w=  120;
	  	rect.h = 157;
		  eink_display(&rect, 0, get_rect_data); 
}
 
 { 
		  struct display_rect rect;
		
		  rect.x = 0;
		  rect.y = 0;
		  rect.w=  360;
	  	rect.h = 600;
		  eink_display(&rect, 0, eink_getdata); 
}
 

}
	///////////////
		//////////////zga add
	//Set LPTMR to timeout about 5 seconds

		ADC_Init();
		Calibrate_ADC();
		ADC_Init();
		DMA1_Init();
	//////////////zga add
		// clear flag  
		APP_TRACE("start 1\n\r");
	 _task_create_at(0, SHELL_TASK, 0, shell_task_stack, SHELL_TASK_STACK_SIZE);
	 _task_create_at(0, MMA8415_TASK, 0, mma8451_task_stack, MMA8451_TASK_STACK_SIZE);
	  _task_create_at(0, WIFI_TASK, 0, wifi_task_stack, WIFI_TASK_STACK_SIZE);

	
	

for(;;)
	{
		 mqx_ret = _lwsem_wait(&g_lptmr_int_sem);

	
	//	_time_delay_ticks(10);
		tester++;
//_time_delay_ticks(10);
		//APP_TRACE("tester is: %d\r\n",tester);
		_time_get_elapsed_ticks(&ttt);
          APP_TRACE("high ttt %d, low ttt%d\r\n", ttt.TICKS[1],ttt.TICKS[0]);
		APP_TRACE("flash id 0x%x\r\n", spiid);
		if(Measured)
		{	Measured=0;
				APP_TRACE ("%d, %d ,%d \r\n", (uint16_t) MeasuredValues[0],(uint16_t) MeasuredValues[1],tester);
		}
		if((GetTouchON()==TRUE))
			{
				SetSysStatus(ACTIVE_OPEN);
			}
		switch (sysStatus)
	{
		case ACTIVE_OPEN:
					bInitStill=FALSE;
					bInitVLPS=FALSE;
					APP_TRACE ("ACTIVE_OPEN\r\n");
					if(bInitOpen==FALSE)
					{
						bInitOpen=TRUE;
					//	enablemma8451running();
						SysTick_PDD_EnableDevice(SysTick_BASE_PTR, PDD_ENABLE);
					}
					
					
			break;

    case ACTIVE_STILL:
					bInitOpen=FALSE;
					bInitVLPS=FALSE;
				APP_TRACE ("ACTIVE_still\r\n");
					if(bInitStill==FALSE)
					{
						bInitStill=TRUE;
						enablemma8451detect();
					}
					enter_vlps();
		case 	VLPSMODE:
					bInitOpen=FALSE;
					bInitStill=FALSE;
				APP_TRACE ("vlpsmode\r\n");
					if(bInitVLPS==FALSE)
					{
						bInitVLPS=TRUE;
						enablemma8451standby();
					}
					enter_vlps();
    default:
            break;
	}


	} 
	
}


/*
** ===================================================================
**     Event       :  ColorTask (module mqx_tasks)
**
**     Component   :  Task3 [MQXLite_task]
**     Description :
**         MQX task routine. The routine is generated into mqx_tasks.c
**         file.
**     Parameters  :
**         NAME            - DESCRIPTION
**         task_init_data  - 
**     Returns     : Nothing
** ===================================================================
*/
void ColorTask(uint32_t task_init_data)
{
  LDD_TError Error;
  signed char Color[3];
  while(1)
  {
   // Error = !ReadAccRegs(I2C_DeviceData, &DataState, OUT_X_MSB, 3 * ACC_REG_SIZE, (uint8_t*) Color);  // Read x,y,z acceleration data.
   // if (!Error)
		{
    //  if (!BlinkFlag)
			{
        PWMTimerRGB_Enable(PWMTimerRGB_DeviceData);
        PWMTimerRGB_SetOffsetTicks(PWMTimerRGB_DeviceData, 0,1000*(1<<(abs(Color[0]/10)))); // x axis - red LED 
        PWMTimerRGB_SetOffsetTicks(PWMTimerRGB_DeviceData, 1, 1000*(1<<(abs(Color[1]/10)))); // y axis - green LED 
        PWMTimerRGB_SetOffsetTicks(PWMTimerRGB_DeviceData, 2, 1000*(1<<(abs(Color[2]/10)))); // z axis - blue LED
      }
    }
  _time_delay_ticks(1);
  }
}

/* END mqx_tasks */

/*
** ###################################################################
**
**     This file was created by Processor Expert 10.0 [05.03]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
