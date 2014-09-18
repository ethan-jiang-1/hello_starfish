/**HEADER********************************************************************
*
* Copyright (c) 2010-2011 Freescale Semiconductor;
* All Rights Reserved
*
***************************************************************************
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* $FileName: boot.S$
* $Version : 3.8.10.3$
* $Date    : Dec-2-2011$
*
* Comments:
*
*
*END************************************************************************/

#include "psp_prv.inc"

#define __ASM__

#include "psp_cpu.h"

#undef __ASM__

/****************************************************************************
 *
 * Function Name    : __boot
 * Returned Value   :
 * Comments         : startup sequence
 *
 ****************************************************************************/

    ASM_CODE_SECTION(.text)
    SET_FUNCTION_ALIGNMENT

    ASM_PUBLIC(__boot)
    ASM_PUBLIC(__set_MSP)

 ASM_PUBLIC_BEGIN(__boot)
 ASM_PUBLIC_FUNC(__boot)
ASM_LABEL(__boot)
        /* prepare process stack pointer */
        mrs r0, MSP
        msr PSP, r0

        /* switch to proccess stack (PSP) */
        mrs r0, CONTROL
        movs r4, #0x2
        orrs r0, r0, r4
        msr CONTROL, r0
        isb

        ASM_EXTERN(__thumb_startup)
        bl ASM_PREFIX(__thumb_startup)
 ASM_PUBLIC_END(__boot)

 ASM_PUBLIC_BEGIN(__set_MSP)
 ASM_PUBLIC_FUNC(__set_MSP)
ASM_LABEL(__set_MSP)
        msr MSP, r0
        bx lr
 ASM_PUBLIC_END(__set_MSP)

        ASM_ALIGN(4)
        ASM_END
