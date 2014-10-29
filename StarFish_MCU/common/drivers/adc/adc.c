#include "io_map.h"
#include "adc.h"

uint8_t cycle_flags;
uint32_t result0A;


void ADC_Init(void)
{
//  	//Turn on ADC and LPTMR clock gate
	SIM_SCGC6 |= SIM_SCGC6_ADC0_MASK;  
//	
//	ADC0_CFG1 &= ~ADC_CFG1_ADICLK_MASK;
//    ADC0_CFG1 |= ADC_CFG1_ADICLK(0);	//bus clock
//	
//	ADC0_CFG1 &= ~ADC_CFG1_MODE_MASK;
////	ADC0_CFG1 |= ADC_CFG1_MODE(0);	//singled-ended 8bit mode
//	ADC0_CFG1 |= ADC_CFG1_MODE(3);	//singled-ended 16bit mode
//        
//	ADC0_CFG1 &= ~ADC_CFG1_ADIV_MASK;
//	ADC0_CFG1 |= ADC_CFG1_ADIV(3); //div by 8
//	
//	ADC0_CFG2 |= ADC_CFG2_MUXSEL_MASK  //select channel B
//			| ADC_CFG2_ADHSC_MASK	//high speed mode
//			;
//	
//	ADC0_SC2 |= ADC_SC2_ADTRG_MASK;	//hardware trigger
//	
//	ADC0_SC2 &= ~ADC_SC2_REFSEL_MASK;
//	ADC0_SC2 |= ADC_SC2_REFSEL(0);		//VREFH/VREFL
//	 
//	ADC0_SC3 &= ~ADC_SC3_ADCO_MASK;  //single shot mode
//	
//	ADC0_SC3 |= ADC_SC3_AVGE_MASK; 	//hardware average
//	
//	ADC0_SC3 &= ~ADC_SC3_AVGS_MASK;
//	ADC0_SC3 |= ADC_SC3_AVGS(0x00); 		//32 sample  		
//        
//        //jjtest
//     //   PMC_REGSC |= 0X01;  //OPEN BANDGAP
 /* ADC0_CFG1: ??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,ADLPC=0,ADIV=0,ADLSMP=0,MODE=3,ADICLK=0 */
  ADC0_CFG1 = ADC_CFG1_ADIV(0x00) |
              ADC_CFG1_MODE(0x03) |
              ADC_CFG1_ADICLK(0x00);       
  /* ADC0_CFG2: ??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,ADACKEN=0,ADHSC=0,ADLSTS=0 */
  ADC0_CFG2 &= (uint32_t)~(uint32_t)(
                ADC_CFG2_ADACKEN_MASK |
                ADC_CFG2_ADHSC_MASK |
                ADC_CFG2_ADLSTS(0x03) |
                0xFFFFFFE0U
               );                                   
  /* ADC0_CV1: ??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,CV=0 */
  ADC0_CV1 = ADC_CV1_CV(0x00);                                   
  /* ADC0_CV2: ??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,CV=0 */
  ADC0_CV2 = ADC_CV2_CV(0x00);                                   
  /* ADC0_OFS: ??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,OFS=4 */
  ADC0_OFS = ADC_OFS_OFS(0x04);                                   
  /* ADC0_SC2: ??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,ADACT=0,ADTRG=0,ACFE=0,ACFGT=0,ACREN=0,DMAEN=1,REFSEL=0 */
  ADC0_SC2 = (ADC_SC2_DMAEN_MASK | ADC_SC2_REFSEL(0x00));                                   
  /* ADC0_SC3: ??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,CAL=0,CALF=0,??=0,??=0,ADCO=0,AVGE=1,AVGS=0 */
  ADC0_SC3 = (ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(0x00));                                   
  /* ADC0_SC1A: ??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,COCO=0,AIEN=0,DIFF=0,ADCH=0x1F */
  ADC0_SC1A = ADC_SC1_ADCH(0x1F);                                   
  /* ADC0_SC1B: ??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,COCO=0,AIEN=0,DIFF=0,ADCH=0x1F */
  ADC0_SC1B = ADC_SC1_ADCH(0x1F); 

}



uint8_t  Calibrate_ADC(void) {
    uint16_t cal_temp;
    
	ADC_Init();
	
	ADC0_SC3 |= ADC_SC3_CALF_MASK;		// clear any existing calibration failed flag
    ADC0_SC3 |= ADC_SC3_CAL_MASK;		// Start Calibration
	while(ADC0_SC3 & ADC_SC3_CAL_MASK){	// wait for calibration to finish
	};
	if(ADC0_SC3 & ADC_SC3_CALF_MASK) 		// There was a fault during calibration
		return 1;

	// Initialize (clear) a 16b variable in RAM.
	cal_temp = 0;
	
    //Add the following plus-side calibration results CLP0, CLP1, CLP2, CLP3, CLP4, CLPS and CLPD to the variable.
	cal_temp += ADC0_CLP0;
	cal_temp += ADC0_CLP1;
	cal_temp += ADC0_CLP2;
	cal_temp += ADC0_CLP3;
	cal_temp += ADC0_CLP4;
	cal_temp += ADC0_CLPS;

	// Divide the variable by two.
	cal_temp = cal_temp >> 1;
	// Set the MSB of the variable.
	cal_temp |= 0x8000;
	
	// Store the value in the plus-side gain calibration registers ADCPG
	ADC0_PG = cal_temp;
	
	// Repeat procedure for the minus-side gain calibration value.
	// Initialize (clear) a 16b variable in RAM.
	cal_temp = 0;

    //Add the following minus-side calibration results CLM0, CLM1, CLM2, CLM3, CLM4, CLMS and CLMD to the variable.
	cal_temp += ADC0_CLM0;
	cal_temp += ADC0_CLM1;
	cal_temp += ADC0_CLM2;
	cal_temp += ADC0_CLM3;
	cal_temp += ADC0_CLM4;
	cal_temp += ADC0_CLMS;
	
	// Divide the variable by two.
	cal_temp = cal_temp >> 1;
	// Set the MSB of the variable.
	cal_temp |= 0x8000;
	
	// Store the value in the minus-side gain calibration registers ADCMG
	ADC0_MG = cal_temp;

	return 0;
}



void ADC_Start(uint8_t channel)
{
    ADC0_SC1A = (ADC_SC1_AIEN_MASK |channel); 	
}

void ADC_Stop(void)
{
    ADC0_SC1A = ADC_SC1_ADCH_MASK; //stop conversion
}



void ADC_Trigger(void)
{
	//Configure LPTMR as ADC trigger
	SIM_SOPT7 |= (SIM_SOPT7_ADC0ALTTRGEN_MASK 
				  | !SIM_SOPT7_ADC0PRETRGSEL_MASK);
	SIM_SOPT7 &= ~SIM_SOPT7_ADC0TRGSEL_MASK;	
	SIM_SOPT7 |= SIM_SOPT7_ADC0TRGSEL(LPTMR0_TRG);   
}



