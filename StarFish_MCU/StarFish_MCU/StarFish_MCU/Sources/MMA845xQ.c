/*
 * MMA845xQ.c
 *
 *  Created on: Aug 14, 2012
 *      Author: B39601
 */
#include "MMA845xQ.h"

extern LDD_TDeviceData *I2C_DeviceData;

bool ReadAccRegs(LDD_TDeviceData *I2CPtr,
    TDataState *DataState, uint8_t Address, uint8_t RegCount, uint8_t *Buffer)
{
  LDD_I2C_TBusState BusState;
  DataState->Sent = FALSE;
  I2C_MasterSendBlock(I2CPtr, &Address, sizeof(Address), LDD_I2C_NO_SEND_STOP);
  while (!DataState->Sent) {}
  if (!DataState->Sent) {
    return FALSE;
  }
  DataState->Received = FALSE;
  I2C_MasterReceiveBlock(I2CPtr, Buffer, RegCount, LDD_I2C_SEND_STOP);
  while (!DataState->Received) {}
  do {I2C_CheckBus(I2CPtr, &BusState);}
  while (BusState != LDD_I2C_IDLE);
  if (!DataState->Received) {
    return FALSE;
  }
  return TRUE;
}

bool WriteAccRegs(LDD_TDeviceData *I2CPtr,
    TDataState *DataState, uint8_t Address, uint8_t RegCount, uint8_t *Data)
{
  LDD_I2C_TBusState BusState;
//  const uint8_t MAX_REG_COUNT = 16;
  //uint8_t SendBuffer[MAX_REG_COUNT];
  uint8_t SendBuffer[16];
  
  SendBuffer[0] = Address;
  memcpy(&SendBuffer[1], Data, RegCount);
  DataState->Sent = FALSE;
  I2C_MasterSendBlock(I2CPtr, &SendBuffer, RegCount + 1, LDD_I2C_SEND_STOP);
  while (!DataState->Sent) {}
  do {I2C_CheckBus(I2CPtr, &BusState);}
  while(BusState != LDD_I2C_IDLE); 
  if (!DataState->Sent) {
    return FALSE;
  }
  return TRUE;
}
