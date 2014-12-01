

#include "lptmr.h"
#include "io_map.h"
#include "app_trace.h"
#include "nvic.h"
#include "Common.h"

extern void dma_setup();
volatile bool bSysActive=FALSE;
volatile bool bVLPSMode=FALSE;
#define CPU_STOP_TICK 20
uint_8 sleepcounter=0;
LWSEM_STRUCT     g_lptmr_int_sem;
extern uint_8 GetSysStatus();
extern void SetSysStatus(uint_8 sys);
extern uint_8 GetAccStatus();
extern void SetAccStatus(uint_8 sys);
/*******************************************************************************
*
*   PROCEDURE NAME:
*       lptmr_init -
*
*******************************************************************************/
void lptmr_isr(void* p_arg)
{
  //GPIOB_PSOR = 0x00080000;           // clear Port B 19 indicate wakeup
//	GPIOA_PDOR ^= (1<<16);
  //APP_TRACE("\n****LPT ISR entered*****\r\n");
  // enable timer
  // enable interrupts
  // clear the flag
  LPTMR0_CSR |=  LPTMR_CSR_TCF_MASK;   // write 1 to TCF to clear the LPT timer compare flag
  LPTMR0_CSR = ( LPTMR_CSR_TEN_MASK | LPTMR_CSR_TIE_MASK | LPTMR_CSR_TCF_MASK  );
  dma_setup(); 

	if(GetSysStatus()==ACTIVE_OPEN && GetAccStatus()==ACC_STILL)
	{
		sleepcounter++;
		if(sleepcounter>CPU_STOP_TICK)
		{
			sleepcounter=0;
			SetSysStatus(ACTIVE_STILL);
		}
	}
		
	_lwsem_post(&g_lptmr_int_sem);

}
void Lptmr_Init(int count, int clock_source)
{
	 _mqx_uint       mqx_ret;
    SIM_SCGC5 |= SIM_SCGC5_LPTMR_MASK;
	
	SIM_SOPT1 &= ~SIM_SOPT1_OSC32KSEL_MASK;
    SIM_SOPT1 |= SIM_SOPT1_OSC32KSEL(2); // ERCLK32 is RTC OSC CLOCK
	
	PORTC_PCR1 &= ~PORT_PCR_MUX_MASK;
	PORTC_PCR1 |= PORT_PCR_MUX(1);//enable ptc1 alt1 functions to select RTC_CLKIN function
	
	/********************************************************************************* 
	*	On L2K tower board, we use external 32kHz clock instead of 32kHz crystal, so please
	*	don't enable the 32kHz crystal oscillator 
	**********************************************************************************/
	/*
	RTC_CR |= RTC_CR_OSCE_MASK |
              RTC_CR_CLKO_MASK |
              RTC_CR_SC8P_MASK  ;	*/

	LPTMR0_PSR &= ~LPTMR_PSR_PRESCALE_MASK;
    LPTMR0_PSR |=  LPTMR_PSR_PRESCALE(0); // 0000 is div 2
    LPTMR0_PSR |= LPTMR_PSR_PBYP_MASK;  // LPO feeds directly to LPT
    LPTMR0_PSR &= ~LPTMR_PSR_PCS_MASK;
	LPTMR0_PSR |= LPTMR_PSR_PCS(clock_source); // use the choice of clock
	
    if (clock_source== 0)
      APP_TRACE("\n LPTMR Clock source is the MCGIRCLK \n\r");
    if (clock_source== 1)
      APP_TRACE("\n LPTMR Clock source is the LPOCLK \n\r");
    if (clock_source== 2)
      APP_TRACE("\n LPTMR Clock source is the ERCLK32 \n\r");
    if (clock_source== 3)
      APP_TRACE("\n LPTMR Clock source is the OSCERCLK \n\r");
             
    LPTMR0_CMR = LPTMR_CMR_COMPARE(count);  //Set compare value

    LPTMR0_CSR |=(  LPTMR_CSR_TCF_MASK   // Clear any pending interrupt
                 | LPTMR_CSR_TIE_MASK   // LPT interrupt enabled
                 |!LPTMR_CSR_TPP_MASK   //TMR Pin polarity
                 |!LPTMR_CSR_TFC_MASK   // Timer Free running counter is reset whenever TMR counter equals compare
                 |!LPTMR_CSR_TMS_MASK   //LPTMR0 as Timer
                );
		enable_irq(28) ;
		_int_install_isr(LDD_ivIndex_INT_LPTimer, lptmr_isr, NULL);
		    mqx_ret = _lwsem_create(&g_lptmr_int_sem, 0);
    ASSERT_PARAM(MQX_OK == mqx_ret);
//				   // ready for this interrupt.  
//		set_irq_priority(28, 2);
	
}



void Lptmr_Start(void)
{
	LPTMR0_CSR |= LPTMR_CSR_TEN_MASK;   //Turn on LPT and start counting
}


