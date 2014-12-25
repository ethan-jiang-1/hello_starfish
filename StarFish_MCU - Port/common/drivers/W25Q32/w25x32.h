
#ifndef __W25X32_H
#define __W25X32_H
#include "psptypes.h"
#include "io_map.h"

/*********************************************************************************************************
 ** macro definition 
*********************************************************************************************************/
#define W25X32_BLOCK_SIZE                  0x00010000		                    /* The size of block            */
#define W25X32_SECTOR_SIZE                 0x00001000                          /* The size of sector           */ 
#define W25X32_PAGE_SIZE                   256ul      		                    /* The size of page   
*/


#define BLOCK_PROTECTION_LOCK_DOWN_NULL				 0x00 
#define BLOCK_PROTECTION_LOCK_DOWN_UP_1_OF_64  0x01
#define BLOCK_PROTECTION_LOCK_DOWN_UP_1_OF_32  0x02
#define BLOCK_PROTECTION_LOCK_DOWN_UP_1_OF_16  0x03
#define BLOCK_PROTECTION_LOCK_DOWN_UP_1_OF_8   0x04
#define BLOCK_PROTECTION_LOCK_DOWN_UP_1_OF_4   0x05
#define BLOCK_PROTECTION_LOCK_DOWN_UP_1_OF_2   0x06
#define BLOCK_PROTECTION_LOCK_DOWN_ALL          0x07
/*********************************************************************************************************
** Function name     : ssp_init
** Descriptions      : Inialize ssp controller
** Input parameters  : none
** Output parameters : none
*********************************************************************************************************/
void SSPInit(void);
uint8_t flash_write_sector (uint32_t WAddr, uint8_t *buf, uint32_t WLength);


/*********************************************************************************************************
** Function name     : flash_read_id
** Descriptions      : Get flash IDcode
** Input parameters  : none
** Output parameters : Flash IDcode
*********************************************************************************************************/
uint16_t flash_read_id (void);

uint8_t flash_read_status ( void );


/*********************************************************************************************************
** Function name     : flash_read_data
** Descriptions      : Read flash memory 
** Input parameters  : RAddr    -- the start address to read
** Output parameters : buf      -- the buffer to receive the read data
**                     RLength	-- the length of the data to read
*********************************************************************************************************/
uint8_t flash_read_data (uint32_t RAddr, uint8_t *buf, uint32_t RLength);


/*********************************************************************************************************
** Function name     : flash_write_data
** Descriptions      : Write flash memory ,not just in one page memory 
** Input parameters  : WAddr    -- the start address to write
** Output parameters : buf      -- the buffer to write the data
**                     RLength	-- the length of the data to write
*********************************************************************************************************/							
uint8_t flash_write_data (uint32_t WAddr, uint8_t *buf, uint32_t WLength);


/*********************************************************************************************************
** Function name     : flash_all_erase
** Descriptions      : Erase the whole flash 
** Input parameters  : None
** Output parameters : None
*********************************************************************************************************/
uint8_t flash_whole_erase( void );


/*********************************************************************************************************
** Function name     : flash_sel_erases
** Descriptions      : Erase the selected flash 
** Input parameters  : startSec -- start sector	number
**                     endSec   -- end sector number
** Output parameters : None
*********************************************************************************************************/
uint8_t flash_sector_erase (uint32_t addr);

uint8_t flash_block_erase (uint32_t addr);
#endif
/*********************************************************************************************************
  End Of File
*********************************************************************************************************/
