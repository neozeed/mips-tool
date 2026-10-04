/***
*stdarg.h - defines ANSI-style macros for variable argument functions
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines ANSI-style macros for accessing arguments
*	of functions which take a variable number of arguments.
*	[ANSI]
*
****/

#ifndef _INC_STDARG



#ifndef _VA_LIST_DEFINED
typedef char *	va_list;
#define _VA_LIST_DEFINED
#endif


#ifdef i386
#define _INTSIZEOF(n)	( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) ) 

#define va_start(ap,v)	( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)	( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)	( ap = (va_list)0 )
#endif

#ifdef MIPS

#define va_start(ap,v) ap  = (va_list)&v + sizeof(v)
#define va_end(list)
#define va_arg(list, mode) ((mode *)(list =\
 (char *) ((((int)list + (__builtin_alignof(mode)<=4?3:7)) &\
 (__builtin_alignof(mode)<=4?-4:-8))+sizeof(mode))))[-1]

/*  +++++++++++++++++++++++++++++++++++++++++++
    Because of parameter passing conventions in C:
    use mode=int for char, and short types
    use mode=double for float types
    use a pointer for array types
    +++++++++++++++++++++++++++++++++++++++++++ */


#endif

#define _INC_STDARG

#endif	/* _INC_STDARG */
