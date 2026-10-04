/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989-1991  Microsoft Corporation

Module Name:

    ntldr.h

Abstract:

    This module implements the public interfaces of the Loader (Ldr)
    subsystem. Ldr is coupled with the session manager. It is not
    a seperate process.

Author:

    Mike O'Leary (mikeol) 22-Mar-1990

[Environment:]

    optional-environment-info (e.g. kernel mode only...)

[Notes:]

    optional-notes

Revision History:

--*/

#ifndef _NTLDRAPI_
#define _NTLDRAPI_

#include <ntdef.h>

//
// Private flags for loader data table entries
//

#define LDRP_ENTRY_INSERTED             0x00008000
#define LDRP_ENTRY_PROCESSED            0x00004000
#define LDRP_UNLOAD_IN_PROGRESS         0x00002000
#define LDRP_LOAD_IN_PROGRESS           0x00001000

//
// Loader Data Table. Used to track DLLs loaded into an
// image.
//

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    PVOID DllBase;
    HANDLE SectionHandle;
    STRING FullDllName;
    STRING BaseDllName;
    USHORT LoadCount;
    USHORT Flags;
    UCHAR Reserved2C[8];
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef
NTSTATUS
(*PDLL_INIT_ROUTINE) (
    IN PVOID DllHandle,
    IN BOOLEAN Initialize,
    IN PCONTEXT Context OPTIONAL
    );


VOID
NTAPI
LdrShutdownProcess(
    VOID
    );

NTSTATUS
NTAPI
LdrLoadDll(
    IN PSZ DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PSTRING DllName,
    OUT PVOID *DllHandle
    );

NTSTATUS
NTAPI
LdrGetDllHandle(
    IN PSZ DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PSTRING DllName,
    OUT PVOID *DllHandle
    );

NTSTATUS
NTAPI
LdrUnloadDll(
    IN PVOID DllHandle
    );

NTSTATUS
NTAPI
LdrGetProcedureAddress(
    IN PVOID DllHandle,
    IN PSTRING ProcedureName OPTIONAL,
    IN ULONG ProcedureNumber OPTIONAL,
    OUT PVOID *ProcedureAddress
    );

NTSTATUS
NTAPI
LdrFindResource(
    IN PVOID DllHandle,
    IN PULONG ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry
    );

NTSTATUS
NTAPI
LdrAccessResource(
    IN PVOID DllHandle,
    IN PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry,
    OUT PVOID *Address OPTIONAL,
    OUT PULONG Size OPTIONAL
    );

#endif
