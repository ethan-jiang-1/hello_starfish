/*******************************************************************************************************************************************************
*
*
*  Write flash for waveform and  cmdset
*  结构 4字节     4字节       4字节       4字节
*       cmdlen    cmdstart    wavlen      wavstart
*********************************************************************************************************************************************************/

#include "partition.h"
#include "app_trace.h"
#include "eink_display.h"
#include "w25x32.h"
unsigned long ImageAddr=0;

void flash_write_test()
{
	int i;
	int j;
	uint8_t buf[256];
	uint8_t rbuf[256];
	for(i = 0; i < 256; i++)
	{
		buf[i] = i;
	}
	flash_write_data(0x000000, buf, 256);
	flash_read_data(0x000000, rbuf, 256);
	for(i = 0; i < 256; i++)
	{
		if(i == rbuf[i])
		{
			continue;
		}
	}
}

/********************************************************************************************************************************************************




**********************************************************************************************************************************************************/

void write_baseinfo(void)
{
	unsigned int startaddress = FIRMWARE_OFFSET;
	unsigned int data; 
	unsigned char info[16];
	unsigned char i;
	unsigned int buf[4] = {CMDSET_LENGTH,CMDSET_START_ADDR, WAVEFORM_LENGTH, FIRMWARE_OFFSET };
	unsigned char *Pcmd = (unsigned char *)buf;
	unsigned char temp;
	
	spi_lcm_exchange(BUS_TO_SPI);
	
	for(i=0; i < 16; i++)
	{
		flash_write_data(startaddress + i, Pcmd + i, 1);
	}
	for(i=0; i < 16; i++)
	{
		flash_read_data(startaddress + i,&temp, 1);
		
		if(temp != Pcmd[i])
		{
			APP_TRACE("write_baseinfo [%08x] = %02x -> %02xerror\n",startaddress + i,  temp,Pcmd[i] );
			
		}
	}	
	spi_lcm_exchange(BUS_TO_LCM);	
	
}	


void read_baseinfo(void)
{
	unsigned long int  startaddress = FIRMWARE_OFFSET;
	unsigned char buf[16] = {0};
	spi_lcm_exchange(BUS_TO_SPI);
	flash_read_data(startaddress, buf, 16);
	spi_lcm_exchange(BUS_TO_LCM);
}

#if 1
void get_cmdset_waveform(unsigned long int *cmdlen, unsigned long int  *wavlen)
{
	unsigned long int startaddress = FIRMWARE_OFFSET;
	unsigned char buf[16] = {0};
	unsigned long int len;
	spi_lcm_exchange(BUS_TO_SPI);
	flash_read_data(startaddress, buf, 16);
	spi_lcm_exchange(BUS_TO_LCM);

	len = buf[0];
	len += buf[1] << 8;
	len += buf[2] << 16;
  len += buf[3] << 24;
	
	*cmdlen = len;
	len = buf[8];
	len += buf[9] << 8;
	len += buf[10] << 16;
  len	+= buf[11] << 24;
	
	*wavlen = len;

}
#endif

unsigned int total_len;
unsigned int error_len;
#if 0
extern unsigned char cmdset[];
void write_cmdset(void)
{
	unsigned long int i = 0;
	unsigned long int startaddress = CMDSET_START_ADDR;
	unsigned char *Pcmd = cmdset;
	unsigned char temp;
	
	total_len = 0;
	error_len = 0;
	spi_lcm_exchange(BUS_TO_SPI);
	for(i=0; i < CMDSET_LENGTH; i++)
	{
		flash_write_data(startaddress + i, Pcmd + i, 1);
	}
	
	total_len = CMDSET_LENGTH;
	
	for(i=0; i < CMDSET_LENGTH; i++)
	{
		flash_read_data(startaddress + i,&temp, 1);
		
		if(temp != Pcmd[i])
		{
			APP_TRACE("write_cmdset [%08x] = %02x -> %02x   error\n",startaddress + i,  temp,Pcmd[i] );
			error_len ++;
			
		}
	}	
	spi_lcm_exchange(BUS_TO_LCM);
	APP_TRACE("write_cmdset is OK! %d, %d \r\n", total_len, error_len);
}

#endif


void read_cmdset(void)
{
	unsigned char buf[16] = {0};
	unsigned long int startaddress = CMDSET_START_ADDR;
	spi_lcm_exchange(BUS_TO_SPI);
	flash_read_data(startaddress, buf, 16);
	spi_lcm_exchange(BUS_TO_LCM);
			
  startaddress = CMDSET_START_ADDR + CMDSET_LENGTH - 16;
  spi_lcm_exchange(BUS_TO_SPI);
	flash_read_data(startaddress, buf, 16);
  spi_lcm_exchange(BUS_TO_LCM);		
	APP_TRACE("\nbuf[0]= %02x\n", buf[0]);			
}


#if 0
extern unsigned short wfm_set[];

