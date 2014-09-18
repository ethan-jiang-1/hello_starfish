/******************** (C) COPYRIGHT 2014 FRDM **********************************
 * File Name          : app_mma8451.h
 * Author             : Bruce.zhu
 * Date First Issued  : 2014/03/09
 * Description        : This file contains the software implementation for the
 *                      accel MMA8451 control unit
 *******************************************************************************
 * History:
 * DATE       | VER   | AUTOR      | Description
 * 2014/03/09 | v1.0  | Bruce.zhu  | initial released
 *******************************************************************************/

#ifndef _APP_MMA8451_H_
#define _APP_MMA8451_H_

#include "PE_Types.h"
#include "MMA845xQ.h"

/* MMA8451Q IOMap */

/* External 3-axis accelerometer data register addresses */
#define OUT_X_MSB           0x01
#define OUT_X_LSB           0x02
#define OUT_Y_MSB           0x03
#define OUT_Y_LSB           0x04
#define OUT_Z_MSB           0x05
#define OUT_Z_LSB           0x06

/*
**  INT_SOURCE System Interrupt Status Register
*/
#define INT_SOURCE_REG      0x0C

#define SRC_ASLP_MASK       0x80
#define SRC_FIFO_MASK       0x40
#define SRC_TRANS_MASK      0x20
#define SRC_LNDPRT_MASK     0x10
#define SRC_PULSE_MASK      0x08
#define SRC_FF_MT_1_MASK    0x04
#define SRC_FF_MT_2_MASK    0x02
#define SRC_DRDY_MASK       0x01

/* sets the dynamic range and sets the high-pass filter for the output data */
#define WHO_AM_I            0x0D

/* control register addresses */
#define CTRL_REG_1          0x2A
#define CTRL_REG_2          0x2B

/* control register 3 addresses */
#define CTRL_REG_3          0x2C

#define FIFO_GATE_MASK      0x80
#define WAKE_TRANS_MASK     0x40
#define WAKE_LNDPRT_MASK    0x20
#define WAKE_PULSE_MASK     0x10
#define WAKE_FF_MT_1_MASK   0x08
#define WAKE_FF_MT_2_MASK   0x04
#define IPOL_MASK           0x02
#define PP_OD_MASK          0x01

/* control register 4 addresses */
#define CTRL_REG_4          0x2D

#define INT_EN_ASLP_MASK    0x80
#define INT_EN_FIFO_MASK    0x40
#define INT_EN_TRANS_MASK   0x20
#define INT_EN_LNDPRT_MASK  0x10
#define INT_EN_PULSE_MASK   0x08
#define INT_EN_FF_MT_MASK   0x04
#define INT_EN_DRDY_MASK    0x01

/* control register 5 addresses */
#define CTRL_REG_5          0x2E

#define INT_CFG_ASLP_MASK   0x80
#define INT_CFG_FIFO_MASK   0x40
#define INT_CFG_TRANS_MASK  0x20
#define INT_CFG_LNDPRT_MASK 0x10
#define INT_CFG_PULSE_MASK  0x08
#define INT_CFG_FF_MT_MASK  0x04
#define INT_CFG_DRDY_MASK   0x01

/*
**  FF_MT_CFG Freefall and Motion Configuration Registers
*/
#define FF_MT_CFG_REG       0x15

#define ELE_MASK            0x80
#define OAE_MASK            0x40
#define ZEFE_MASK           0x20
#define YEFE_MASK           0x10
#define XEFE_MASK           0x08


/*
**  FF_MT_SRC Freefall and Motion Source Registers
*/
#define FF_MT_SRC_REG       0x16

#define EA_MASK             0x80
#define ZHE_MASK            0x20
#define ZHP_MASK            0x10
#define YHE_MASK            0x08
#define YHP_MASK            0x04
#define XHE_MASK            0x02
#define XHP_MASK            0x01


/*
**  FF_MT_THS Freefall and Motion Threshold Registers
**  TRANSIENT_THS Transient Threshold Register
*/
#define FT_MT_THS_REG       0x17
#define TRANSIENT_THS_REG   0x1F

#define DBCNTM_MASK         0x80
#define THS6_MASK           0x40
#define THS5_MASK           0x20
#define THS4_MASK           0x10
#define THS3_MASK           0x08
#define THS2_MASK           0x04
#define TXS1_MASK           0x02
#define THS0_MASK           0x01
#define THS_MASK            0x7F


/*
 * Setting theDebounce Counter
 */
#define FF_MT_COUNT_REG     0x18



/*
**  XYZ_DATA_CFG Sensor Data Configuration Register
*/
#define XYZ_DATA_CFG        0x0E
#define HPF_OUT_MASK        0x10
#define FS1_MASK            0x02
#define FS0_MASK            0x01
#define FS_MASK             0x03

#define FULL_SCALE_8G       FS1_MASK
#define FULL_SCALE_4G       FS0_MASK
#define FULL_SCALE_2G       0x00


/* External 3-axis accelerometer control register bit masks */
#define ACTIVE_BIT_MASK     0x01
#define F_READ_BIT_MASK     0x02

#define ACC_REG_SIZE        1U


void    app_mma8451_control_task(uint32_t task_init_data);







#endif /* _APP_MMA8451_H_ */

