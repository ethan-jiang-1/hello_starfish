/*
 * Common.h
 */

#ifndef COMMON_H_
#define COMMON_H_

#include "PE_Types.h"

/* Build settings */
#define CONSOLE_IO_SUPPORT 1

/* MMA8451Q IOMap */
/* External 3-axis accelerometer data register addresses */
#define OUT_X_MSB 0x01
#define OUT_X_LSB 0x02
#define OUT_Y_MSB 0x03
#define OUT_Y_LSB 0x04
#define OUT_Z_MSB 0x05
#define OUT_Z_LSB 0x06
/* External 3-axis accelerometer control register addresses */
#define CTRL_REG_1 0x2A
/* External 3-axis accelerometer control register bit masks */
#define ACTIVE_BIT_MASK 0x01
#define F_READ_BIT_MASK 0x02

#define ACC_REG_SIZE 1U
#define READ_COUNT 5U

/* Blink rate constants */
#define OFFSETTICKS 700000U
#define DIVTICKS 80000U


typedef struct {
  volatile bool Sent;
  volatile bool Received;
} TDataState;

  typedef enum {
      VLPSMODE = 0,
      ACTIVE_STILL = 1,
      ACTIVE_OPEN = 2,
  }SYSTEM_STATUS;
	typedef enum {
				ACC_RUNNING=0,
				ACC_STILL=1,
				ACC_MOTION=2,
				ACC_STANDBY=3,
	}ACC_STATUS;
#endif /* COMMON_H_ */
