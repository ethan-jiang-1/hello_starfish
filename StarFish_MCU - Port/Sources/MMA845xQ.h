/*
 * MMA845xQ.h
 *
 *  Created on: Aug 14, 2012
 *      Author: B39601
 */

#ifndef MMA845XQ_H_
#define MMA845XQ_H_

#include <string.h>
#include "Cpu.h"
#include "Common.h"
#include "I2C.h"

bool ReadAccRegs(LDD_TDeviceData *I2CPtr, TDataState *DataState, uint8_t Address, uint8_t RegCount, uint8_t *Buffer);
bool WriteAccRegs(LDD_TDeviceData *I2CPtr, TDataState *DataState, uint8_t Address, uint8_t RegCount, uint8_t *Data);

 

#endif /* MMA845XQ_H_ */
