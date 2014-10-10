
#include "MKL_spi.h"


/*********************************************************************************************************
** Function name:           spiInit
** Descriptions:            spi初始化
** input parameters:        SPI输入基地址 输入主从模式
** output parameters:       none
** Returned value:          none
** Created by:              
** Created date
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
*********************************************************************************************************/
void spiInit (SPI_MemMapPtr SPI,uint_8 bMode)
{
	
	//port io initlization to this place
	
	/*
     *  SPI的管脚初始化
     */
    PORTB_PCR0  |= PORT_PCR_MUX(1);                                     /* 开启PB0管脚GPIO功能 SPI0 CS1 */
    PORTC_PCR5  |= PORT_PCR_MUX(2)|(uint32_t)~(uint32_t)PORT_PCR_MUX(5);                                   /* 开启PC5管脚SPI0功能 SPI0 SCK */
    PORTC_PCR6  |= PORT_PCR_MUX(2)|(uint32_t)~(uint32_t)PORT_PCR_MUX(5);                                     /* 开启PC6管脚SPI0功能 SPI0 MOSI*/
    PORTC_PCR7  |= PORT_PCR_MUX(2)|(uint32_t)~(uint32_t)PORT_PCR_MUX(5);                                     /* 开启PC7管脚SPI0功能 SPI0 MISO*/
    FGPIOB_PSOR |= 1<<0;                                                /* 配置CS1置高                  */
    FGPIOB_PDDR |= 1<<0;                                                /* 配置CS1为输出                */
    
    /*
     *  开启PA4 PD6管脚GPIO功能  按键输入上拉
     */
    PORTA_PCR4  |= (PORT_PCR_MUX(1) | PORT_PCR_PE_MASK);
	  PORTD_PCR6  |= (PORT_PCR_MUX(1) | PORT_PCR_PE_MASK);
    
		FGPIOB_PSOR |= 1<<18;                                               /* 配置GPIO 置高                */
    FGPIOB_PDDR |= 1<<18;                                               /* 配置GPIO为输出               */
    FGPIOB_PSOR |= 1<<19;                                               /* 配置GPIO 置高                */
    FGPIOB_PDDR |= 1<<19;                                               /* 配置GPIO为输出               */
    FGPIOD_PSOR |= 1<<1;                                                /* 配置GPIO 置高                */
    FGPIOD_PDDR |= 1<<1;                                                /* 配置GPIO为输出               */
    
    FGPIOA_PDDR &= ~(1<<4);                                             /* 配置GPIO为输入               */
		FGPIOD_PDDR &= ~(1<<6);
	//////////end io initlization
	
    SIM_SCGC4 |= SIM_SCGC4_SPI0_MASK;                                   /* 开启SPI0                     */
                                                                        /* CPOL = 0, SCK 为低有效       */
                                                                        /* MSTR = 1, SPI 处于主模式     */
                                                                        /* LSBF = 0, MSB (位7)在先      */
                                                                        /* SPIE = 0, SPI 中断被禁止     */
                                                                        /* CPHA = 0, SCK第一个时钟沿采样*/
    SPI->C1 |= SPI_C1_SPE_MASK ;                                        /* SPI使能                      */
    SPI->C1 &= ~(1 << SPI_C1_CPHA_SHIFT);
    if(bMode == Master) {                                               /* 主模式                       */
        SPI->C1 |= SPI_C1_MSTR_MASK;
    } else {                                                            /* 从模式                       */
        SPI->C1 &= ~(1 << SPI_C1_MSTR_SHIFT);
    }
    SPI->BR |= SPI_BR_SPPR(0) ;                                         /* 时钟分频                     */
}

/*********************************************************************************************************
** 函数名称: Send_Byte
** 函数功能：SPI发送一个字节数据 SPI0驱动
** 输入参数: data
** 输出参数: 无
** 返 回 值：无
*********************************************************************************************************/
uint_8 Send_Byte (uint_8 ucdata)
{
    uint_8 ucTemp;
    while((SPI0_S & SPI_S_SPTEF_MASK) != SPI_S_SPTEF_MASK);             /* 发送前要先判断寄存器         */
    SPI0_DL = ucdata;
    
    while((SPI0_S & SPI_S_SPRF_MASK) != SPI_S_SPRF_MASK);               /* 接收数据有效                 */
    ucTemp = SPI0_DL;                                                    /* 清空接收标记寄存器           */
    ucTemp = ucTemp;
	return ucTemp;
}

/*********************************************************************************************************
** 函数名称:Get_Byte
** 函数功能:SPI接口接收一个字节数据 使用SPI0
** 输入参数:无
** 输出参数:无
*********************************************************************************************************/
uint_8 Get_Byte (void)
{
    uint_8 ucTemp;
    while((SPI0_S & SPI_S_SPTEF_MASK) != SPI_S_SPTEF_MASK);             /* 发送前要先判断寄存器         */
    SPI0_DL = 0xff;                                                      /* 发送空数据读取内容           */

    while((SPI0_S & SPI_S_SPRF_MASK) != SPI_S_SPRF_MASK);               /* 接收数据有效                 */
    ucTemp = SPI0_DL;
    return (uint_8)(ucTemp);                                             /* 返回接收到的数据             */
}

/*********************************************************************************************************
** Function name:           spiTX
** Descriptions:            spi初始化
** input parameters:        SPI输入基地址 查询方式发送数据
** output parameters:       none
** Returned value:          none
** Created by:              JiaoLong Liu
** Created date:            2012-12-12
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
*********************************************************************************************************/
void spiTX(SPI_MemMapPtr SPI,uint_8 ucData)
{
    while((SPI->S & SPI_S_SPTEF_MASK) != SPI_S_SPTEF_MASK);             /* 发送前要先判断寄存器         */
    SPI->DL = ucData;
}

/*********************************************************************************************************
  END FILE 
*********************************************************************************************************/



