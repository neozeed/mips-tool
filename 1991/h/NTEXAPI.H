/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ntexapi.h

Abstract:

    This module is the header file for the all the system services that
    are contained in the "ex" directory.

Author:

    David N. Cutler (davec) 5-May-1989

Revision History:

--*/

#ifndef _NTEXAPI_
#define _NTEXAPI_

#include <ntdef.h>
#include <ntseapi.h>


//
// Delay thread execution.
//

NTSYSAPI
NTSTATUS
NTAPI
NtDelayExecution (
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER DelayInterval
    );


// begin_ntifs
//
// Event Specific Access Rights.
//

#define EVENT_QUERY_STATE       0x0001
#define EVENT_MODIFY_STATE      0x0002  // winnt
#define EVENT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3) // winnt

// end_ntifs

//
// Event Information Classes.
//

typedef enum _EVENT_INFORMATION_CLASS {
    EventBasicInformation
    } EVENT_INFORMATION_CLASS;

//
// Event Information Structures.
//

typedef struct _EVENT_BASIC_INFORMATION {
    EVENT_TYPE EventType;
    LONG EventState;
} EVENT_BASIC_INFORMATION, *PEVENT_BASIC_INFORMATION;

//
// Event object function definitions.
//



NTSYSAPI
NTSTATUS
NTAPI
NtCreateEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN EVENT_TYPE EventType,
    IN BOOLEAN InitialState
    );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
NtPulseEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQueryEvent (
    IN HANDLE EventHandle,
    IN EVENT_INFORMATION_CLASS EventInformationClass,
    OUT PVOID EventInformation,
    IN ULONG EventInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtResetEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );


//
// Event Specific Access Rights.
//

#define EVENT_PAIR_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE)


//
// Event pair object function definitions.
//

