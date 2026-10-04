/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1990-1991  Microsoft Corporation

Module Name:

    ntmips.h

Abstract:

    User-mode visible Mips specific structures and constants

Author:

    David N. Cutler (davec) 31-Mar-1990

Revision History:

--*/

#ifndef _NTMIPS_
#define _NTMIPS_

#include <ntdef.h>
#include <ntxcapi.h>
#include <ntkeapi.h>

//
// Define address space layout as defined by MIPS memory management.
//

#define KUSEG_BASE 0x0                  // base of user segment
#define KSEG0_BASE 0x80000000           // base of cached kernel physical
#define KSEG1_BASE 0xa0000000           // base of uncached kernel physical
#define KSEG2_BASE 0xc0000000           // base of cached kernel virtual

//
// Define MIPS specific kernel debugger information.
//
// The following structure contains machine specific data passed to
// the host system kernel debugger in a wait state change message.
//

#define DBGKD_MAXSTREAM 16

typedef struct _DBGKD_CONTROL_REPORT {
    ULONG InstructionCount;
    UCHAR InstructionStream[DBGKD_MAXSTREAM];
} DBGKD_CONTROL_REPORT, *PDBGKD_CONTROL_REPORT;

//
// The following structure contains information that the host system
// kernel debugger wants to set on every continue operation and avoids
// the need to send extra packets of information.
//

typedef ULONG DBGKD_CONTROL_SET, *PDBGKD_CONTROL_SET;


//
// Define processor status register fields.
//

typedef struct _PSR {
    ULONG IEC : 1;
    ULONG KUC : 1;
    ULONG IEP : 1;
    ULONG KUP : 1;
    ULONG IEO : 1;
    ULONG KUO : 1;
    ULONG X1 : 2;
    ULONG INTMASK : 8;
    ULONG ISC : 1;
    ULONG SWC : 1;
    ULONG PZ : 1;
    ULONG CM : 1;
    ULONG PE : 1;
    ULONG TS : 1;
    ULONG BEV : 1;
    ULONG X2 : 5;
    ULONG CU0 : 1;
    ULONG CU1 : 1;
    ULONG CU2 : 1;
    ULONG CU3 : 1;
} PSR, *PPSR;

//
// Define system time structure.
//

typedef struct _KSYSTEM_TIME {
    ULONG LowPart;
    LONG High1Time;
    LONG High2Time;
} KSYSTEM_TIME, *PKSYSTEM_TIME;

//
// Define Processor Control Region Structure.
//


typedef struct _KPCR {
    struct _KPRCB *Prcb;
    KSPIN_LOCK DispatcherLock;
    KSPIN_LOCK DpcLock;
    KSPIN_LOCK ProfileLock;
    PVOID XcodeDispatch[32];
    PLIST_ENTRY ProfileListHead;
    ULONG ProfileInterval;
    ULONG ProfileCount;
    ULONG Reserved;
    PVOID UserApcDispatcher;
    PVOID UserExceptionDispatcher;
    PVOID RtlpLockRangeStart;
    PVOID RtlpLockRangeEnd;
    PVOID ArgumentTable;
    PVOID InitialStack;
    PVOID PanicStack;
    PVOID ServiceTable;
    PVOID SystemGp;
    PVOID Teb;
    PKINTERRUPT_ROUTINE *InterruptRoutine;
    struct _KTHREAD *CurrentThread;
    PLIST_ENTRY DpcListHead;
    struct _KTIMER **FirstTimer;
    struct _KDPC *QuantumEndDpc;
    PKSYSTEM_TIME SystemTime;
    struct _KDPC *TimerExpireDpc;
    ULONG SavedEpc;
    ULONG FirstLevelDcacheSize;
    ULONG FirstLevelIcacheSize;
    PVOID PageDirectory;
    ULONG BadVaddr;
    ULONG SavedT8;
    ULONG SavedT9;
    PULONG ServiceLimit;
    ULONG FirstFreePage;
    ULONG LastFreePage;
    ULONG TimeIncrement;
    UCHAR IrqlMask[32];
    UCHAR IrqlTable[9];
    UCHAR CurrentIrql;
    UCHAR SetMember;
    UCHAR Spare;
} KPCR, *PKPCR;

//
// The following flags control the contents of the CONTEXT structure.
//

#define CONTEXT_CONTROL         0x00010001L
#define CONTEXT_FLOATING_POINT  0x00010002L
#define CONTEXT_INTEGER         0x00010004L

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER)

