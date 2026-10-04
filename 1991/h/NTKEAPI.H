/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ntkeapi.h

Abstract:

    This module contains the include file for data types that are exported
    by kernel for general use.

Author:

    David N. Cutler (davec) 27-Jul-1989

Environment:

    Any mode.

Revision History:

--*/

#ifndef _NTKEAPI_
#define _NTKEAPI_

#include <ntdef.h>

#define MAXIMUM_WAIT_OBJECTS 64     // Maximum number of wait objects

//
// Spin Lock
//

typedef ULONG *KSPIN_LOCK;
typedef KSPIN_LOCK *PKSPIN_LOCK;

//
// Interrupt routine (first level dispatch)
//

typedef
VOID
(*PKINTERRUPT_ROUTINE) (
    VOID
    );


#endif
