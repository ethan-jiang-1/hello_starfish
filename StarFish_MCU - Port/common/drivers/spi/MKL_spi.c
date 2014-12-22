
#include "MKL_spi.h"


/*********************************************************************************************************
** Function name:           spiInit
** Descriptions:            spi��ʼ��
** input parameters:        SPI�������ַ ��������ģʽ
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
     *  SPI�Ĺܽų�ʼ��
     */
    PORTE_PCR16  |= PORT_PCR_MUX(1);                                     /* ����PB0�ܽ�GPIO���� SPI0 CS1 */
    PORTE_PCR17  |= PORT_PCR_MUX(2)|(uint32_t)~(uint32_t)PORT_PCR_MUX(5);                                   /* ����PC5�ܽ�SPI0���� SPI0 SCK */
    PORTE_PCR19  |= PORT_PCR_MUX(2)|(uint32_t)~(uint32_t)PORT_PCR_MUX(5);                                     /* ����PC6�ܽ�SPI0���� SPI0 MOSI*/
    PORTE_PCR18  |= PORT_PCR_MUX(2)|(uint32_t)~(uint32_t)PORT_PCR_MUX(5);                                     /* ����PC7�ܽ�SPI0���� SPI0 MISO*/
    FGPIOE_PSOR |= 1<<16;                                                /* ����CS1�ø�                  */
    FGPIOE_PDDR |= 1<<16;                                                /* ����CS1Ϊ���                */
    
    
	
    SIM_SCGC4 |= SIM_SCGC4_SPI0_MASK;                                   /* ����SPI0                     */
                                                                        /* CPOL = 0, SCK Ϊ����Ч       */
                                                                        /* MSTR = 1, SPI ������ģʽ     */
                                                                        /* LSBF = 0, MSB (λ7)����      */
                                                                        /* SPIE = 0, SPI �жϱ���ֹ     */
                                                                        /* CPHA = 0, SCK��һ��ʱ���ز���*/
    SPI->C1 |= SPI_C1_SPE_MASK ;                                        /* SPIʹ��                      */
    SPI->C1 &= ~(1 << SPI_C1_CPHA_SHIFT);
    if(bMode == Master) {                                               /* ��ģʽ                       */
        SPI->C1 |= SPI_C1_MSTR_MASK;
    } else {                                                            /* ��ģʽ                       */
        SPI->C1 &= ~(1 << SPI_C1_MSTR_SHIFT);
    }
    SPI->BR |= SPI_BR_SPPR(0) ;                                         /* ʱ�ӷ�Ƶ                     */
}

/*********************************************************************************************************
** ��������: Send_Byte
** �������ܣ�SPI����һ���ֽ����� SPI0����
** �������: data
** �������: ��
** �� �� ֵ����
*********************************************************************************************************/
uint_8 Send_Byte (uint_8 ucdata)
{
    uint_8 ucTemp;
    while((SPI0_S & SPI_S_SPTEF_MASK) != SPI_S_SPTEF_MASK);             /* ����ǰҪ���жϼĴ���         */
    SPI0_DL = ucdata;
    
    while((SPI0_S & SPI_S_SPRF_MASK) != SPI_S_SPRF_MASK);               /* ����������Ч                 */
    ucTemp = SPI0_DL;                                                    /* ��ս��ձ�ǼĴ���           */
    ucTemp = ucTemp;
	return ucTemp;
}

/*********************************************************************************************************
** ��������:Get_Byte
** ��������:SPI�ӿڽ���һ���ֽ����� ʹ��SPI0
** �������:��
** �������:��
*********************************************************************************************************/
uint_8 Get_Byte (void)
{
    uint_8 ucTemp;
    while((SPI0_S & SPI_S_SPTEF_MASK) != SPI_S_SPTEF_MASK);             /* ����ǰҪ���жϼĴ���         */
    SPI0_DL = 0xff;                                                      /* ���Ϳ����ݶ�ȡ����           */

    while((SPI0_S & SPI_S_SPRF_MASK) != SPI_S_SPRF_MASK);               /* ����������Ч                 */
    ucTemp = SPI0_DL;
    return (uint_8)(ucTemp);                                             /* ���ؽ��յ�������             */
}

/*********************************************************************************************************
** Function name:           spiTX
** Descriptions:            spi��ʼ��
** input parameters:        SPI�������ַ ��ѯ��ʽ��������
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
    while((SPI->S & SPI_S_SPTEF_MASK) != SPI_S_SPTEF_MASK);             /* ����ǰҪ���жϼĴ���         */
    SPI->DL = ucData;
}

/*********************************************************************************************************
  END FILE 
*********************************************************************************************************/



