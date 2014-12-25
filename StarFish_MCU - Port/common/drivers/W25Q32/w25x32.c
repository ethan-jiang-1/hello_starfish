
#include "w25x32.h"


/* 宏,用于定义w25x32的片选引脚 */
#define SPI_FLASH_CS_LOW()       FGPIOE_PCOR |= 1 << 16;                            /* 配置CS1置低 */
#define SPI_FLASH_CS_HIGH()      FGPIOE_PSOR |= 1 << 16;  



#define SPI_FLASH_CMD_SST_AAI_WORD_PROGRAM 0xAD

/*********************************************************************************************************
** Function name     : ssp_init
** Descriptions      : Inialize ssp controller
** Input parameters  : none
** Output parameters : none
** Returned value    : none
*********************************************************************************************************/

/*********************************************************************************************************
** Function name     : W25X10_ReadStatus
** Descriptions      : Read the state register in the flash memory
** Input parameters  : none
** Output parameters : The value of the state register
** Returned value    : none
*********************************************************************************************************/
uint8_t flash_read_status ( void )
{
	uint8_t status;

	SPI_FLASH_CS_LOW();				 								    /* 选中SPI Flash                */
	Send_Byte(0x05);					        
	status = Send_Byte(0xff);				        
	SPI_FLASH_CS_HIGH();	    									    /* P0.2--1,CS = 1 释放SPI Flash */
	return status;											    /* Return Reg 1's Content		*/
}

/*********************************************************************************************************
** Function name     : flash_write_enable
** Descriptions      : Enable the flash memory to write
** Input parameters  : none
** Output parameters : none
** Returned value    : none
*********************************************************************************************************/
static void flash_write_enable (void)
{
	while (flash_read_status() & 0x01 != 0x00);                         /* Wait for flash free          */

	SPI_FLASH_CS_LOW();				 									    /* P0.2--0,CS = 0 选中SPI Flash */

	Send_Byte(0x06);

	SPI_FLASH_CS_HIGH();													    /* P0.2--1,CS = 1 释放SPI Flash */

	while (flash_read_status() & 0x03 != 0x02);                         /* Wait for operation complete  */
}

/*********************************************************************************************************
** Function name     : flash_read_id
** Descriptions      : Get flash IDcode
** Input parameters  : none
** Output parameters : Flash IDcode
** Returned value    : Flash IDcode
*********************************************************************************************************/
uint_16 flash_read_id (void)
{
	uint_16 IDcode=0;

	SPI_FLASH_CS_LOW();				 									    /* P0.2--0,CS = 0 选中SPI Flash */
	Send_Byte(0x90);
	Send_Byte(0x00);
	Send_Byte(0x00);
	Send_Byte(0x00);
 // IDcode = (Send_Byte(0xff) << 8) | Send_Byte(0xff);
	IDcode = (Send_Byte(0x55) << 8) | Send_Byte(0x55);
	SPI_FLASH_CS_HIGH();													    /* P0.2--1,CS = 1 释放SPI Flash */

	return IDcode; 												   
}

/*********************************************************************************************************
** Terry: The function must be called once before progremme the SPI Flash
**
** Function name     : flash_write_status 
** Descriptions      : Write the write state register in the flash memory
** Input parameters  : none
** Output parameters : The value of the write state register
** Returned value    : none
*********************************************************************************************************/
void flash_write_status ( uint_16 Status )
{
	flash_write_enable();
	Status|=0x01<<1; //Set SEL to 1
	SPI_FLASH_CS_LOW();				 									    /* P0.2--0,CS = 0 选中SPI Flash */
	Send_Byte(0x01);
	Send_Byte((Status & 0x0000FF));
	Send_Byte((Status & 0x00FF00) >> 8);
	SPI_FLASH_CS_HIGH();													    /* P0.2--1,CS = 1 释放SPI Flash */

	return;			
}

