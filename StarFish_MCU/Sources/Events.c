/** ###################################################################
**     Filename    : Events.c
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
**         IO1_OnBlockReceived               - void IO1_OnBlockReceived(LDD_TUserData *UserDataPtr);
**         IO1_OnBlockSent                   - void IO1_OnBlockSent(LDD_TUserData *UserDataPtr);
**         TSS1_fOnFault                     - void TSS1_fOnFault(byte u8FaultElecNum);
**         TSS1_fOnInit                      - void TSS1_fOnInit(void);
**         TSS1_fCallBack0                   - void TSS1_fCallBack0(TSS_CONTROL_ID u8ControlId);
**         BlinkRateCounter_OnCounterRestart - void BlinkRateCounter_OnCounterRestart(LDD_TUserData *UserDataPtr);
**         I2C_OnMasterBlockSent             - void I2C_OnMasterBlockSent(LDD_TUserData *UserDataPtr);
**         I2C_OnMasterBlockReceived         - void I2C_OnMasterBlockReceived(LDD_TUserData *UserDataPtr);
**         I2C_OnBusStopDetected             - void I2C_OnBusStopDetected(LDD_TUserData *UserDataPtr);
**         Cpu_OnNMIINT00                    - void Cpu_OnNMIINT00(void);
**
** ###################################################################*/
/* MODULE Events */

#include "Cpu.h"
#include "Events.h"
#include "mqx_tasks.h"

/* User includes (#include below this line is not maintained by Processor Expert) */
#include "Common.h"

extern volatile bool BlinkFlag;
extern LDD_TDeviceData *PWMTimerRGB_DeviceData;
extern LDD_TDeviceData *PeriodicTimer;
 


/*
** ===================================================================
**     Event       :  BlinkRateCounter_OnCounterRestart (module Events)
**
**     Component   :  BlinkRateCounter [TimerUnit_LDD]
**     Description :
**         Called if counter overflow/underflow or counter is
**         reinitialized by modulo or compare register matching.
**         OnCounterRestart event and Timer unit must be enabled. See
**         <SetEventMask> and <GetEventMask> methods. This event is
**         available only if a <Interrupt> is enabled.
**     Parameters  :
**         NAME            - DESCRIPTION
**       * UserDataPtr     - Pointer to the user or
**                           RTOS specific data. The pointer passed as
**                           the parameter of Init method.
**     Returns     : Nothing
** ===================================================================
*/
void BlinkRateCounter_OnCounterRestart(LDD_TUserData *UserDataPtr)
{
  BlinkFlag ^= 1;
  if (BlinkFlag) {
    PWMTimerRGB_Disable(PWMTimerRGB_DeviceData);
  }
}

/*
** ===================================================================
**     Event       :  I2C_OnMasterBlockSent (module Events)
**
**     Component   :  I2C [I2C_LDD]
**     Description :
**         This event is called when I2C in master mode finishes the
**         transmission of the data successfully. This event is not
**         available for the SLAVE mode and if MasterSendBlock is
**         disabled. 
**     Parameters  :
**         NAME            - DESCRIPTION
**       * UserDataPtr     - Pointer to the user or
**                           RTOS specific data. This pointer is passed
**                           as the parameter of Init method.
**     Returns     : Nothing
** ===================================================================
*/
void I2C_OnMasterBlockSent(LDD_TUserData *UserDataPtr)
{
  TDataState *DataState = (TDataState*)UserDataPtr;
  DataState->Sent = TRUE;
}

/*
** ===================================================================
**     Event       :  I2C_OnMasterBlockReceived (module Events)
**
**     Component   :  I2C [I2C_LDD]
**     Description :
**         This event is called when I2C is in master mode and finishes
**         the reception of the data successfully. This event is not
**         available for the SLAVE mode and if MasterReceiveBlock is
**         disabled.
**     Parameters  :
**         NAME            - DESCRIPTION
**       * UserDataPtr     - Pointer to the user or
**                           RTOS specific data. This pointer is passed
**                           as the parameter of Init method.
**     Returns     : Nothing
** ===================================================================
*/
void I2C_OnMasterBlockReceived(LDD_TUserData *UserDataPtr)
{
  TDataState *DataState = (TDataState*)UserDataPtr;
  DataState->Received = TRUE;
}

/*
** ===================================================================
**     Event       :  Cpu_OnNMIINT (module Events)
**
**     Component   :  Cpu [MKL26Z256LH4]
**     Description :
**         This event is called when the Non maskable interrupt had
**         occurred. This event is automatically enabled when the <NMI
**         interrrupt> property is set to 'Enabled'.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void Cpu_OnNMIINT(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  I2C_OnError (module Events)
**
**     Component   :  I2C [I2C_LDD]
*/
/*!
**     @brief
**         This event is called when an error (e.g. Arbitration lost)
**         occurs. The errors can be read with GetError method.
**     @param
**         UserDataPtr     - Pointer to the user or
**                           RTOS specific data. This pointer is passed
**                           as the parameter of Init method.
*/
/* ===================================================================*/
void I2C_OnError(LDD_TUserData *UserDataPtr)
{
  /* Write your code here ... */
}

/* END Events */

/*
** ###################################################################
**
**     This file was created by Processor Expert 10.0 [05.03]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