typedef struct _CONTEXT {

    //
    // This section is always present and is used as an argument build
    // area.
    //

    ULONG Argument[4];

    //
    // This section is specified/returned if the ContextFlags word contains
    // the flag CONTEXT_FLOATING_POINT.
    //

    ULONG FltF0;
    ULONG FltF1;
    ULONG FltF2;
    ULONG FltF3;
    ULONG FltF4;
    ULONG FltF5;
    ULONG FltF6;
    ULONG FltF7;
    ULONG FltF8;
    ULONG FltF9;
    ULONG FltF10;
    ULONG FltF11;
    ULONG FltF12;
    ULONG FltF13;
    ULONG FltF14;
    ULONG FltF15;
    ULONG FltF16;
    ULONG FltF17;
    ULONG FltF18;
    ULONG FltF19;
    ULONG FltF20;
    ULONG FltF21;
    ULONG FltF22;
    ULONG FltF23;
    ULONG FltF24;
    ULONG FltF25;
    ULONG FltF26;
    ULONG FltF27;
    ULONG FltF28;
    ULONG FltF29;
    ULONG FltF30;
    ULONG FltF31;

    //
    // This section is specified/returned if the ContextFlags word contains
    // the flag CONTEXT_INTEGER.
    //
    // N.B. The registers gp, sp, and ra are defined in this section, but are
    //  considered part of the control context rather than part of the integer
    //  context.
    //
    // N.B. Register zero is not stored in the frame.
    //

    ULONG IntZero;
    ULONG IntAt;
    ULONG IntV0;
    ULONG IntV1;
    ULONG IntA0;
    ULONG IntA1;
    ULONG IntA2;
    ULONG IntA3;
    ULONG IntT0;
    ULONG IntT1;
    ULONG IntT2;
    ULONG IntT3;
    ULONG IntT4;
    ULONG IntT5;
    ULONG IntT6;
    ULONG IntT7;
    ULONG IntS0;
    ULONG IntS1;
    ULONG IntS2;
    ULONG IntS3;
    ULONG IntS4;
    ULONG IntS5;
    ULONG IntS6;
    ULONG IntS7;
    ULONG IntT8;
    ULONG IntT9;
    ULONG IntK0;
    ULONG IntK1;
    ULONG IntGp;
    ULONG IntSp;
    ULONG IntS8;
    ULONG IntRa;
    ULONG IntLo;
    ULONG IntHi;

    //
    // This section is specified/returned if the ContextFlags word contains
    // the flag CONTEXT_FLOATING_POINT.
    //

    ULONG Fsr;

    //
    // This section is specified/returned if the ContextFlags word contains
    // the flag CONTEXT_CONTROL.
    //
    // N.B. The registers gp, sp, and ra are defined in the integer section,
    //   but are considered part of the control context rather than part of
    //   the integer context.
    //

    ULONG Fir;
    ULONG Psr;

    //
    // The flags values within this flag control the contents of
    // a CONTEXT record.
    //
    // If the context record is used as an input parameter, then
    // for each portion of the context record controlled by a flag
    // whose value is set, it is assumed that that portion of the
    // context record contains valid context. If the context record
    // is being used to modify a thread's context, then only that
    // portion of the threads context will be modified.
    //
    // If the context record is used as an IN OUT parameter to capture
    // the context of a thread, then only those portions of the thread's
    // context corresponding to set flags will be returned.
    //
    // The context record is never used as an OUT only parameter.
    //

    ULONG ContextFlags;

    ULONG Fill[2];
} CONTEXT, *PCONTEXT;

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {
    PULONG FloatingContext[20];
    PULONG FltF20;
    PULONG FltF21;
    PULONG FltF22;
    PULONG FltF23;
    PULONG FltF24;
    PULONG FltF25;
    PULONG FltF26;
    PULONG FltF27;
    PULONG FltF28;
    PULONG FltF29;
    PULONG FltF30;
    PULONG FltF31;
    PULONG IntegerContext[16];
    PULONG IntS0;
    PULONG IntS1;
    PULONG IntS2;
    PULONG IntS3;
    PULONG IntS4;
    PULONG IntS5;
    PULONG IntS6;
    PULONG IntS7;
    PULONG IntT8;
    PULONG IntT9;
    PULONG IntK0;
    PULONG IntK1;
    PULONG IntGp;
    PULONG IntSp;
    PULONG IntS8;
    PULONG IntRa;
} KNONVOLATILE_CONTEXT_POINTERS, *PKNONVOLATILE_CONTEXT_POINTERS;

//
// Define R3000 PTE format for memory management.
//

typedef struct _HARDWARE_PTE {
    ULONG Filler : 6;
    ULONG Write : 1;
    ULONG CopyOnWrite : 1;
    ULONG Global : 1;
    ULONG Valid : 1;
    ULONG Dirty : 1;
    ULONG CacheDisable : 1;
    ULONG PageFrameNumber : 20;
} HARDWARE_PTE, *PHARDWARE_PTE;

//
// Define MIPS exception handling structures and function prototypes.
//
// Function table entry structure definition.
//

typedef struct _RUNTIME_FUNCTION {
    ULONG BeginAddress;
    ULONG EndAddress;
    PEXCEPTION_ROUTINE ExceptionHandler;
    PVOID HandlerData;
    ULONG PrologEndAddress;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

//
// Scope table structure definition.
//

typedef struct _SCOPE_TABLE {
    ULONG Count;
    struct
    {
        ULONG BeginAddress;
        ULONG EndAddress;
        ULONG HandlerAddress;
        ULONG JumpTarget;
    } ScopeRecord[1];
} SCOPE_TABLE, *PSCOPE_TABLE;

//
// Runtime Library function prototypes.
//

VOID
RtlCaptureContext (
    OUT PCONTEXT ContextRecord
    );

PRUNTIME_FUNCTION
RtlLookupFunctionEntry (
    IN ULONG ControlPc
    );

ULONG
RtlVirtualUnwind (
    IN ULONG ControlPc,
    IN PRUNTIME_FUNCTION FunctionEntry,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PKNONVOLATILE_CONTEXT_POINTERS ContextPointers OPTIONAL
    );

//
// Define C structured exception handing function prototypes.
//

typedef struct _DISPATCHER_CONTEXT {
    ULONG ControlPc;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG EstablisherFrame;
    PCONTEXT ContextRecord;
} DISPATCHER_CONTEXT, *PDISPATCHER_CONTEXT;


struct _EXCEPTION_POINTERS;

typedef
LONG
(*EXCEPTION_FILTER) (
    struct _EXCEPTION_POINTERS *ExceptionPointers
    );

typedef
VOID
(*TERMINATION_HANDLER) (
    BOOLEAN is_abnormal
    );

// begin_winnt

#if defined(_MIPS_)

VOID
__jump_unwind (
    PVOID Fp,
    PVOID TargetPc
    );

#endif // MIPS

// end_winnt

#define NtCurrentTeb() (*(struct _TEB *volatile *)0x7ffff0c4UL)

#endif
