/*
 * File:		NVIC.c
 * Purpose:		Definitions for NVIC operation
 *
 * Notes:
 */

#include "nvic.h"
#include "IO_Map.h"


/***********************************************************************/
/*
 * Initialize the NVIC to enable the specified IRQ.
 *
 * NOTE: The function only initializes the NVIC to enable a single IRQ.
 * Interrupts will also need to be enabled in the ARM core. This can be
 * done using the EnableInterrupts macro.
 *
 * Parameters:
 * irq    irq number to be enabled (the irq number NOT the vector number)
 */
void enable_irq (int irq)
{
    /* Make sure that the IRQ is an allowable number. Up to 32 is
     * used.
     *
     * NOTE: If you are using the interrupt definitions from the header
     * file, you MUST SUBTRACT 16!!!
     */
    if (irq > 32)
        return;
    else
    {
      /* Set the ICPR and ISER registers accordingly */
      NVIC_ICPR |= 1 << (irq%32);
      NVIC_ISER |= 1 << (irq%32);
    }
}


/***********************************************************************/
/*
 * Initialize the NVIC to disable the specified IRQ.
 *
 * NOTE: The function only initializes the NVIC to disable a single IRQ.
 * If you want to disable all interrupts, then use the DisableInterrupts
 * macro instead.
 *
 * Parameters:
 * irq    irq number to be disabled (the irq number NOT the vector number)
 */

void disable_irq (int irq)
{
    /* Make sure that the IRQ is an allowable number. Right now up to 32 is
     * used.
     *
     * NOTE: If you are using the interrupt definitions from the header
     * file, you MUST SUBTRACT 16!!!
     */
    if (irq > 32)
        return;
    else
      /* Set the ICER register accordingly */
      NVIC_ICER = 1 << (irq%32);
}


/***********************************************************************/
/*
 * Initialize the NVIC to set specified IRQ priority.
 *
 * NOTE: The function only initializes the NVIC to set a single IRQ priority.
 * Interrupts will also need to be enabled in the ARM core. This can be
 * done using the EnableInterrupts macro.
 *
 * Parameters:
 * irq    irq number to be enabled (the irq number NOT the vector number)
 * prio   irq priority. 0-3 levels. 0 max priority
 */

void set_irq_priority (int irq, int prio)
{
    /*irq priority pointer*/
    uint8 *prio_reg;
    uint8 err = 0;
    uint8 div = 0;

    /* Make sure that the IRQ is an allowable number. Right now up to 32 is
     * used.
     *
     * NOTE: If you are using the interrupt definitions from the header
     * file, you MUST SUBTRACT 16!!!
     */
    if (irq > 32)
    {
        err = 1;
    }

    if (prio > 3)
    {
        err = 1;
    }

    if (err != 1)
    {
        /* Determine which of the NVICIPx corresponds to the irq */
        div = irq / 4;
        prio_reg = (uint8 *)((uint32)&NVIC_IP(div));
        /* Assign priority to IRQ */
        *prio_reg = ( (prio&0x3) << (8 - ARM_INTERRUPT_LEVEL_BITS) );
    }
}


/***********************************************************************/

