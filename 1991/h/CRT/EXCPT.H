/***
*excpt.h - defines exception values, types and routines
*
*    Copyright (c) 1990-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*    This file contains the definitions and prototypes for the compiler-
*    dependent intrinsics, support functions and keywords which implement
*    the structured exception handling extensions.
*
****/

#ifndef _INC_EXCPT
#define _INC_EXCPT

#define _CRTAPI1
#define _CRTAPI2

/*
 * Exception disposition return values.
 */
typedef enum _EXCEPTION_DISPOSITION {
    ExceptionContinueExecution,
    ExceptionContinueSearch,
    ExceptionNestedException,
    ExceptionCollidedUnwind
} EXCEPTION_DISPOSITION;


/*
 * Declarations to keep MIPS compiler happy
 */
typedef struct _EXCEPTION_POINTERS *Exception_info_ptr;
struct _EXCEPTION_RECORD;
struct _CONTEXT;
struct _DISPATCHER_CONTEXT;

EXCEPTION_DISPOSITION __C_specific_handler (
	struct _EXCEPTION_RECORD *ExceptionRecord,
	void *EstablisherFrame,
	struct _CONTEXT *ContextRecord,
	struct _DISPATCHER_CONTEXT *DispatcherContext
	);

#define try                     __builtin_try
#define except                  __builtin_except
#define finally                 __builtin_finally
#define leave                   __builtin_leave
#define GetExceptionCode()      __exception_code
#define exception_code()        __exception_code
#define GetExceptionInformation()       (struct _EXCEPTION_POINTERS *)__exception_info
#define exception_info()                (struct _EXCEPTION_POINTERS *)__exception_info
#define AbnormalTermination()   __abnormal_termination
#define abnormal_termination()  __abnormal_termination

extern unsigned long __exception_code;
extern int __exception_info;
extern int __abnormal_termination;

#define EXCEPTION_EXECUTE_HANDLER    1
#define EXCEPTION_CONTINUE_SEARCH    0
#define EXCEPTION_CONTINUE_EXECUTION -1

#endif /* _INC_EXCPT */
