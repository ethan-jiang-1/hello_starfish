
#ifndef __W25X32_H
#define __W25X32_H
#include "psptypes.h"
#include "io_map.h"

/*********************************************************************************************************
 ** macro definition 
*********************************************************************************************************/
#define W25X32_BLOCK_SIZE                  0x00010000		                    /* The size of block            */
#define W25X32_SECTOR_SIZE                 0x00001000                          /* The size of sector           */ 
#define W25X32_PAGE_SIZE                   256ul      		                    /* The size of page             */

/*********************************************************************************************************
** Function name     : ssp_init
** Descriptions      : Inialize ssp controller
** Input parameters  : none
** Output parameters : none
*********************************************************************************************************/
extern void SSPInit(void);
extern uint8_t flash_write_sector (uint32_t WAddr, uint8_t *buf, uint32_t WLength);


/*********************************************************************************************************
** Function name     : flash_read_id
** Descriptions      : Get flash IDcode
** Input parameters  : none
** Output parameters : Flash IDcode
*********************************************************************************************************/
extern uint16_t flash_read_id (void);

extern uint8_t flash_read_status ( void );


/*********************************************************************************************************
** Function name     : flash_read_data
** Descriptions      : Read flash memory 
** Input parameters  : RAddr    -- the start address to read
** Output parameters : buf      -- the buffer to receive the read data
**                     RLength	-- the length of the data to read
*********************************************************************************************************/
extern uint8_t flash_read_data (uint32_t RAddr, uint8_t *buf, uint32_t RLength);


/*********************************************************************************************************
** Function name     : flash_write_data
** Descriptions      : Write flash memory ,not just in one page memory 
** Input parameters  : WAddr    -- the start address to write
** Output parameters : buf      -- the buffer to write the data
**                     RLength	-- the length of the data to write
*********************************************************************************************************/							
extern uint8_t flash_write_data (uint32_t WAddr, uint8_t *buf, uint32_t WLength);


/*********************************************************************************************************
** Function name     : flash_all_erase
** Descriptions      : Erase the whole flash 
** Input parameters  : None
** Output parameters : None
*********************************************************************************************************/
extern uint8_t flash_whole_erase( void );


/*********************************************************************************************************
** Function name     : flash_sel_erases
** Descriptions      : Erase the selected flash 
** Input parameters  : startSec -- start sector	number
**                     endSec   -- end sector number
** Output parameters : None
*********************************************************************************************************/
extern uint8_t flash_sector_erase (uint32_t addr);


#endif
/*********************************************************************************************************
  End Of File
*********************************************************************************************************/