/*********************************************************************************************************
** Function name     : flash_read_data
** Descriptions      : Read flash memory 
** Input parameters  : RAddr    -- the start address to read
** Output parameters : buf      -- the buffer to receive the read data
**                     RLength	-- the length of the data to read
** Returned value    : The operation result. 1 -- sucess, 0 -- false
*********************************************************************************************************/
uint8_t flash_read_data (uint32_t RAddr, uint8_t *buf, uint32_t RLength)
{
	uint8_t Temp;
	uint32_t i;

	if (RLength == 0)
	{
		return 0;
	}

	/*
	 *	Check the state register. If it's busy , wait until it's free
	 */
	while(1)														
	{														
		Temp = flash_read_status( );								
		Temp &= 0x01;											
		if(Temp == 0x00)									
			break;									
		for(i=0; i<10; i++);						
	}

	SPI_FLASH_CS_LOW();				 									    /* P0.2--0,CS = 0 选中SPI Flash */
	Send_Byte(0x03);
	Send_Byte((RAddr & 0xFF0000) >> 16);
	Send_Byte((RAddr & 0x00FF00) >> 8);
	Send_Byte((RAddr & 0x0000FF));
	for (i=0; i<RLength; i++)
	{
		buf[i] = Send_Byte(0xff);
	}
	SPI_FLASH_CS_HIGH();													    /* P0.2--1,CS = 1 释放SPI Flash */

	return 1;
}


/*********************************************************************************************************
** Function name     : flash_write_sector
** Descriptions      : Write flash memory , just in one page memory 
** Input parameters  : WAddr    -- the start address to write
** Output parameters : buf      -- the buffer to write the data
**                     RLength	-- the length of the data to write
** Returned value    : The operation result. 1 -- sucess, 0 -- false
*********************************************************************************************************/
uint8_t flash_write_sector (uint32_t WAddr, uint8_t *buf, uint32_t WLength)
{
	uint32_t i;

	if (WLength == 0)
	{
		return 0;
	}

	flash_write_enable();												    /* Write enable                 */


	SPI_FLASH_CS_LOW();				 									    /* P0.2--0,CS = 0 选中SPI Flash */
	Send_Byte(0x02);
	Send_Byte((WAddr & 0xFF0000) >> 16);
	Send_Byte((WAddr & 0x00FF00) >> 8);
	Send_Byte((WAddr & 0x0000FF));
	for (i=0; i<WLength; i++)
	{
		Send_Byte(buf[i]);	
	}
	SPI_FLASH_CS_HIGH();													    /* P0.2--1,CS = 1 释放SPI Flash */

	while (flash_read_status() & 0x01 != 0x00);

	return 1;
}

/*********************************************************************************************************
** Function name     : flash_write_data
** Descriptions      : Write flash memory ,not just in one page memory 
** Input parameters  : WAddr    -- the start address to write
** Output parameters : buf      -- the buffer to write the data
**                     RLength	-- the length of the data to write
** Returned value    : The operation result. 1 -- sucess, 0 -- false
*********************************************************************************************************/							
uint8_t flash_write_data (uint32_t WAddr, uint8_t *buf, uint32_t WLength)
{
	uint32_t dealer, remainder;
	uint32_t i, addr, len = 0;

	if (WLength == 0)
	{
		return 0;
	}

	remainder = WAddr % W25X32_PAGE_SIZE;

	/*
	 * Write the data not enough to one page memory
	 */
	if (remainder != 0)
	{
		len = W25X32_PAGE_SIZE - remainder;
		if (len < WLength)
		{
			flash_write_sector(WAddr, buf, len);
		} else
		{
			flash_write_sector(WAddr, buf, WLength);
			return 1;
		}
	}
	
	/*
	 * Calculate the rest data, then write several packets with whole page memory
	 */
	remainder = (WLength - len) % W25X32_PAGE_SIZE;
	dealer    = (WLength - len) / W25X32_PAGE_SIZE;
	for (i=0; i<dealer; i++)
	{
		addr = len + (i * W25X32_PAGE_SIZE);
		flash_write_sector(WAddr+addr, (uint8_t *)&buf[addr], W25X32_PAGE_SIZE);
	}
	
	/*
	 * Write the last data that not enough to one page memory
	 */
	if (remainder != 0)
	{
		addr = len + (i * W25X32_PAGE_SIZE);
		flash_write_sector(WAddr+addr, (uint8_t *)&buf[addr], remainder);
	}												
	
	return 1;
}


/*********************************************************************************************************
** Function name     : flash_all_erase
** Descriptions      : Erase the whole flash 
** Input parameters  : None
** Output parameters : None
** Returned value    : The operation result. 1 -- sucess, 0 -- false
*********************************************************************************************************/
uint8_t flash_whole_erase( void )
{
	flash_write_enable();												    /* Write enable                 */
	
	SPI_FLASH_CS_LOW();				 									    /* P0.2--0,CS = 0 选中SPI Flash */
 	Send_Byte(0xC7);
	SPI_FLASH_CS_HIGH();													    /* P0.2--1,CS = 1 释放SPI Flash */

	while (flash_read_status() & 0x01 != 0x00);	                        /* Wait for the flash free      */
    
	return 1;
}

