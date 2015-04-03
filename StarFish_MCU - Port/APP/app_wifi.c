

#include "app_wifi.h"


#include "app_trace.h"
#include <string.h>
#include <stdlib.h>
#include "w25x32.h"
#include "eink_display.h"
#include "partition.h"

#include "app_rgb_led.h"


#define WIFI_MSG_NUMBER        64
#define WIFI_MSG_SIZE          1
static uint_32  g_uart0_rx_queue[sizeof(LWMSGQ_STRUCT)/sizeof(uint_32) + WIFI_MSG_NUMBER* WIFI_MSG_SIZE];

#define WIFI_RECV_BUF_SIZE     1124  /*This is maxium buffer size: 1024=binary data size, 100=maxium header size*/
static uint_8  g_wifi_com_recv_buf[WIFI_RECV_BUF_SIZE];

uint8_t wifi_task_stack[WIFI_TASK_STACK_SIZE];
//////////////////////WIFI CMD
#define WIFI_TAG_HEADER				"%B1Q,"
#define WIFI_IMAGE_TAG_HEADER							"%B1Q,300,"	
#define WIFI_IMAGE_TAG_LEN								(sizeof(WIFI_IMAGE_TAG_HEADER) - 1)
#define WIFI_FLASHDATA_HEADER							"%B1Q,800,"	
#define WIFI_FLASHDATA_LEN								(sizeof(WIFI_FLASHDATA_HEADER) - 1)
#define WIFI_IMAGE_CMD_HEADER_LEN						(sizeof("%B1Q,300,1234,1000,1,1234567890")) //32 byte.

uint8_t gImgBStart=0;
uint32_t gImgAddr=0;
extern unsigned long ImageAddr;
static void uart0_irq_handler(void* p_arg);

/*v1表示第几张图，v2空着，没有使用.*/
void ChangePicCMD(uint32_t v1,uint32_t v2 )
{
	
	APP_TRACE("test1\r\n");
	ImageAddr=IMAGE_START_ADD*v1*IMAGE_SPACE_IN_FLASH;
	 { 
		  struct display_rect rect;
		
		  rect.x = 0;
		  rect.y = 0;
		  rect.w=  360;
	  	rect.h = 600;
		  eink_display(&rect, 0, eink_getdata); 
}

}
/*v1,v2没有使用*/
void GetWaterCMD(uint32_t v1,uint32_t v2)
{
	APP_TRACE("test2\r\n");
}
/*v1表示第几张图，v2空着，没有使用.*/
void EraseFileCMD(uint32_t v1,uint32_t v2)
{
	APP_TRACE("EraseFileCMD\r\n");
	EraseFile(v1);
}
void DataFlashEraseSector(uint32_t v1,uint32_t v2)
{
	APP_TRACE("DataFlashEraseSector \r\n");
	flash_sector_erase(v1);
}
void DataFlashEraseChip(uint32_t v1,uint32_t v2)
{
	printf("DataFlashEraseChip \r\n");
	flash_whole_erase();
}
#if 0
bool verifyCheckSum( dword dwTargetCheckSum, uint_8 *pData, dword nDataLen )
{

		dword dwCheckSum = 0;
	while (nDataLen--)
	{
		dwCheckSum+=*pData++;
	}
	return dwCheckSum == dwTargetCheckSum;
}

static int ParseChecksum(uint_8 * pData, int dwLen)
	{

		int dwMulti = 1;
		int dwCheckSum = 0;
		int i=0;
		for(i=dwLen; i>0; --i)
		{
			dwCheckSum += ((*(pData + i - 1) - '0') * dwMulti);
			dwMulti *= 10;
		}

		return dwCheckSum;
	}
	#endif
	/*--------------------------------------------------------------
  * Data flash programe command
	* "%B1Q,800,start add, Date Length,data(HEX),cc\r\n		
  *--------------------------------------------------------------
  */
