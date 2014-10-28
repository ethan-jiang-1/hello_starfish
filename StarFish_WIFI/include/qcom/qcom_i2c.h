/**
 * @file
 */
/******************************************************************************
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 * $ATH_LICENSE_HOSTSDK0_C$

 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the license is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the license for the specific language governing permissions and
 *    limitations under the license.
 ******************************************************************************/
#ifndef __CUST_I2C_H__
#define __CUST_I2C_H__
typedef enum {
	I2C_FREQ_400K 	=	6,
	I2C_FREQ_200K 	=	7,
	I2C_FREQ_100K 	=	8,
	I2C_FREQ_50K 	=	9
}I2C_FREQ;
typedef enum {
	I2C_SCK_1	=	4,
	I2C_SCK_2	=	8,
	I2C_SCK_3	=	12,
	I2C_SCK_4	=	18,
	I2C_SCK_5	=	20//currently used
}I2C_SCK_PIN;
typedef enum {
	I2C_SDA_1	=	3,
	I2C_SDA_2	=	9,
	I2C_SDA_3	=	13,
	I2C_SDA_4	=	19//currently used
}I2C_SDA_PIN;
/* i2c_init - initialize i2c interface. */
/**
 * This function is used to initialize i2c pins and frequency.
 *
 * @param sck_pin		sck gpio pin(4, 8, 12, 18, 20).
 * @param sda_pin		sda gpio pin(3, 9, 13, 19).
 * @param freq		i2c lock frequency(50, 100, 200, 400).
 */
A_INT32 qcom_i2c_init(I2C_SCK_PIN sck_pin, I2C_SDA_PIN sda_pin, I2C_FREQ freq);

/* qcom_i2c_read - read eeprom through i2c. */
/**
 * This function is used to read specific device address
 *
 * @param DevAddr   device address for I2C operations
 * @param addr		reg address to read.
 * @param data		pointer to chunk data.
 * @param length		length of chunk data, 0~8 byte is preferred
 *
 * @return 			1 on success. else on error.
 */
A_INT32 qcom_i2c_read(A_INT8 DevAddr , A_INT32 addr, A_UCHAR *data, A_INT32 length);
/* eeprom_i2c_write - write eeprom through i2c. */
/**
 * This function is used to write eeprom.
 *
 * @param addr		reg address to write.
 * @param data		pointer to chunk data.
 * @param length		length of chunk data, 0~6 byte is preferred
 *
 * @return 			1 on success. else on error.
 */
A_INT32 qcom_i2c_write(A_INT8 DevAddr, A_INT32 addr, A_UCHAR *data, A_INT32 length);
/*
 * Constant
 */
#define SI_OK       1
#define SI_ERR      0

#endif
