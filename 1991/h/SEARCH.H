/***
*search.h - declarations for searcing/sorting routines
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains the declarations for the sorting and
*	searching routines.
*	[System V]
*
****/

#ifndef _INC_SEARCH



#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif


/* function prototypes */

void * _lsearch(const void *, void  *, unsigned int *, unsigned int,
	int (*)(const void *, const void *));
void * _lfind(const void *, const void *, unsigned int *, unsigned int,
	int (*)(const void *, const void *));
void * bsearch(const void *, const void *, size_t, size_t,
	int (*)(const void *, const void *));
void qsort(void *, size_t, size_t, int (*)(const void *,
	const void *));

#ifndef __STDC__
/* Non-ANSI names for compatibility */
#define lfind	_lfind
#define lsearch _lsearch
#endif

#define _INC_SEARCH

#endif	/* _INC_SEARCH */