NTSYSAPI
NTSTATUS
NTAPI
NtCreateEventPair (
    OUT PHANDLE EventPairHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenEventPair(
    OUT PHANDLE EventPairHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
NtWaitLowEventPair(
    IN HANDLE EventPairHandle,
    IN BOOLEAN Alertable
    );

NTSYSAPI
NTSTATUS
NTAPI
NtWaitHighEventPair(
    IN HANDLE EventPairHandle,
    IN BOOLEAN Alertable
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetLowWaitHighEventPair(
    IN HANDLE EventPairHandle,
    IN BOOLEAN Alertable
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetHighWaitLowEventPair(
    IN HANDLE EventPairHandle,
    IN BOOLEAN Alertable
    );





NTSYSAPI
NTSTATUS
NTAPI
NtSetLowEventPair(
    IN HANDLE EventPairHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetHighEventPair(
    IN HANDLE EventPairHandle
    );


//
// Mutant Specific Access Rights.
//

// begin_winnt
#define MUTANT_QUERY_STATE      0x0001

#define MUTANT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|\
                          MUTANT_QUERY_STATE)
// end_winnt

//
// Mutant Information Classes.
//

typedef enum _MUTANT_INFORMATION_CLASS {
    MutantBasicInformation
    } MUTANT_INFORMATION_CLASS;

//
// Mutant Information Structures.
//

typedef struct _MUTANT_BASIC_INFORMATION {
    LONG CurrentCount;
    BOOLEAN OwnedByCaller;
    BOOLEAN AbandonedState;
} MUTANT_BASIC_INFORMATION, *PMUTANT_BASIC_INFORMATION;

//
// Mutant object function definitions.
//

NTSYSAPI
NTSTATUS
NTAPI
NtCreateMutant (
    IN PHANDLE MutantHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN BOOLEAN InitialOwner
    );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenMutant (
    OUT PHANDLE MutantHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQueryMutant (
    IN HANDLE MutantHandle,
    IN MUTANT_INFORMATION_CLASS MutantInformationClass,
    OUT PVOID MutantInformation,
    IN ULONG MutantInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtReleaseMutant (
    IN HANDLE MutantHandle,
    OUT PLONG PreviousCount OPTIONAL
    );


//
// Semaphore Specific Access Rights.
//

#define SEMAPHORE_QUERY_STATE       0x0001
#define SEMAPHORE_MODIFY_STATE      0x0002  // winnt

#define SEMAPHORE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3) // winnt


//
// Semaphore Information Classes.
//

typedef enum _SEMAPHORE_INFORMATION_CLASS {
    SemaphoreBasicInformation
    } SEMAPHORE_INFORMATION_CLASS;

//
// Semaphore Information Structures.
//

typedef struct _SEMAPHORE_BASIC_INFORMATION {
    LONG CurrentCount;
    LONG MaximumCount;
} SEMAPHORE_BASIC_INFORMATION, *PSEMAPHORE_BASIC_INFORMATION;

//
// Semaphore object function definitions.
//

NTSYSAPI
NTSTATUS
NTAPI
NtCreateSemaphore (
    OUT PHANDLE SemaphoreHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN LONG InitialCount,
    IN LONG MaximumCount
    );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenSemaphore(
    OUT PHANDLE SemaphoreHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQuerySemaphore (
    IN HANDLE SemaphoreHandle,
    IN SEMAPHORE_INFORMATION_CLASS SemaphoreInformationClass,
    OUT PVOID SemaphoreInformation,
    IN ULONG SemaphoreInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtReleaseSemaphore(
    IN HANDLE SemaphoreHandle,
    IN LONG ReleaseCount,
    OUT PLONG PreviousCount OPTIONAL
    );


//
// Timer Specific Access Rights.
//

#define TIMER_QUERY_STATE       0x0001
#define TIMER_MODIFY_STATE      0x0002

#define TIMER_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|\
                          TIMER_QUERY_STATE|TIMER_MODIFY_STATE)


//
// Timer Information Classes.
//

typedef enum _TIMER_INFORMATION_CLASS {
    TimerBasicInformation
    } TIMER_INFORMATION_CLASS;

//
// Timer Information Structures.
//

typedef struct _TIMER_BASIC_INFORMATION {
    LARGE_INTEGER RemainingTime;
    BOOLEAN TimerState;
} TIMER_BASIC_INFORMATION, *PTIMER_BASIC_INFORMATION;

//
// Timer APC routine definition.
//

typedef
VOID
(*PTIMER_APC_ROUTINE) (
    IN PVOID TimerContext,
    IN ULONG TimerLowValue,
    IN LONG TimerHighValue
    );

//
// Timer object function definitions.
//

NTSYSAPI
NTSTATUS
NTAPI
NtCreateTimer (
    OUT PHANDLE TimerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenTimer (
    OUT PHANDLE TimerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
NtCancelTimer (
    IN HANDLE TimerHandle,
    OUT PBOOLEAN CurrentState OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQueryTimer (
    IN HANDLE TimerHandle,
    IN TIMER_INFORMATION_CLASS TimerInformationClass,
    OUT PVOID TimerInformation,
    IN ULONG TimerInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetTimer (
    IN HANDLE TimerHandle,
    IN PLARGE_INTEGER DueTime,
    IN PTIMER_APC_ROUTINE TimerApcRoutine OPTIONAL,
    IN PVOID TimerContext OPTIONAL,
    OUT PBOOLEAN PreviousState OPTIONAL
    );


//
// System Time and Timer function definitions
//

NTSYSAPI
NTSTATUS
NTAPI
NtQuerySystemTime (
    OUT PLARGE_INTEGER SystemTime
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetSystemTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER PreviousTime OPTIONAL
    );


//
//  Locally Unique Identifier (LUID) allocation
//

NTSYSAPI
NTSTATUS
NTAPI
NtAllocateLocallyUniqueId(
    OUT PLUID Luid
    );


//
// Profile Object Definitions
//

#define PROFILE_CONTROL           0x0001
#define PROFILE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | PROFILE_CONTROL)

NTSYSAPI
NTSTATUS
NTAPI
NtCreateProfile (
    OUT PHANDLE ProfileHandle,
    IN HANDLE Process OPTIONAL,
    IN PVOID ProfileBase,
    IN ULONG ProfileSize,
    IN ULONG BucketSize,
    IN PULONG Buffer,
    IN ULONG BufferSize
    );

NTSYSAPI
NTSTATUS
NTAPI
NtStartProfile (
    IN HANDLE ProfileHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtStopProfile (
    IN HANDLE ProfileHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetIntervalProfile (
    IN ULONG Interval
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQueryIntervalProfile (
    OUT PULONG Interval
    );


//
// Performance Counter Definitions
//

NTSYSAPI
NTSTATUS
NTAPI
NtQueryPerformanceCounter (
    OUT PLARGE_INTEGER PerformanceCounter,
    OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
    );


//
// System Information Classes.
//

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,
    SystemProcessorInformation,
    SystemPerformanceInformation
} SYSTEM_INFORMATION_CLASS;

//
// System Information Structures.
//

typedef struct _SYSTEM_BASIC_INFORMATION {
    ULONG OemMachineId;
    ULONG TimerResolution;
    ULONG PageSize;
    ULONG AllocationGranularity;
    ULONG MinimumUserModeAddress;
    ULONG MaximumUserModeAddress;
    UCHAR ActiveProcessorsAffinityMask;
    UCHAR NumberOfProcessors;
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_INFORMATION {
    ULONG ProcessorType;
    ULONG Reserved1;
    ULONG Reserved2;
} SYSTEM_PROCESSOR_INFORMATION, *PSYSTEM_PROCESSOR_INFORMATION;

typedef struct _SYSTEM_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleProcessTime;
    ULONG PageFaultCount;
    ULONG AvailablePages;
    ULONG PagedPoolPages;
    ULONG NonPagedPoolPages;
} SYSTEM_PERFORMANCE_INFORMATION, *PSYSTEM_PERFORMANCE_INFORMATION;

#define PROCESSOR_MIPS_R3000    3000

NTSYSAPI
NTSTATUS
NTAPI
NtQuerySystemInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );


//
// Global flags that can be set to control system behaviour
// Flag word is 32 bits.
//

#define FLG_SHOW_LDR_SNAPS              0x00000010
#define FLG_DISPLAY_LOAD_TIME           0x00000800


#endif // _NTEXAPI_