/*********************************************************************************************************
** Function name     : flash_block_erase
** Descriptions      : Block erase 
** Input parameters  : addr -- block address
** Output parameters : None
** Returned value    : The operation result. 1 -- sucess, 0 -- false
*********************************************************************************************************/
uint8_t flash_block_erase (uint32_t addr)
{
	flash_write_enable();												    /* Write enable                 */
	
	SPI_FLASH_CS_LOW();				 									    /* P0.2--0,CS = 0 选中SPI Flash */
	Send_Byte(0xD8);
	Send_Byte((addr & 0xFF0000) >> 16);
	Send_Byte((addr & 0x00FF00) >> 8);
	Send_Byte(addr & 0x0000FF);
 	SPI_FLASH_CS_HIGH();													    /* P0.2--1,CS = 1 释放SPI Flash */

	while (flash_read_status() & 0x01 != 0x00);					        /* Wait for the flash free      */
    
	return 1;
}

/*********************************************************************************************************
** Function name     : flash_sector_erase
** Descriptions      : Sector erase 
** Input parameters  : addr -- sector address
** Output parameters : None
** Returned value    : The operation result. 1 -- sucess, 0 -- false
*********************************************************************************************************/
uint8_t flash_sector_erase (uint32_t addr)
{
	flash_write_enable();												    /* Write enable                 */
	
	SPI_FLASH_CS_LOW();				 									    /* P0.2--0,CS = 0 选中SPI Flash */
	Send_Byte(0x20);
	Send_Byte((addr & 0xFF0000) >> 16);
	Send_Byte((addr & 0x00FF00) >> 8);
	Send_Byte(addr & 0x0000FF);
 	SPI_FLASH_CS_HIGH();													    /* P0.2--1,CS = 1 释放SPI Flash */

	while (flash_read_status() & 0x01 != 0x00);							/* Wait for the flash free      */
    
	return 1;
}


/*********************************************************************************************************
** Function name     : flash_sel_erases
** Descriptions      : Erase the selected flash 
** Input parameters  : startSec -- start sector	number
**                     endSec   -- end sector number
** Output parameters : None
** Returned value    : The operation result. 1 -- sucess, 0 -- false
*********************************************************************************************************/
uint8_t flash_sel_erases (uint32_t startSec, uint32_t endSec)
{
	uint32_t i;

	for (i=startSec; i<=endSec; i++)
	{
		flash_sector_erase(i * W25X32_SECTOR_SIZE);
	}

	return(1);
}



//////////////////
void EWSR()
{
	SPI_FLASH_CS_LOW();			/* enable device */
	Send_Byte(0x50);		/* enable writing to the status register */
	SPI_FLASH_CS_HIGH();			/* disable device */
}



/************************************************************************/
/* PROCEDURE: WREN							*/
/*									*/
/* This procedure enables the Write Enable Latch.  It can also be used 	*/
/* to Enables Write Status Register.					*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void WREN()
{
	SPI_FLASH_CS_LOW();			/* enable device */
	Send_Byte(0x06);		/* send WREN command */
	SPI_FLASH_CS_HIGH();			/* disable device */
}

/************************************************************************/
/* PROCEDURE: WRDI							*/
/*									*/
/* This procedure disables the Write Enable Latch.			*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void WRDI()
{
	SPI_FLASH_CS_LOW();			/* enable device */
	Send_Byte(0x04);		/* send WRDI command */
	SPI_FLASH_CS_HIGH();			/* disable device */
}

/************************************************************************/
/* PROCEDURE: EBSY							*/
/*									*/
/* This procedure enable SO to output RY/BY# status during AAI 		*/
/* programming.								*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void EBSY()
{
	SPI_FLASH_CS_LOW();			/* enable device */
	Send_Byte(0x70);		/* send EBSY command */
	SPI_FLASH_CS_HIGH();			/* disable device */
}

/************************************************************************/
/* PROCEDURE: DBSY							*/
/*									*/
/* This procedure disable SO as output RY/BY# status signal during AAI	*/
/* programming.								*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void DBSY()
{
	SPI_FLASH_CS_LOW();			/* enable device */
	Send_Byte(0x80);		/* send DBSY command */
	SPI_FLASH_CS_HIGH();			/* disable device */
}


