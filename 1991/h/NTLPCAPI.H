/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ntlpcapi.h

Abstract:

    This is the include file for the Local Procedure Call (LPC) sub-component
    of NTOS.

Author:

    Steve Wood (stevewo) 13-Mar-1989

Revision History:

--*/

#ifndef _NTLPCAPI_
#define _NTLPCAPI_

#include <ntpsapi.h>
#include <ntseapi.h>

typedef struct _PORT_MESSAGE {
    union {
        struct {
            CSHORT DataLength;
            CSHORT TotalLength;
        } s1;
        ULONG Length;
    } u1;
    union {
        struct {
            CSHORT Type;
            CSHORT DataInfoOffset;
        } s2;
        ULONG ZeroInit;
    } u2;
    CLIENT_ID ClientId;
    ULONG MessageId;
//  UCHAR Data[];
} PORT_MESSAGE, *PPORT_MESSAGE;

typedef struct _PORT_DATA_ENTRY {
    PVOID Base;
    ULONG Size;
} PORT_DATA_ENTRY, *PPORT_DATA_ENTRY;

typedef struct _PORT_DATA_INFORMATION {
    ULONG CountDataEntries;
    PORT_DATA_ENTRY DataEntries[1];
} PORT_DATA_INFORMATION, *PPORT_DATA_INFORMATION;

//
// Valid return values for the PORT_MESSAGE Type file
//

#define LPC_REQUEST             1
#define LPC_REPLY               2
#define LPC_DATAGRAM            3
#define LPC_CLIENT_DIED         6
#define LPC_EXCEPTION           7
#define LPC_DEBUG_EVENT         8

typedef struct _PORT_VIEW {
    ULONG Length;
    HANDLE SectionHandle;
    ULONG SectionOffset;
    ULONG ViewSize;
    PVOID ViewBase;
    PVOID ViewRemoteBase;
} PORT_VIEW, *PPORT_VIEW;

typedef struct _REMOTE_PORT_VIEW {
    ULONG Length;
    ULONG ViewSize;
    PVOID ViewBase;
} REMOTE_PORT_VIEW, *PREMOTE_PORT_VIEW;

typedef struct _LPCP_PORT_INFORMATION {
    ULONG Length;
    CLIENT_ID ClientId;
    ULONG RequestId;
    PVOID KernelContext;
    ULONG ViewSize;
} LPCP_PORT_INFORMATION;

NTSYSAPI
NTSTATUS
NTAPI
NtCreatePort(
    OUT PHANDLE PortHandle,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG MaxConnectionInfoLength,
    IN ULONG MaxMessageLength
    );

NTSYSAPI
NTSTATUS
NTAPI
NtConnectPort(
    OUT PHANDLE PortHandle,
    IN PSTRING PortName,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN PVOID KernelContext OPTIONAL,
    IN OUT PPORT_VIEW ClientView OPTIONAL,
    OUT PREMOTE_PORT_VIEW ServerView OPTIONAL,
    OUT PULONG MaxMessageLength OPTIONAL,
    IN OUT PVOID ConnectionInformation OPTIONAL,
    IN OUT PULONG ConnectionInfoLength OPTIONAL,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtListenPort(
    IN HANDLE PortHandle,
    OUT struct _LPCP_PORT_INFORMATION *Information,
    OUT PVOID ConnectionInformation OPTIONAL,
    IN OUT PULONG ConnectionInfoLength OPTIONAL,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtAcceptConnectPort(
    OUT PHANDLE PortHandle,
    IN PVOID PortContext,
    IN struct _LPCP_PORT_INFORMATION *Information,
    IN BOOLEAN AcceptConnection,
    IN BOOLEAN SeparateQueue,
    IN OUT PPORT_VIEW ClientView OPTIONAL,
    OUT PREMOTE_PORT_VIEW ServerView OPTIONAL,
    IN PVOID ConnectionInformation OPTIONAL,
    IN ULONG ConnectionInfoLength
    );

NTSYSAPI
NTSTATUS
NTAPI
NtCompleteConnectPort(
    IN HANDLE PortHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtRequestPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage
    );

NTSYSAPI
NTSTATUS
NTAPI
NtRequestWaitReplyPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtReplyPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE ReplyMessage,
    IN ULONG ReplyBoost
    );

NTSYSAPI
NTSTATUS
NTAPI
NtReplyWaitReplyPort(
    IN HANDLE PortHandle,
    IN OUT PPORT_MESSAGE ReplyMessage,
    IN ULONG ReplyBoost,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtReplyWaitReceivePort(
    IN HANDLE PortHandle,
    OUT PVOID *PortContext OPTIONAL,
    IN PPORT_MESSAGE ReplyMessage OPTIONAL,
    IN ULONG ReplyBoost,
    OUT PPORT_MESSAGE ReceiveMessage,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtImpersonateClientOfPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE Message
    );

#endif // _NTLPCAPI_