static int  do_DataFlash_Prog_cmd(uint_8* pData,int8_t length)
{
	//"%B1Q,800,1234,16,1,000102030405(HEX),"
	
	char szAddress[10];
	char szLength[10];
	uint32_t iAddress;
	uint32_t iBufferLength;
	int i =0;	
	int8_t iCommaPos[]={-1,-1,-1,-1,-1,-1,-1,-1};
	int8_t iLastCommaPos=-1;
	uint_8 *pDataTmp=NULL;
	uint_8 *pTemp=NULL;
	uint_8 iSolvedLen=0;
	APP_TRACE("do_DataFlash_Prog_cmd %d\r\n",length);
	memset(szAddress,0,sizeof(szAddress));
	memset(szLength,0,sizeof(szLength));
  // now search ','
//	pData=memchr(pData,'%',length);
	if(NULL==pData)
	{
		return;
	}
	pDataTmp=pData;
	while(length-iSolvedLen>1)
	{
		pTemp=memchr(pDataTmp,',',length-iSolvedLen);
		if(NULL!=pTemp)
		{
			iCommaPos[i]=pTemp-pData;
			
			pDataTmp=pTemp+1;
			iSolvedLen=pTemp-pData+1;
			APP_TRACE("Comma: %d\r\n",iCommaPos[i]);
			i++;
			if(i>5) break;
			
		}
		else
		{
			break;
		}
	}


	memcpy(szAddress, pData + iCommaPos[1]+1, iCommaPos[2]-iCommaPos[1]-1);
  memcpy(szLength, pData + iCommaPos[2]+1, iCommaPos[3]-iCommaPos[2]-1);
	APP_TRACE("szAddress: %s\r\n",szAddress);
	APP_TRACE("szLength: %s\r\n",szLength);

	iAddress=atoi(szAddress);
	iBufferLength=atoi(szLength);
	
	APP_TRACE("szAddress: %d\r\n",iAddress);
	APP_TRACE("szLength: %d\r\n",iBufferLength);
	
	if(length<iBufferLength+iCommaPos[3]+1)
	{
		return -1;
	}
	//Program Flash
	if(iBufferLength%2==0)
	{
	flash_write_word (iAddress,pData+iCommaPos[3]+1 , iBufferLength );
	}
	else
	{
		flash_write_word (iAddress,pData+iCommaPos[3]+1 , iBufferLength-1 );
		flash_write_sector (iAddress+iBufferLength-1, pData+iBufferLength-1, 1);
	}
	//Read for check
	pTemp=malloc(iBufferLength);
	flash_read_data (iAddress,pTemp, iBufferLength);
	for(i=0;i<iBufferLength;i++)
	{
			APP_TRACE("%02x ",pTemp[i]);
		if(i%16==0)
		{
			APP_TRACE("\r\n");
		}
	}
	
	

	return 1;
	
}

/**
*--------------------------------------------------------------
  * WIFI picture cmd:
	* %B1Q,ID,file index,length, terminate flag,checksum,binary data.
	* file index: for exapmle, total 9999 image can be stored in the flash. 
	* length means how many byte of binary data
	*	terminate flag means it is the last frame image or not.
	* %B1Q,IMAGE_ID(300),9999,1000,1,1234567890,0015609326......  
  *--------------------------------------------------------------
  */
static int  do_wifi_image_cmd(uint_8* pData)
{
//"%B1Q,300,1234,1000,1,0123456789,"
	wifi_image_info info;
	uint8_t i=0;
	char szBuf[64] = {0}; //reply message
	char szFileIndex[5] = {0};
	char szDataLen[5] = {0};
	char szEndFlag[2] = {0};
	char szCheckSum[11] = {0};
	memcpy(szFileIndex, pData + 9, 4);
	memcpy(szDataLen, pData + 14, 4);
	memcpy(szEndFlag, pData + 19, 1);
	memcpy(szCheckSum, pData + 21, 10);
	info.m_lFileIndex=atoi(szFileIndex);
	info.m_lDataLen = atoi(szDataLen);
	info.m_lEndFlag = atoi(szEndFlag);
/*if it is 1.img, get the start address of 1.img*/
	if(gImgBStart==0)
	{
		gImgAddr=GetFileStartAddr(info.m_lFileIndex);
		gImgBStart=1;
	}
	
/*disable check sum calculation*/
	#if 0
	info.m_dwCheckSum =ParseChecksum((uint_8*)(szCheckSum), strlen(szCheckSum));
	if(verifyCheckSum(info.m_dwCheckSum, (uint_8 *)(pData+WIFI_IMAGE_CMD_HEADER_LEN), 
			info.m_lDataLen))
#endif
	{
		
		/*start to record spi*/
		for(i=0;i<info.m_lDataLen;i++)
		{
			flash_write_data (gImgAddr+i,pData + 32+i , 1 );
		}
		gImgAddr=gImgAddr+info.m_lDataLen;
		/*end record flash*/
		snprintf(szBuf, sizeof(szBuf)-1, "%B1P,300,1\r\n");
    uart0_send_string((uint8_t*)szBuf);
		if(info.m_lEndFlag==1)
		{
			gImgBStart=0;
			gImgAddr=0;
			/*send all data stored in flash to eink*/
			
		}
		
	}

	return 1;
	
}


