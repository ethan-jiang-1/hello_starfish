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


LDD_TDeviceData *I2C_DeviceData = NULL;
LDD_TDeviceData *PWMTimerRGB_DeviceData = NULL;
LDD_TDeviceData *PeriodicTimer = NULL;

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
void Init_Task(uint32_t task_init_data)
{
	
	  trace_init();
	 show_version_information();
	 _task_create_at(0, SHELL_TASK, 0, shell_task_stack, SHELL_TASK_STACK_SIZE);
	 _task_create_at(0, MMA8415_TASK, 0, mma8451_task_stack, MMA8451_TASK_STACK_SIZE);
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
    Error = !ReadAccRegs(I2C_DeviceData, &DataState, OUT_X_MSB, 3 * ACC_REG_SIZE, (uint8_t*) Color);  // Read x,y,z acceleration data.
    if (!Error) {
      if (!BlinkFlag) {
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
