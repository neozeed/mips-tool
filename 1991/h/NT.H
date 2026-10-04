/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989  Microsoft Corporation

Module Name:

    nt.h

Abstract:

    Top level include file for applications and subsystems written to
    the NT API.  Including this file defines all the public types
    and system calls that an application program can use.

    Initial clients of this include file are the NT Shell, along with
    the Windows, OS2 and POSIX subsystems.


Author:

    Steve Wood   [stevewo]   23-Feb-1989

Revision History:

--*/

#ifndef NT_INCLUDED
#define NT_INCLUDED

#include <crt/excpt.h>
#include <ntdef.h>

#include <ntstatus.h>
#include <ntkeapi.h>

#ifdef _MIPS_
#include <ntmips.h>
#endif // _MIPS_

#include <ntobapi.h>
#include <ntseapi.h>
#include <ntimage.h>
#include <ntldr.h>
#include <ntpsapi.h>
#include <ntxcapi.h>
#include <ntlpcapi.h>
#include <ntioapi.h>
#include <ntexapi.h>
#include <ntmmapi.h>

#endif // NT_INCLUDED