int HandleMessage(char* pData)
{
	 char* delims = ",";
    int id,nSize,i= 0;
		int para[5]={0};
		char *p_result;
		char *buf= pData;
		FComFunPair funTbl [] = {FCOM_FUNC_TBL};
    APP_TRACE("%s\r\n",(const char*)buf);
		while((p_result=strtok(buf,(char*)delims))!=NULL) {
			
                para[i]=atoi(p_result);
								i++;
                 buf=NULL;
				}
				APP_TRACE("s1:%d,s2:%d,s3:%d\r\n",para[0],para[1],para[2]);

	id=para[0];
 nSize = sizeof(funTbl) / sizeof(funTbl[0]);			
	for(i=0; i<nSize; ++i)
	{
		
		if(funTbl[i].m_nID==id)
		{
			(*funTbl[i].m_pFunAddr)(para[1],para[2]);
		}
			
	}		

	return 1;
}
bool IsImage()
{
	if(0 == memcmp(g_wifi_com_recv_buf, WIFI_IMAGE_TAG_HEADER, WIFI_IMAGE_TAG_LEN))
		return TRUE;
	else 
		return FALSE;
}
bool IsFlashData()
{
	if(0 == memcmp(g_wifi_com_recv_buf, WIFI_FLASHDATA_HEADER, WIFI_FLASHDATA_LEN))
		return TRUE;
	else 
		return FALSE;
}
/**
  * @brief: deal with the recviced data
  * @param: length, reception character length
  * @retval: none
  * @note:
  *
  */
void    do_cmd_wifidata(uint8_t length)
{
    char*               p;
    int                 ret;
    int                 state;

    // first, in order to deal with the string
    // we need add '0' at the end of string
	APP_TRACE("----------do_cmd_wifidata %d\r\n",length);
    g_wifi_com_recv_buf[length] = 0;

    APP_TRACE((const char*)g_wifi_com_recv_buf);
    /*
		* Image has higher priority than the others message, so handle it firstly.
		*/
		 if(IsImage())
		 {
			 do_wifi_image_cmd(g_wifi_com_recv_buf);
			 return;
		 }
		 else if(IsFlashData)
		 {
			 do_DataFlash_Prog_cmd(g_wifi_com_recv_buf,length);
			 return;
		 }
		 /*if it is not image related message(photo image and firmware image),
		 * and then this message must belong to normal case:
		 * %B1Q,ID, Parameter1, parmater2,par3.
		 * Is it necessary to use terminate flag?
		 */
		 p = strstr((const char*)g_wifi_com_recv_buf, WIFI_TAG_HEADER);
		 if(p)
		 {
			  p += strlen(WIFI_TAG_HEADER);
			 ret=HandleMessage(p);
		 }
		

}






void    app_wifi_control_task(uint32_t task_init_data)
{
    _mqx_uint           ret;
    _mqx_max_type       msg = 0;
    uint8_t             pos = 0;

    // create recv msg queue
    ret = _lwmsgq_init(g_uart0_rx_queue, WIFI_MSG_NUMBER, WIFI_MSG_SIZE);
    ASSERT_PARAM(MQX_OK == ret);
		 // delay 5 seconds to ingore boot up message from wifi.
    _time_delay_ticks(200*5);

    // init UART0 to communication with wifi
    init_uart0(uart0_irq_handler);


    for (;;)
    {
        /* wait 5*5=25ms
				Adopt the easiest and stupid way to receive one complete message based on time out mechansim instead of finding "terminating" symbol.
			*/
        ret = _lwmsgq_receive(g_uart0_rx_queue, &msg, LWMSGQ_RECEIVE_BLOCK_ON_EMPTY, 5, NULL);
        switch (ret)
        {
            case MQX_OK:
                if (pos < WIFI_RECV_BUF_SIZE - 1)
                {// save data
                    g_wifi_com_recv_buf[pos++] = (uint8_t)msg;
                }
                else
                {
                    APP_TRACE("error: buffer full\r\n");
                    // data error, clear the buffer
                    pos = 0;
                }
                break;

            case LWMSGQ_TIMEOUT:
                if (pos)
                {
                    do_cmd_wifidata(pos);
                    // clear buffer, to reload next data reception
                    pos = 0;
                }
                break;

            default:
                APP_TRACE("g_uart0_rx_queue recv error: 0x%x\r\n", ret);
                break;

        }
    }
}



static void uart0_irq_handler(void* p_arg)
{
    _mqx_uint       ret;
    _mqx_max_type   msg;

    if (UART0_PDD_ReadInterruptStatusReg(UART0_BASE_PTR) & UART0_S1_RDRF_MASK)
    {
        msg = UART0_PDD_GetChar8(UART0_BASE_PTR);

        ret = _lwmsgq_send(g_uart0_rx_queue, &msg, 0);
        ASSERT_PARAM(MQX_OK == ret);
    }
}






