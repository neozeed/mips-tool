/***
*direct.h - function declarations for directory handling/creation
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This include file contains the function declarations for the library
*	functions related to directory handling and creation.
*
****/

#ifndef _INC_DIRECT


#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

/* function prototypes */

int _chdir(const char *);
int _chdrive(int);
char * _getcwd(char *, int);
char * _getdcwd(int, char *, int);
int _getdrive(void);
int _mkdir(const char *);
int _rmdir(const char *);

#ifndef __STDC__
/* Non-ANSI names for compatibility */
#define chdir	_chdir
#define getcwd	_getcwd
#define mkdir	_mkdir
#define rmdir	_rmdir
#endif	/* __STDC__ */

#define _INC_DIRECT

#endif	/* _INC_DIRECT */
