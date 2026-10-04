/***
*io.h - declarations for low-level file handling and I/O functions
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains the function declarations for the low-level
*	file handling and I/O functions.
*
****/

#ifndef _INC_IO


/* function prototypes */

int _access(const char *, int);
int _chmod(const char *, int);
int _chsize(int, long);
int _close(int);
int _commit(int);
int _creat(const char *, int);
int _dup(int);
int _dup2(int, int);
int _eof(int);
long _filelength(int);
int _isatty(int);
int _locking(int, int, long);
long _lseek(int, long, int);
char * _mktemp(char *);
int _open(const char *, int, ...);
int _pipe(int *, unsigned int, int);
int _read(int, void *, unsigned int);
int remove(const char *);
int rename(const char *, const char *);
int _setmode(int, int);
int _sopen(const char *, int, int, ...);
long _tell(int);
int _umask(int);
int _unlink(const char *);
int _write(int, const void *, unsigned int);

long _get_osfhandle(int);
int _open_osfhandle(long, int);

#ifndef __STDC__
/* Non-ANSI names for compatibility */
#define access	   _access
#define chmod	   _chmod
#define chsize	   _chsize
#define close	   _close
#define creat	   _creat
#define dup	   _dup
#define dup2	   _dup2
#define eof	   _eof
#define filelength _filelength
#define isatty	   _isatty
#define locking    _locking
#define lseek	   _lseek
#define mktemp	   _mktemp
#define open	   _open
#define read	   _read
#define setmode    _setmode
#define sopen	   _sopen
#define tell	   _tell
#define umask	   _umask
#define unlink	   _unlink
#define write	   _write
#endif	/* __STDC__ */

#define _INC_IO

#endif	/* _INC_IO */
