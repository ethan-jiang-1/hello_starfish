

#ifndef _APP_WIFI_H_
#define _APP_WIFI_H_

#include "uart.h"
#include "MQX1.h"

/* MQX Lite include files */
#include "mqxlite.h"
#include "mqxlite_prv.h"
#include "mutex.h"
#include "lwmsgq.h"
#include "lwtimer.h"
#include "psp_prv.h"


typedef struct 
	{
		uint_32	m_lFileIndex;
		uint_32	m_lDataLen;
		uint_32	m_lEndFlag;
		uint_32	m_dwCheckSum;
	} wifi_image_info;

typedef void (*HandleFun) (uint32_t,uint32_t);
typedef struct 
	{
		int	m_nID;
		HandleFun		m_pFunAddr;
	}FComFunPair;	

	#define FCOM_FUNC_TBL		{100,				&ChangePicCMD},\
                        {101,         &GetWaterCMD},\
												{102,			&EraseFileCMD}
void    app_wifi_control_task(uint32_t task_init_data);



#endif /* _APP_WIFI_H_ */