void write_wav(void)
{

	unsigned long int i = 0;
	unsigned long int len;
	unsigned long int startaddress = 0;
	unsigned char *Pcmd = (unsigned char *)wfm_set;
	unsigned char temp;
	
	unsigned char times;
	
	error_len = 0;
	if(waveform_p == 0xff)
		return;	
	
	startaddress = WAVFORM_START_ADDR + 200*16*(waveform_p -1);
	spi_lcm_exchange(BUS_TO_SPI);
	
	if(waveform_p < 23){
		len = 200 * 16;
	}
	else
	{
		len = WAVEFORM_LENGTH -  200*16 * (waveform_p -1);
	}
	
	total_len = len;
	
	for(i=0; i < len; i++)
	{
		times = 0;
		flash_write_data(startaddress + i, Pcmd + i, 1);
		
		
		do{
		flash_read_data(startaddress + i,&temp, 1);
		
			if(temp != Pcmd[i])
			{
				flash_write_data(startaddress + i, Pcmd + i, 1);
				APP_TRACE("error addr: %08x\n", startaddress + i);
			}

		}while(times++ < 0);
		
	}
	
	
	for(i=0; i < len; i++)
	{
		flash_read_data(startaddress + i,&temp, 1);
		
		if(temp != Pcmd[i])
		{
			//APP_TRACE("\r\nwrite_wav [%08x] = %02x -> %02xerror\r\n",startaddress + i,  temp,Pcmd[i] );
			error_len++;
			
		}
	}
	
	
	spi_lcm_exchange(BUS_TO_LCM);		

	APP_TRACE("write_wav is OK! %d, %d \r\n", total_len, error_len);
	
}

#endif
#if 0
extern unsigned char gImage_image[];

void write_image(void)
{

	unsigned long int i = 0;
	unsigned long int len;
	unsigned long int startaddress = 0;
	unsigned char *Pcmd = gImage_image;
	unsigned char temp;
	
	unsigned char times;
	len = 0;
	error_len = 0;
	
	if(image_p == 0xff)
		return;	
	
	startaddress = IMAGE_START_ADD + 300*16*(image_p -1);
	spi_lcm_exchange(BUS_TO_SPI);
	
	if(image_p< 2)
		{
		  len = 300 * 16;
	  }
	else
	{
		len = IMAGE_LENGTH   -  300*16 * (image_p -1);
	}
	
	total_len = len;
	
	for(i=0; i < len; i++)
	{
		times = 0;
		flash_write_data(startaddress + i, Pcmd + i, 1);
		
		
		do{
		flash_read_data(startaddress + i,&temp, 1);
		
			if(temp != Pcmd[i])
			{
				flash_write_data(startaddress + i, Pcmd + i, 1);
				APP_TRACE("error addr: %08x\n", startaddress + i);
			}

		}while(times++ < 0);
		
	}
	
	
	for(i=0; i < len; i++)
	{
		flash_read_data(startaddress + i,&temp, 1);
		
		if(temp != Pcmd[i])
		{
			APP_TRACE("\r\nwrite_wav [%08x] = %02x -> %02xerror\r\n",startaddress + i,  temp,Pcmd[i] );
			error_len++;
			
		}
	}
	
	
	spi_lcm_exchange(BUS_TO_LCM);		

	APP_TRACE("write_image %d is OK! %d, %d \r\n",image_p, total_len, error_len);
	
}
#endif

void read_wav(void)
{  
	  unsigned char buf[16] = {0};
	  unsigned long int startaddress = WAVFORM_START_ADDR;
		
		spi_lcm_exchange(BUS_TO_SPI);
		flash_read_data(startaddress, buf, 16);
		spi_lcm_exchange(BUS_TO_LCM);
	  APP_TRACE("\nbuf[0]= %02x\n", buf[0]);
}

void read_spi_falsh(unsigned long  int address, unsigned char *buf, unsigned long int size)
{
		spi_lcm_exchange(BUS_TO_SPI);
		flash_read_data(address, buf, size);
		spi_lcm_exchange(BUS_TO_LCM);	
	
	 //APP_TRACE("read_spi_flash is ok!  \r\n");
}

/*
输入参数 buflen: 假设图像total 1000 byte, buflen=900：表示已经读取了100，还剩余900没有读取.
返回值: 表示本次从flash中读取的长度存放到全局变量exchange_buff[]中.
*/
#if 1
int eink_getdata(int buflen)
{
  unsigned long int i,len,dumplen=0;
	unsigned long int startaddress = ImageAddr; //图像在flash中的起始地址.
	unsigned char buf[DATA_BUFFER_LEN]={0};
	unsigned char *pdata = buf;
                len = DATA_BUFFER_LEN;
	dumplen = IMAGE_LENGTH -buflen;
	if(buflen > len)
	{
	 flash_read_data(startaddress+dumplen,buf,len);
		for(i=0;i<len;i++)
		{
		 exchange_buff[i]=buf[i];
		
		}
  return len;	
	}else{
	
	 flash_read_data(startaddress+dumplen,buf,len);
		for(i=0;i<buflen;i++)
		{
		 exchange_buff[i]=buf[i];
		}
		return buflen;
	}
	
}
#endif