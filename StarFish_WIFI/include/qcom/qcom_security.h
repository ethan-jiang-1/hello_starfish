/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 * $ATH_LICENSE_HOSTSDK0_C$
 */

#ifndef __QCOM__SECURITY_H__
#define __QCOM__SECURITY_H__

void qcom_sec_md5_init(void);
void qcom_sec_md5_update(unsigned char *data, int len);
void qcom_sec_md5_final(char md5_val[]);

#endif
