/*
 * File:		NVIC.h
 * Purpose:		Definitions for NVIC operation
 *
 * Notes:
 */

#ifndef _NVIC_H_
#define _NVIC_H_

#include "common.h"

/*ARM Cortex M0 implementation for interrupt priority shift*/
#define ARM_INTERRUPT_LEVEL_BITS          2


void enable_irq (int);
void disable_irq (int);
void set_irq_priority (int, int);


#endif	/* _NVIC_H_ */