/*********************************************************************************************************
** Function name     : flash_write_word
** Descriptions      : Write flash memory , just in one page memory 
** Input parameters  : WAddr    -- the start address to write
** Output parameters : buf      -- the buffer to write the data
**                     RLength	-- the length of the data to write
** Returned value    : The operation result. 1 -- sucess, 0 -- false
*********************************************************************************************************/
#ifdef HARDWARE_END_OF_WRITE_DECECTION
void CheckBusy()
{
	uint32_t PortE19Reg=PORTE_PCR19;
	//Terry Here should delay some time to wait SDO come to Low and than check it come to high
	int i;

	for( i=0;i<100;i++);
  PORTE_PCR19 =  PORT_PCR_PS_MASK |
                    PORT_PCR_PE_MASK | 
                    PORT_PCR_PFE_MASK |
                    PORT_PCR_MUX(1);  
	FGPIOE_PDDR &=~( 1<<19);

	while(FGPIOE_PDIR&(0x01<<19)==(0x01<<19));

	PORTE_PCR19=PortE19Reg;
	
}
/*********************************************************************************************************
** Function name     : flash_write_word((Hardware end of write detection)
** Descriptions      : Write flash memory , just in one page memory 
** Input parameters  : WAddr    -- the start address to write
** Output parameters : buf      -- the buffer to write the data
**                     WLength	-- the length of the data to write,WLength must be an even number.
** Returned value    : The operation result. 1 -- sucess, 0 -- false
*********************************************************************************************************/


uint8_t flash_write_word (uint32_t WAddr, uint16_t *buf, uint32_t WLength)
{
	uint32_t i;

	if (WLength == 0)
	{
		return 0;
	}
	if(WLength%2!=0)
	{
		return 0;
	}
	WLength/=2;
	
	EBSY();			
	WREN();
	SPI_FLASH_CS_LOW();			
	Send_Byte(SPI_FLASH_CMD_SST_AAI_WORD_PROGRAM);			/* send AAI command */
	Send_Byte(((WAddr & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
	Send_Byte(((WAddr & 0xFFFF) >> 8));
	Send_Byte(WAddr & 0xFF);
	for (i=0; i<WLength; i++)
	{
		APP_TRACE("%d ",i);
		Send_Byte(buf[i] & 0xFF);
		Send_Byte(((buf[i] & 0xFFFF) >> 8));
		SPI_FLASH_CS_HIGH();
		CheckBusy();
		if(i<WLength-1)
			{
				SPI_FLASH_CS_LOW();	
				Send_Byte(SPI_FLASH_CMD_SST_AAI_WORD_PROGRAM);			/* send AAI command */
			}
			
				
	}
	WRDI();
	DBSY();
	while (flash_read_status() & 0x01 != 0x00);	
	return 1;

}
#endif

/*********************************************************************************************************
** Function name     : flash_write_word (Software end of write detection)
** Descriptions      : Write flash memory , just in one page memory 
** Input parameters  : WAddr    -- the start address to write
** Output parameters : buf      -- the buffer to write the data
**                     WLength	-- the length of the data to write,WLength must be an even number.
** Returned value    : The operation result. 1 -- sucess, 0 -- false
*********************************************************************************************************/

uint8_t flash_write_word (uint32_t WAddr, uint16_t *buf, uint32_t WLength)
{
	uint32_t i;


	if (WLength == 0)
	{
		return 0;
	}
	if(WLength%2!=0)
	{
		return 0;
	}
	WLength/=2;
	WREN();
	SPI_FLASH_CS_LOW();			
	Send_Byte(SPI_FLASH_CMD_SST_AAI_WORD_PROGRAM);			/* send AAI command */
	Send_Byte(((WAddr & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
	Send_Byte(((WAddr & 0xFFFF) >> 8));
	Send_Byte(WAddr & 0xFF);
	for (i=0; i<WLength; i++)
	{
	//	APP_TRACE("%d ",i);
		Send_Byte(buf[i] & 0xFF);
		Send_Byte(((buf[i] & 0xFFFF) >> 8));
		SPI_FLASH_CS_HIGH();
		while (flash_read_status() & 0x01 != 0x00);	
		if(i<WLength-1)
			{
				SPI_FLASH_CS_LOW();	
				Send_Byte(SPI_FLASH_CMD_SST_AAI_WORD_PROGRAM);			/* send AAI command */
			}
			
				
	}
	WRDI();
	while (flash_read_status() & 0x01 != 0x00);	
	return 1;

}
/*********************************************************************************************************
  End Of File
*********************************************************************************************************/













