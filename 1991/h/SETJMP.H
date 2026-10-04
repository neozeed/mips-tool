/***
*setjmp.h - definitions/declarations for setjmp/longjmp routines
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the machine-dependent buffer used by
*	setjmp/longjmp to save and restore the program state, and
*	declarations for those routines.
*	[ANSI/System V]
*
*******************************************************************************/

#ifndef _INC_SETJMP



/* define the buffer type for holding the state information */

#ifdef i386
#define _JBLEN  6       /* ebx, esi, edi, ebp, esp, eip */
#endif

#ifdef MIPS
#define _JBLEN  2       /* virtual frame and target ip */
#endif


/* define the buffer type for holding the state information */

#ifndef _JMP_BUF_DEFINED
typedef  int  jmp_buf[_JBLEN];
#define _JMP_BUF_DEFINED
#endif



/* _setjmp compiler intrinsic does not work with current Win32
 * implementation so map it to setjmp
 */

#define _setjmp	setjmp




/* function prototypes */

int _setjmp(jmp_buf);
void longjmp(jmp_buf, int);

#define _INC_SETJMP

#endif	/* _INC_SETJMP */
