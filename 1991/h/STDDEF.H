/***
*stddef.h - definitions/declarations for common constants, types, variables
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains definitions and declarations for some commonly
*	used constants, types, and variables.
*	[ANSI]
*
****/

#ifndef _INC_STDDEF



/* define NULL pointer value and the offset() macro */

#ifndef  NULL
#define  NULL	((void *)0)
#endif

#define offsetof(s,m)	(size_t)&(((s *)0)->m)


/* declare reference to errno */

#ifdef	_MT
extern int * _errno(void);
#define errno	(*_errno())
#else
extern int errno;
#endif


/* define the implementation dependent size types */

#ifndef _PTRDIFF_T_DEFINED
typedef int ptrdiff_t;
#define _PTRDIFF_T_DEFINED
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif


#ifdef	_MT

/* define pointer to thread id value */
extern unsigned long * __threadid(void);
#define _threadid   (__threadid())

#endif

#define _INC_STDDEF

#endif	/* _INC_STDDEF */
