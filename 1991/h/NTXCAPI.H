/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989-1991  Microsoft Corporation

Module Name:

    ntxcapi.h

Abstract:

    This module contains procedure prototypes and data structures
    that support structured exception handling.

Author:

    Mark Lucovsky (markl) 29-Jun-1989

Revision History:

--*/

#ifndef _NTXCAPI_
#define _NTXCAPI_

#include <ntdef.h>

//
// Exception flag definitions.
//

#define EXCEPTION_NONCONTINUABLE 0x1    // Noncontinuable exception

#define EXCEPTION_UNWINDING 0x2         // Unwind is in progress
#define EXCEPTION_EXIT_UNWIND 0x4       // Exit unwind is in progress
#define EXCEPTION_STACK_INVALID 0x8     // Stack out of limits or unaligned
#define EXCEPTION_NESTED_CALL 0x10      // Nested exception handler call
#define EXCEPTION_TARGET_UNWIND 0x20    // Target unwind in progress
#define EXCEPTION_COLLIDED_UNWIND 0x40  // Collided exception handler call

#define EXCEPTION_UNWIND (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND | \
                          EXCEPTION_TARGET_UNWIND | EXCEPTION_COLLIDED_UNWIND)

#define IS_UNWINDING(Flag) ((Flag & EXCEPTION_UNWIND) != 0)
#define IS_DISPATCHING(Flag) ((Flag & EXCEPTION_UNWIND) == 0)
#define IS_TARGET_UNWIND(Flag) (Flag & EXCEPTION_TARGET_UNWIND)

//
// Define maximum number of exception parameters.
//

#define EXCEPTION_MAXIMUM_PARAMETERS 4 // maximum number of exception parameters

//
// Exception record definition.
//

typedef struct _EXCEPTION_RECORD {
    /*lint -e18 */  // Don't complain about different definitions
    NTSTATUS ExceptionCode;
    /*lint +e18 */  // Resume checking for different definitions
    ULONG ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    ULONG NumberParameters;
    ULONG ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
    } EXCEPTION_RECORD;
typedef EXCEPTION_RECORD *PEXCEPTION_RECORD;

typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    struct _CONTEXT *ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

//
// Exception dispatcher routine definition.
//

NTSYSAPI
BOOLEAN
NTAPI
RtlDispatchException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN struct _CONTEXT *ContextRecord
    );

//
// Exception handling procedure prototypes.
//

NTSYSAPI
VOID
NTAPI
RtlRaiseStatus (
    IN NTSTATUS Status
    );

NTSYSAPI
VOID
NTAPI
RtlRaiseException (
    IN PEXCEPTION_RECORD
    );

NTSYSAPI
VOID
NTAPI
RtlUnwind (
    IN PVOID TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue
    );

//
// Continue execution.
//

NTSYSAPI
NTSTATUS
NTAPI
NtContinue (
    IN struct _CONTEXT *ContextRecord,
    IN BOOLEAN TestAlert
    );

//
// Raise exception.
//

NTSYSAPI
NTSTATUS
NTAPI
NtRaiseException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN struct _CONTEXT *ContextRecord,
    IN BOOLEAN FirstChance
    );

#endif // _NTXCAPI_
