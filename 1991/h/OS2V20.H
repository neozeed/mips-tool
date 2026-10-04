/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2v20.h

Abstract:

    This file contains the OS/2 V2.0 API calls exported by the OS/2 Subsystem

Author:

    Steve Wood (stevewo) 19-Sep-1989

Parameters:

    This header file uses the following symbols names to control how much
    of the header file is include.  They are:

        INCL_OS2V20_TASKING
        INCL_OS2V20_FILESYS
        INCL_OS2V20_FSD
        INCL_OS2V20_MEMORY
        INCL_OS2V20_SEMAPHORES
        INCL_OS2V20_TIMERS
        INCL_OS2V20_LOADER
        INCL_OS2V20_NLS
        INCL_OS2V20_EXCEPTIONS
        INCL_OS2V20_ERRORMSG
        INCL_OS2V20_SESSIONMGR
        INCL_OS2V20_DEVICE_SUPPORT
        INCL_OS2V20_PIPES
        INCL_OS2V20_QUEUES
        INCL_OS2V20_ERRORS

Revision History:

--*/

#ifndef _NTDEF__
#include <nt.h>
#endif // _NTDEF_

#ifdef FILE_CREATED
#undef FILE_CREATED // Fortunately NT and OS/2 2.0 both use 0x2 for this
#endif // FILE_CREATED

#ifdef INCL_OS2V20_ALL
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_FSD
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_TIMERS
#define INCL_OS2V20_LOADER
#define INCL_OS2V20_NLS
#define INCL_OS2V20_EXCEPTIONS
#define INCL_OS2V20_ERRORMSG
#define INCL_OS2V20_SESSIONMGR
#define INCL_OS2V20_DEVICE_SUPPORT
#define INCL_OS2V20_PIPES
#define INCL_OS2V20_QUEUES
#define INCL_OS2V20_ERRORS
#endif // INCL_OS2V20_ALL

#define APIRET ULONG
typedef APIRET *PAPIRET;

typedef HANDLE HMODULE, *PHMODULE;
typedef HANDLE PID, *PPID;
typedef HANDLE TID, *PTID;
typedef HANDLE HFILE, *PHFILE;
typedef HANDLE HDIR, *PHDIR;
typedef HANDLE HTIMER, *PHTIMER;
typedef HANDLE HQUEUE, *PHQUEUE;
typedef HANDLE HSEM, *PHSEM;
typedef HANDLE HMTX, *PHMTX;
typedef HANDLE HEV, *PHEV;
typedef HANDLE HMUX, *PHMUX;

typedef UCHAR BYTE, *PBYTE;

typedef ULONG BOOL32, *PBOOL32;

typedef int (*PFN)();

/* Tasking API Calls */

/* Process Information Block */


typedef struct _PIB {
    PID     ProcessId;              /* Process I.D. */
    PID     ParentProcessId;        /* Parent process I.D. */
    HMODULE ImageFileHandle;        /* Program (.EXE) module handle */
    PCHAR   CommandLine;            /* Command line pointer */
    PCHAR   Environment;            /* Environment pointer */
    ULONG   Status;                 /* Process Status */
    ULONG   Type;                   /* Process Type */
} PIB, *PPIB;

/* Process Type codes (Type field) */

#define PT_FULLSCREEN       0       /* Full screen app. */
#define PT_REALMODE         1       /* Real mode process */
#define PT_WINDOWABLEVIO    2       /* VIO windowable app. */
#define PT_PM               3       /* Presentation Manager app. */
#define PT_DETACHED         4       /* Detached app. */

/* Process Status Flag definitions (Status field) */

#define PS_EXITLIST         1       /* Thread is in exitlist routine */

typedef APIRET (*PFNPROCESS)( HMODULE ImageFileHandle, ULONG Reserved,
                               PCHAR Environment, PCHAR CommandLine );

typedef struct _OS2_TIB {
    ULONG   ThreadId;               /* OS/2 ID for the thread */
    ULONG   Priority;               /* Thread priority */
    ULONG   Version;                /* Version number for this structure */
    USHORT  MustCompleteCount;
} OS2_TIB, *POS2_TIB;

typedef APIRET (*PFNTHREAD)( ULONG Parameter );

typedef VOID (*PFNEXITLIST)( ULONG ExitReason );

typedef struct _RESULTCODES {
    ULONG ExitReason;
    ULONG ExitResult;
} RESULTCODES, *PRESULTCODES;

typedef struct _FTIME {
    USHORT twosecs : 5;
    USHORT minutes : 6;
    USHORT hours   : 5;
} FTIME, *PFTIME;

typedef struct _FDATE {
    USHORT day     : 5;
    USHORT month   : 4;
    USHORT year    : 7;
} FDATE, *PFDATE;

typedef struct _FDATETIME {
    FDATE date;
    FTIME time;
} FDATETIME, *PFDATETIME;

typedef struct _FILESTATUS {
    FDATE  fdateCreation;
    FTIME  ftimeCreation;
    FDATE  fdateLastAccess;
    FTIME  ftimeLastAccess;
    FDATE  fdateLastWrite;
    FTIME  ftimeLastWrite;
    ULONG  cbFile;
    ULONG  cbFileAlloc;
    ULONG  attrFile;
} FILESTATUS, *PFILESTATUS;

typedef struct _FILESTATUS2 {
    FDATE  fdateCreation;
    FTIME  ftimeCreation;
    FDATE  fdateLastAccess;
    FTIME  ftimeLastAccess;
    FDATE  fdateLastWrite;
    FTIME  ftimeLastWrite;
    ULONG  cbFile;
    ULONG  cbFileAlloc;
    ULONG  attrFile;
    ULONG  cbList;
} FILESTATUS2, *PFILESTATUS2;

/*
 * CCHMAXPATH is the maximum fully qualified path name length including
 * the drive letter, colon, backslashes and terminating NULL.
 */
#define CCHMAXPATH      260

/*
 * CCHMAXPATH is the maximum fully qualified path name length including
 * the drive letter, colon, backslashes and terminating NULL.
 */
#define CCHMAXCOMP      255

/*
 * CCHMAXPATHCOMP is the maximum individual path component name length
 * including a terminating NULL.
 */
#define CCHMAXPATHCOMP  256

typedef struct _FILEFINDBUF3 {
    ULONG   oNextEntryOffset;
    FDATE   fdateCreation;
    FTIME   ftimeCreation;
    FDATE   fdateLastAccess;
    FTIME   ftimeLastAccess;
    FDATE   fdateLastWrite;
    FTIME   ftimeLastWrite;
    ULONG   cbFile;
    ULONG   cbFileAlloc;
    ULONG   attrFile;
    UCHAR   cchName;
    CHAR    achName[CCHMAXPATHCOMP];
} FILEFINDBUF3, *PFILEFINDBUF3;

typedef struct _FILEFINDBUF4 {
    ULONG   oNextEntryOffset;
    FDATE   fdateCreation;
    FTIME   ftimeCreation;
    FDATE   fdateLastAccess;
    FTIME   ftimeLastAccess;
    FDATE   fdateLastWrite;
    FTIME   ftimeLastWrite;
    ULONG   cbFile;
    ULONG   cbFileAlloc;
    ULONG   attrFile;
    ULONG   cbList;
    UCHAR   cchName;
    CHAR    achName[CCHMAXPATHCOMP];
} FILEFINDBUF4, *PFILEFINDBUF4;

/* extended attribute structures */

typedef struct _GEA2 {
    ULONG oNextEntryOffset; /* offset to next GEA */
    BYTE cbName;            /* name length not including NULL */
    CHAR szName[1];         /* attribute name */
} GEA2, *PGEA2;

typedef struct _GEA2LIST {
    ULONG cbList;           /* total bytes of structure including full list */
    GEA2 list[1];           /* variable length GEA structures */
} GEA2LIST, *PGEA2LIST;

typedef struct _FEA2 {
    ULONG oNextEntryOffset; /* offset to next FEA */
    BYTE fEA;               /* flags                              */
    BYTE cbName;            /* name length not including NULL */
    USHORT cbValue;         /* value length */
    CHAR szName[1];         /* attribute name */
} FEA2, *PFEA2;

/* flags for _FEA.fEA */

#define FEA_NEEDEA 0x80     /* need EA bit */

typedef struct _FEA2LIST {
    ULONG cbList;            /* total bytes of structure including full list */
    FEA2 list[1];            /* variable length FEA structures */
} FEA2LIST, *PFEA2LIST;

typedef struct _EAOP2 {
    PGEA2LIST fpGEA2List;     /* general EA list */
    PFEA2LIST fpFEA2List;     /* full EA list */
    ULONG oError;
} EAOP2, *PEAOP2;

#define ULONG_MASK  ~3
#define RoundUpToUlong(number)  ((number+(sizeof(ULONG)-1)) & ULONG_MASK)

/*
 *  The following macros can be used to calculate the size of the current FEA
 *  and the offset of the next FEA
 *
 */

#define FEA_sizeof(fea)         (sizeof(FEA2)+(fea).cbName+1+(fea).cbValue)
#define FEA_oNextEntryOffset(fea)   (RoundUpToUlong(FEA_sizeof(fea))

/*
 *  The following macros can be used to access the fea_szName and
 *  fea_aValue fields in a given FEA.
 *
 */

#define     FEA_szNameFromFEA(fea)  (((PCHAR)&(fea))+sizeof(FEA2))
#define     FEA_AValueFromFEA(fea)  (((PCHAR)&(fea))+sizeof(FEA2)+(fea).cbName+1)

typedef struct _FILELOCK {
    LONG lOffset;
    LONG lRange;
} FILELOCK, *PFILELOCK;

typedef struct _FSQBUFFER {
    USHORT  iType;                      /* Item type */
    USHORT  cbName;                     /* Length of item name, sans NULL */
    UCHAR   szName[1];                  /* ASCIIZ item name */
    USHORT  cbFSDName;                  /* Length of FSD name, sans NULL */
    UCHAR   szFSDName[1];               /* ASCIIZ FSD name */
    USHORT  cbFSAData;                  /* Length of FSD Attach data returned */
    UCHAR   rgFSAData[1];               /* FSD Attach data from FSD */
} FSQBUFFER, *PFSQBUFFER;

typedef struct _PSEMRECORD {
    HSEM        hsemCur;
    ULONG       ulUser;
} SEMRECORD, *PSEMRECORD;

#define DC_SEM_NAMEPREFIX "\\SEM32\\"

#define DCMW_MAX_SEMRECORDS 64  /* DosCreateMuxWait limit on the           */
                                /* CountMuxWaitEntries parameter           */


typedef struct _DATETIME {
    UCHAR   hours;
    UCHAR   minutes;
    UCHAR   seconds;
    UCHAR   hundredths;
    UCHAR   day;
    UCHAR   month;
    USHORT  year;
    SHORT   timezone;
    UCHAR   weekday;
} DATETIME, *PDATETIME;

typedef struct _STARTDATA {
    USHORT  Length;
    USHORT  Related;
    USHORT  FgBg;
    USHORT  TraceOpt;
    PSZ     PgmTitle;
    PSZ     PgmName;
    PBYTE   PgmInputs;
    PBYTE   TermQ;
    PBYTE   Environment;
    USHORT  InheritOpt;
    USHORT  SessionType;
    PSZ     IconFile;
    ULONG   PgmHandle;
    USHORT  PgmControl;
    USHORT  InitXPos;
    USHORT  InitYPos;
    USHORT  InitXSize;
    USHORT  InitYSize;
    USHORT  Reserved;
    PSZ     ObjectBuffer;
    ULONG   ObjectBuffLen;
} STARTDATA, *PSTARTDATA;

#define SSF_RELATED_INDEPENDENT 0
#define SSF_RELATED_CHILD       1

#define SSF_FGBG_FORE           0
#define SSF_FGBG_BACK           1

#define SSF_TRACEOPT_NONE       0
#define SSF_TRACEOPT_TRACE      1
#define SSF_TRACEOPT_TRACEALL   2

#define SSF_INHERTOPT_SHELL     0
#define SSF_INHERTOPT_PARENT    1

/* note that these types are identical to those in pmshl.h for PROG_* */
#define SSF_TYPE_DEFAULT        0
#define SSF_TYPE_FULLSCREEN     1
#define SSF_TYPE_WINDOWABLEVIO  2
#define SSF_TYPE_PM             3
#define SSF_TYPE_VDM            4
#define SSF_TYPE_GROUP          5
#define SSF_TYPE_DLL            6
#define SSF_TYPE_WINDOWEDVDM    7
#define SSF_TYPE_PDD            8
#define SSF_TYPE_VDD            9

/* note that these flags are identical to those in pmshl.h for SHE_* */
#define SSF_CONTROL_VISIBLE     0x0000
#define SSF_CONTROL_INVISIBLE   0x0001
#define SSF_CONTROL_MAXIMIZE    0x0002
#define SSF_CONTROL_MINIMIZE    0x0004
#define SSF_CONTROL_NOAUTOCLOSE 0x0008
#define SSF_CONTROL_SETPOS      0x8000

#pragma pack(1)
typedef struct _STATUSDATA {
    USHORT Length;
    USHORT SelectInd;
    USHORT BondInd;
} STATUSDATA, *PSTATUSDATA;
#pragma pack()

/* STATUSDATA.SelectInd constants */

#define TARGET_UNCHANGED       0x0000
#define TARGET_SELECTABLE      0x0001
#define TARGET_NOT_SELECTABLE  0x0002

/* STATUSDATA.BondInd constants */

#define BOND_UNCHANGED  0x0000
#define BOND_CHILD      0x0001
#define BOND_NONE       0x0002

typedef struct _REQUESTDATA {
    PID SenderProcessId;
    ULONG SenderData;
} REQUESTDATA, *PREQUESTDATA;

#ifdef INCL_OS2V20_TASKING

APIRET
DosCreateThread(
    OUT PTID ThreadId,
    IN PFNTHREAD StartAddress,
    IN ULONG Parameter,
    IN ULONG Flags,
    IN ULONG StackSize
    );

/* Flags parameter values */

#define DCT_RUNABLE 0               // Create thread in a runable state
                                    // allocate low thread ID (1,2,..n)
#define DCT_SUSPENDED 1             // Create thread in a suspened state

#define DCT_ATTACHED 4              // Created from Win32 thread

APIRET
DosResumeThread(
    IN TID ThreadId
    );

APIRET
DosSuspendThread(
    IN TID ThreadId
    );

APIRET
DosGetThreadInfo(
    OUT PNT_TIB *ThreadInfo,
    OUT PPIB *ProcessInfo
    );

APIRET
DosGetProcessInfo(
    OUT PPIB *ProcessInfo
    );

VOID
DosExit(
    IN ULONG ExitAction,
    IN ULONG ExitResult
    );

#define EXIT_THREAD         0
#define EXIT_PROCESS        1

/* ExitReason values (also passed to ExitList routines) */

#define TC_EXIT          0
#define TC_HARDERROR     1
#define TC_TRAP          2
#define TC_KILLPROCESS   3

/* Wait target values */

#define DCWA_PROCESS        0
#define DCWA_PROCESSTREE    1

/* Wait option values */

#define DCWW_WAIT   0
#define DCWW_NOWAIT 1

APIRET
DosWaitChild(
    IN ULONG WaitTarget,
    IN ULONG WaitOption,
    OUT PRESULTCODES ResultCodes,
    OUT PPID ResultProcessId,
    IN PID ProcessId
    );

APIRET
DosWaitThread(
    IN OUT PTID ThreadId,
    IN ULONG WaitOption
    );

APIRET
DosEnterCritSec( VOID );

APIRET
DosExitCritSec( VOID );

APIRET
DosSleep(
    IN ULONG MilliSeconds
    );

APIRET
DosExecPgm(
    OUT PSZ ErrorText,
    IN LONG ErrorTextMaximumLength,
    IN ULONG Flags,
    IN PSZ Arguments OPTIONAL,
    IN PSZ Variables OPTIONAL,
    OUT PRESULTCODES ResultCodes,
    IN PSZ ImageFileName
    );

/* DosExecPgm Flags parameter values */

#define EXEC_SYNC           0
#define EXEC_ASYNC          1
#define EXEC_ASYNCRESULT    2
#define EXEC_TRACE          3
#define EXEC_BACKGROUND     5
#define EXEC_FROZEN         4
#define EXEC_TRACETREE      6

APIRET
DosExitList(
    ULONG OrderCode,
    PFNEXITLIST ExitRoutine
    );

/* DosExitList functions */

#define EXLST_ADD       1
#define EXLST_REMOVE    2
#define EXLST_EXIT      3

APIRET
DosKillProcess(
    IN ULONG KillTarget,
    IN PID ProcessId
    );

#define DKP_PROCESSTREE     0
#define DKP_PROCESS         1

APIRET
DosSetPriority(
    IN ULONG Scope,
    IN ULONG Class,
    IN LONG Delta,
    IN ULONG TargetId
    );

/* Priority scopes */

#define PRTYS_PROCESS       0
#define PRTYS_PROCESSTREE   1
#define PRTYS_THREAD        2

/* Priority classes */

#define PRTYC_NOCHANGE      0
#define PRTYC_IDLETIME      1
#define PRTYC_REGULAR       2
#define PRTYC_TIMECRITICAL  3
#define PRTYC_FOREGROUNDSERVER  4

/* Priority deltas */

#define PRTYD_MINIMUM      -31
#define PRTYD_MAXIMUM       31


APIRET
DosSetDefaultDisk(
    IN ULONG DiskNumber
    );

APIRET
DosQueryCurrentDisk(
    OUT PULONG DiskNumber,
    OUT PULONG LogicalDrives
    );

APIRET
DosSetCurrentDir(
    IN PSZ DirectoryName
    );

APIRET
DosQueryCurrentDir(
    IN ULONG DiskNumber,
    OUT PSZ DirectoryName,
    IN OUT PULONG DirectoryNameLength
    );

APIRET
DosQueryVerify(
    OUT PBOOL32 Verify
    );

APIRET
DosSetVerify(
    IN BOOL32 Verify
    );

APIRET
DosSetMaxFH(
    IN ULONG MaxFileHandles
    );

APIRET
DosScanEnv(
    IN PSZ VariableName,
    OUT PSZ *VariableValue
    );

APIRET
DosSearchPath(
    IN ULONG SearchFlags,
    IN PSZ PathOrVariableName,
    IN PSZ FileName,
    OUT PBYTE Buffer,
    IN ULONG Length
    );

/* DosSearchPath() constants */

#define SEARCH_PATH            0x0000
#define SEARCH_CUR_DIRECTORY   0x0001
#define SEARCH_ENVIRONMENT     0x0002
#define SEARCH_IGNORENETERRS   0x0004

/* definitions for DosSearchPath control word */
#define DSP_IMPLIEDCUR          1 /* current dir will be searched first */
#define DSP_PATHREF             2 /* from env.variable */
#define DSP_IGNORENETERR        4 /* ignore net errs & continue search */

#endif // INCL_OS2V20_TASKING


#ifdef INCL_OS2V20_FILESYS

/* File System API Calls */

/*** File System */

/* File info levels: All listed API's */
#define FIL_STANDARD            1       /* Info level 1, standard file info */
#define FIL_QUERYEASIZE         2       /* Level 2, return Full EA size */
#define FIL_QUERYEASFROMLIST    3       /* Level 3, return requested EA's */
#define FIL_QUERYALLEAS         4       /* Level 4, return all EAs NOT SUPPORTED */
/* File info levels: Dos32...PathInfo only */
#define FIL_QUERYFULLNAME       5       /* Level 5, return fully qualified
                                           name of file */
#define FIL_NAMEISVALID         6       /* Level 6, check validity of file/path
                                           name for this FSD */

#define FIL_SETEAS              2       /* Level 2, set EAs */

#define MAXQFILEINFOLEVEL       FIL_QUERYEASFROMLIST    // level 4 is invalid
#define MAXSETFILEINFOLEVEL     FIL_SETEAS
#define MAXQPATHINFOLEVEL       FIL_NAMEISVALID         // level 4 is invalid
#define MAXSETPATHINFOLEVEL     FIL_SETEAS
#define MAXFINDINFOLEVEL        FIL_QUERYEASFROMLIST

/* File time and date types */

/*
 * Equates for the types of EAs that follow the convention that we have
 * established.
 *
 * Values 0xFFFE thru 0x8000 are reserved.
 * Values 0x0000 thru 0x7fff are user definable.
 * Value  0xFFFC is not used
 */

#define EAT_BINARY      0xFFFE          /* length preceeded binary */
#define EAT_ASCII       0xFFFD          /* length preceeded ASCII */
#define EAT_BITMAP      0xFFFB          /* length preceeded bitmap */
#define EAT_METAFILE    0xFFFA          /* length preceeded metafile */
#define EAT_ICON        0xFFF9          /* length preceeded icon */
#define EAT_EA          0xFFEE          /* length preceeded ASCII */
                                        /* name of associated data (#include) */
#define EAT_MVMT        0xFFDF          /* multi-valued, multi-typed field */
#define EAT_MVST        0xFFDE          /* multi-valued, single-typed field */
#define EAT_ASN1        0xFFDD          /* ASN.1 field */


APIRET
DosOpen(
    IN PSZ FileName,
    OUT PHFILE FileHandle,
    OUT PULONG ActionTaken,
    IN ULONG CreateSize,
    IN ULONG FileAttribute,
    IN ULONG OpenFlags,
    IN ULONG OpenMode,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL
    );

/* DosOpen/DosQueryFHState/DosQueryFileInfo et al file attributes; also */
/* known as Dos File Mode bits... */
#define FILE_READONLY   0x0001
#define FILE_HIDDEN     0x0002
#define FILE_SYSTEM     0x0004
#define FILE_DIRECTORY  0x0010
#define FILE_ARCHIVED   0x0020

#define ATTR_CHANGEABLE (FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_ARCHIVED)
#define ATTR_ALL (FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_DIRECTORY | FILE_ARCHIVED)
#define ATTR_NOT_NORM   0x8000          // do not find normal files

/* DosOpen() actions */

#define FILE_EXISTED    0x0001
#define FILE_CREATED    0x0002
#define FILE_TRUNCATED  0x0003

/* DosOpen() open flags */
#define FILE_OPEN_EXISTING_FILE     0x0001
#define FILE_TRUNCATE_EXISTING_FILE 0x0002
#define FILE_CREATE_NEW_FILE        0x0010

/*     this nibble applies if file already exists                xxxx */

#define OPEN_ACTION_FAIL_IF_EXISTS     0x0000  /* ---- ---- ---- 0000 */
#define OPEN_ACTION_OPEN_IF_EXISTS     0x0001  /* ---- ---- ---- 0001 */
#define OPEN_ACTION_REPLACE_IF_EXISTS  0x0002  /* ---- ---- ---- 0010 */

/*     this nibble applies if file does not exist           xxxx      */

#define OPEN_ACTION_FAIL_IF_NEW        0x0000  /* ---- ---- 0000 ---- */
#define OPEN_ACTION_CREATE_IF_NEW      0x0010  /* ---- ---- 0001 ---- */

#define OPEN_ACTION_RESERVED         0xFFEC

/* DosOpen/DosSetFHState flags */

#define OPEN_ACCESS_READONLY        0x00000000  /* ---- ---- ---- -000 */
#define OPEN_ACCESS_WRITEONLY       0x00000001  /* ---- ---- ---- -001 */
#define OPEN_ACCESS_READWRITE       0x00000002  /* ---- ---- ---- -010 */
#define OPEN_SHARE_DENYREADWRITE    0x00000010  /* ---- ---- -001 ---- */
#define OPEN_SHARE_DENYWRITE        0x00000020  /* ---- ---- -010 ---- */
#define OPEN_SHARE_DENYREAD         0x00000030  /* ---- ---- -011 ---- */
#define OPEN_SHARE_DENYNONE         0x00000040  /* ---- ---- -100 ---- */
#define OPEN_FLAGS_NOINHERIT        0x00000080  /* ---- ---- 1--- ---- */
#define OPEN_FLAGS_NO_LOCALITY      0x00000000  /* ---- -000 ---- ---- */
#define OPEN_FLAGS_SEQUENTIAL       0x00000100  /* ---- -001 ---- ---- */
#define OPEN_FLAGS_RANDOM           0x00000200  /* ---- -010 ---- ---- */
#define OPEN_FLAGS_RANDOMSEQUENTIAL 0x00000300  /* ---- -011 ---- ---- */
#define OPEN_FLAGS_NO_CACHE         0x00001000  /* ---1 ---- ---- ---- */
#define OPEN_FLAGS_FAIL_ON_ERROR    0x00002000  /* --1- ---- ---- ---- */
#define OPEN_FLAGS_WRITE_THROUGH    0x00004000  /* -1-- ---- ---- ---- */
#define OPEN_FLAGS_DASD             0x00008000  /* 1--- ---- ---- ---- */

#define ACCESS_FLAGS (OPEN_ACCESS_READONLY | OPEN_ACCESS_WRITEONLY | \
                      OPEN_ACCESS_READWRITE)
#define SHARE_FLAGS (OPEN_SHARE_DENYREADWRITE | OPEN_SHARE_DENYWRITE | \
                     OPEN_SHARE_DENYREAD | OPEN_SHARE_DENYNONE)
#define QFHSTATE_FLAGS (OPEN_FLAGS_NOINHERIT | OPEN_FLAGS_WRITE_THROUGH | \
                        OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NO_CACHE |  \
                        ACCESS_FLAGS | SHARE_FLAGS)
#define SETFHSTATE_FLAGS (OPEN_FLAGS_NOINHERIT | OPEN_FLAGS_WRITE_THROUGH | \
                          OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NO_CACHE)
#define OPEN_RESERVED ~(OPEN_ACCESS_READONLY | OPEN_ACCESS_WRITEONLY | \
                         OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE | \
                         OPEN_SHARE_DENYWRITE | OPEN_SHARE_DENYREAD | \
                         OPEN_SHARE_DENYNONE | OPEN_FLAGS_NOINHERIT | \
                         OPEN_FLAGS_NO_LOCALITY | OPEN_FLAGS_SEQUENTIAL | \
                         OPEN_FLAGS_RANDOM | OPEN_FLAGS_RANDOMSEQUENTIAL | \
                         OPEN_FLAGS_NO_CACHE | OPEN_FLAGS_FAIL_ON_ERROR | \
                         OPEN_FLAGS_WRITE_THROUGH | OPEN_FLAGS_DASD)


#define LOCALITY_FLAGS (OPEN_FLAGS_NO_LOCALITY | OPEN_FLAGS_SEQUENTIAL | \
                        OPEN_FLAGS_RANDOM | OPEN_FLAGS_RANDOMSEQUENTIAL)

APIRET
DosClose(
    IN HFILE FileHandle
    );

APIRET
DosRead(
    IN HFILE FileHandle,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    );

APIRET
DosWrite(
    IN HFILE FileHandle,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

APIRET
DosDelete(
    IN PSZ FileName
    );


APIRET
DosDupHandle(
    IN HFILE OldFileHandle,
    IN OUT PHFILE NewFileHandle
    );

#define DDH_NEW_HANDLE -1

APIRET
DosQueryFHState(
    IN HFILE FileHandle,
    OUT PULONG OpenMode
    );

APIRET
DosSetFHState(
    IN HFILE FileHandle,
    IN ULONG OpenMode
    );

APIRET
DosQueryHType(
    IN HFILE FileHandle,
    OUT PULONG HandleType,
    OUT PULONG DeviceFlags
    );

/* DosQueryHType() */
/* Handle classes (low 8 bits of Handle Type) */
#define FHT_DISKFILE    0x0000          /* Disk file handle */
#define FHT_CHRDEV      0x0001          /* Character device handle */
#define FHT_PIPE        0x0002          /* Pipe handle */

/* Handle bits (high 8 bits of Handle Type) */
#define FHB_DSKREMOTE           0x8000  /* Remote disk */
#define FHB_CHRDEVREMOTE        0x8000  /* Remote character device */
#define FHB_PIPEREMOTE          0x8000  /* Remote pipe */

#define HANDTYPE_FILE     0x0000
#define HANDTYPE_DEVICE   0x0001
#define HANDTYPE_PIPE     0x0002
#define HANDTYPE_NETWORK  0x8000

APIRET
DosFindFirst(
    IN PSZ FileName,
    IN OUT PHDIR DirectoryHandle,
    IN ULONG FileAttributes,
    IN PFILEFINDBUF3 Buffer,
    IN ULONG Length,
    IN OUT PULONG CountEntriesFound,
    IN ULONG FileInformationLevel
    );

APIRET
DosFindNext(
    IN HDIR DirectoryHandle,
    IN PFILEFINDBUF3 Buffer,
    IN ULONG Length,
    IN OUT PULONG CountEntriesFound
    );

/* DosFindFirst/Next Directory handle types */

#define HDIR_SYSTEM  1          /* Use system handle (1) */
#define HDIR_CREATE  -1         /* Allocate a new, unused handle */

APIRET
DosFindClose(
    IN HDIR DirectoryHandle
    );

APIRET
DosSetFileSize(
    IN HFILE FileHandle,
    IN ULONG NewFileSize
    );

APIRET
DosResetBuffer(
    IN HFILE FileHandle
    );

#define FLUSH_ALL     -1      // Flush buffers for all of process's handles

APIRET
DosSetFilePtr(
    IN HFILE FileHandle,
    IN LONG StartingFilePosition,
    IN ULONG NewFilePosition,
    IN OUT PULONG CurrentFilePosition
    );

/* DosSetFilePtr() file position codes */

#define FILE_BEGIN      0x0000  /* Move relative to beginning of file */
#define FILE_CURRENT    0x0001  /* Move relative to current fptr position */
#define FILE_END        0x0002  /* Move relative to end of file */

APIRET
DosSetFileLocks(
    IN HFILE FileHandle,
    IN PFILELOCK UnlockRequest,
    IN PFILELOCK LockRequest
    );

APIRET
DosMove(
    IN PSZ OldFileName,
    IN PSZ NewFileName
    );

APIRET
DosCopy(
    IN PSZ OldFileName,
    IN PSZ NewFileName,
    IN ULONG CopyOption
    );

/* DosCopy option bits; may be or'ed together */

#define DCPY_EXISTING   0x00001         /* Copy even if target exists */
#define DCPY_APPEND     0x00002         /* Append to existing file, do not replace */
#define DCPY_EA_FAIL_COPY 0x0004        /* fail copy if EAs not supported */
                                        /* by destination file system */
#define DCPY_ALL (DCPY_EXISTING | DCPY_APPEND | DCPY_EA_FAIL_COPY)


APIRET
DosEditName(
    IN ULONG EditLevel,
    IN PSZ SourceString,
    IN PSZ EditString,
    OUT PBYTE Buffer,
    IN ULONG Length
    );

#define EDIT_LEVEL_ONE  0x0001          // use OS2 v1.2 editing semantics
#define MAX_EDIT_LEVEL  EDIT_LEVEL_ONE

APIRET
DosFileIO(
    IN HFILE FileHandle,
    IN PBYTE CommandBuffer,
    IN ULONG Length,
    OUT PUSHORT ErrorOffset
    );

/* DosFileIO command words */

#define FIO_LOCK        0       /* Lock Files */
#define FIO_UNLOCK      1       /* Unlock Files */
#define FIO_SEEK        2       /* Seek (set file ptr) */
#define FIO_READ        3       /* File Read */
#define FIO_WRITE       4       /* File Write */

/* Lock Sharing Modes */
#define FIO_NOSHARE     0       /* None */
#define FIO_SHAREREAD   1       /* Read-Only */

typedef struct _FIOLOCKCMD {
    USHORT  usCmd;                      /* Cmd = FIO_LOCK */
    USHORT  cLockCnt;                   /* Lock records that follow */
    ULONG   cTimeOut;                   /* in Msec */
} FIOLOCKCMD, *PFIOLOCKCMD;


typedef struct _FIOLOCKREC {
    USHORT  fShare;                     /* FIO_NOSHARE or FIO_SHAREREAD */
    ULONG   cbStart;                    /* Starting offset for lock region */
    ULONG   cbLength;                   /* Length of lock region */
} FIOLOCKREC, *PIOLOCKREC;


typedef struct _FIOUNLOCKCMD {
    USHORT  usCmd;                      /* Cmd = FIO_UNLOCK */
    USHORT  cUnlockCnt;                 /* Unlock records that follow */
} FIOUNLOCKCMD, *PFIOUNLOCKCMD;


typedef struct _FIOUNLOCKREC {
    ULONG   cbStart;                    /* Starting offset for unlock region */
    ULONG   cbLength;                   /* Length of unlock region */
} FIOUNLOCKREC, *PFIOUNLOCKREC;


typedef struct _FIOSEEKCMD {
    USHORT  usCmd;                      /* Cmd = FIO_SEEK */
    USHORT  fsMethod;                   /* One of: FPM_BEGINNING, FPM_CURRENT,
                                           or FPM_END */
    ULONG   cbDistance;                 /* Byte offset for seek */
    ULONG   cbNewPosition;              /* Bytes from start of file after
                                           seek */
} FIOSEEKCMD, *PFIOSEEKCMD;


typedef struct _FIOREADWRITE {
    USHORT  usCmd;                      /* Cmd = FIO_READ or FIO_WRITE */
    PVOID   pbBuffer;                   /* Pointer to data buffer */
    ULONG   cbBufferLen;                /* Bytes in buffer or max size */
    ULONG   cbActualLen;                /* Bytes actually read/written */
} FIOREADWRITE, *PFIOREADWRITE;



APIRET
DosCreateDir(
    IN PSZ DirectoryName,
    IN PEAOP2 DirectoryAttributes OPTIONAL
    );

APIRET
DosDeleteDir(
    IN PSZ DirectoryName
    );

APIRET
DosQueryFileInfo(
    IN HFILE FileHandle,
    IN ULONG FileInformationLevel,
    OUT PBYTE Buffer,
    IN ULONG Length
    );

APIRET
DosSetFileInfo(
    IN HFILE FileHandle,
    IN ULONG FileInformationLevel,
    IN OUT PBYTE Buffer,
    IN ULONG Length
    );

APIRET
DosQueryPathInfo(
    IN PSZ pszPath,
    IN ULONG FileInformationLevel,
    OUT PBYTE Buffer,
    IN ULONG Length
    );

APIRET
DosSetPathInfo(
    IN PSZ pszPath,
    IN ULONG FileInformationLevel,
    IN OUT PBYTE Buffer,
    IN ULONG Length,
    IN ULONG Flags
    );

/* defines for DosSetPathInfo Flags parameter values */
#define DSPI_WRTTHRU    0x10    /* write through */

APIRET
DosEnumAttribute(
    IN ULONG RefType,
    IN PVOID FileRef,
    IN ULONG EntryNum,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN OUT PULONG ActualLength,
    IN ULONG FileInformationLevel
    );

#define ENUM_EANAME     1       // DosEnumAttribute infolevel 1

/* Infolevels for DosEnumAttribute  */
#define ENUMEA_LEVEL_NO_VALUE   1       /* FEA without value */

/* Reference types for DosEnumAttribute */
#define ENUMEA_REFTYPE_FHANDLE  0       /* file handle */
#define ENUMEA_REFTYPE_PATH     1       /* path name */
#define ENUMEA_REFTYPE_MAX      ENUMEA_REFTYPE_PATH

/* level 1 info returned from DosEnumAttribute */

typedef struct _DENA1 {
    ULONG  oNextEntryOffset; /* offset to next EA */
    UCHAR  fEA;              /* flags */
    UCHAR  cbName;           /* length of name excluding NULL */
    USHORT cbValue;          /* length of value */
    UCHAR  szName[1];         /* variable length asciiz name */
} DENA1, *PDENA1;

#endif // INCL_OS2V20_FILESYS

#ifdef INCL_OS2V20_FSD

/* FSD API Calls */


APIRET
DosFSAttach(
    IN PSZ DeviceName,
    IN PSZ FsName,
    IN PBYTE FsData,
    IN ULONG FsDataLength,
    IN ULONG AttachFlags
    );

/* Dos32FsAttach() */
/* Attact or detach */
#define FS_ATTACH       0       /* Attach file server */
#define FS_DETACH       1       /* Detach file server */


APIRET
DosQueryFSAttach(
    IN PSZ DeviceName,
    IN ULONG Ordinal,
    IN ULONG FsInformationLevel,
    OUT PBYTE FsAttributes,
    IN OUT PULONG FsAttributesLength
    );

/* DosQueryFSAttach() */
/* Information level types (defines method of query) */
#define FSAIL_QUERYNAME 1       /* Return data for a Drive or Device */
#define FSAIL_DEVNUMBER 2       /* Return data for Ordinal Device # */
#define FSAIL_DRVNUMBER 3       /* Return data for Ordinal Drive # */

/* Item types (from data structure item "iType") */
#define FSAT_CHARDEV    1       /* Resident character device */
#define FSAT_PSEUDODEV  2       /* Pusedu-character device */
#define FSAT_LOCALDRV   3       /* Local drive */
#define FSAT_REMOTEDRV  4       /* Remote drive attached to FSD */

typedef struct  _FSQBUFFER2 {           /* Data structure for QFSAttach */
        USHORT  iType;                  /* Item type */
        USHORT  cbName;                 /* Length of item name, sans NULL */
        USHORT  cbFSDName;              /* Length of FSD name, sans NULL */
        USHORT  cbFSAData;              /* Length of FSD Attach data returned */
        UCHAR   szName[1];              /* ASCIIZ item name */
        UCHAR   szFSDName[1];           /* ASCIIZ FSD name */
        UCHAR   rgFSAData[1];           /* FSD Attach data from FSD */
} FSQBUFFER2, *PFSQBUFFER2;

APIRET
DosFSCtl(
    IN PBYTE Data,
    IN ULONG DataLength,
    OUT PULONG ActualDataLength,
    IN PBYTE Parameters,
    IN ULONG ParametersLength,
    OUT PULONG ActualParametersLength,
    IN ULONG Function,
    IN PSZ RouteName,
    IN HFILE FileHandle,
    IN ULONG RoutingMethod
    );

/* Dos32FsCtl() */
/* Routing type */
#define FSCTL_HANDLE    1       /* File Handle directs req routing */
#define FSCTL_PATHNAME  2       /* Path Name directs req routing */
#define FSCTL_FSDNAME   3       /* FSD Name directs req routing */


APIRET
DosQueryFSInfo(
    IN ULONG DiskNumber,
    IN ULONG FsInformationLevel,
    IN PBYTE Buffer,
    IN ULONG Length
    );

APIRET
DosSetFSInfo(
    ULONG DiskNumber,
    ULONG FsInformationLevel,
    PBYTE Buffer,
    ULONG Length
    );

/* FS Drive Info Levels */
#define FSIL_ALLOC      1       /* Drive allocation info (Query only) */
#define FSIL_VOLSER     2       /* Drive Volum/Serial info */

typedef struct _FSALLOCATE {
    ULONG  ulReserved;
    ULONG  cSectorUnit;
    ULONG  cUnit;
    ULONG  cUnitAvail;
    USHORT cbSector;
} FSALLOCATE, *PFSALLOCATE;

#define MAX_LABEL_LENGTH 11     // maximum length of volume label

typedef struct _VOLUMELABEL {
    BYTE cch;
    CHAR szVolLabel[MAX_LABEL_LENGTH+1];
} VOLUMELABEL, *PVOLUMELABEL;

typedef struct _FSINFO {
    ULONG ulVSN;
    VOLUMELABEL vol;
} FSINFO, *PFSINFO;

#endif // INCL_OS2V20_FSD

#ifdef INCL_OS2V20_MEMORY

/*** Memory management API Calls */

APIRET
DosAllocMem(
    OUT PVOID *BaseAddress,
    IN ULONG RegionSize,
    IN ULONG Flags
    );

APIRET
DosFreeMem(
    PVOID BaseAddress
    );

APIRET
DosSetMem(
    IN PVOID BaseAddress,
    IN ULONG RegionSize,
    IN ULONG Flags
    );

APIRET
DosGiveSharedMem(
    IN PVOID BaseAddress,
    IN PID ProcessId,
    IN ULONG Flags
    );

APIRET
DosGetSharedMem(
    IN PVOID BaseAddress,
    IN ULONG Flags
    );

APIRET
DosGetNamedSharedMem(
    OUT PVOID *BaseAddress,
    IN PSZ ObjectName,
    IN ULONG Flags
    );

APIRET
DosAllocSharedMem(
    OUT PVOID *BaseAddress,
    IN PSZ ObjectName,
    IN ULONG RegionSize,
    IN ULONG Flags
    );

APIRET
DosQueryMem(
    IN PVOID BaseAddress,
    IN OUT PULONG RegionSize,
    OUT PULONG Flags
    );

#define DA_SHAREMEM_NAMEPREFIX "\\SHAREMEM\\"

/* Access protection */
#define PAG_READ        0x00000001      /* read access */
#define PAG_WRITE       0x00000002      /* write access */
#define PAG_EXECUTE     0x00000004      /* execute access */
#define PAG_GUARD       0x00000008      /* guard protection */
#define PAG_DEFAULT     0x00000400      /* default (initial) access */

/* Commit */
#define PAG_COMMIT      0x00000010      /* commit storage */
#define PAG_DECOMMIT    0x00000020      /* decommit storage */

/* Allocation attributes */
#define OBJ_TILE        0x00000040      /* tile object */
#define OBJ_GETTABLE    0x00000100      /* gettable by other processes */
#define OBJ_GIVEABLE    0x00000200      /* giveable to other processes */

/* Allocation type (returned from DosQueryMem) */
#define PAG_SHARED      0x00002000      /* shared object */
#define PAG_FREE        0x00004000      /* pages are free */
#define PAG_BASE        0x00010000      /* first page in object */

#define fPERM           (PAG_EXECUTE + PAG_READ + PAG_WRITE + PAG_GUARD)
#define fSHARE          (OBJ_GETTABLE + OBJ_GIVEABLE)

/* DosAllocMem flags */
#define fALLOC          (OBJ_TILE + PAG_COMMIT + fPERM)

/* DosAllocSharedMem flags */
#define fALLOCSHR       (OBJ_TILE + PAG_COMMIT + fSHARE + fPERM)

/* DosGetNamedSharedMem flags */
#define fGETNMSHR       (fPERM)

/* DosGetSharedMem flags */
#define fGETSHR         (fPERM)

/* DosGiveSharedMem flags */
#define fGIVESHR        (fPERM)

/* DosSetMem flags */
#define fSET            (OBJ_TILE + PAG_COMMIT + PAG_DECOMMIT + PAG_DEFAULT + fPERM)

#ifdef NOT_IMPLEMENTED

#define OBJ_PROTECTED   0x00000080      /* protect object
                                           NOTE: This flag is NOT available at
                                           the api level */
#define PAG_PRIVATE     0x00001000      /* private object */

APIRET
DosAliasMem(
    PVOID pb,
    ULONG cb,
    PVOID *ppbAlias
    );

APIRET
DosQueryMemState(
    IN PVOID BaseAddress,
    IN OUT PULONG RegionSize,
    OUT PULONG Flags
    );

/* Page state (returned from DosQueryMemState) */
#define PAG_NPOUT       0x00000000      /* page is not present, not in core */
#define PAG_PRESENT     0x00000001      /* page is present */
#define PAG_NPIN        0x00000002      /* page is not present, in core */
#define PAG_PRESMASK    0x00000003      /* present state mask */
#define PAG_INVALID     0x00000000      /* page is invalid */
#define PAG_RESIDENT    0x00000010      /* page is resident */
#define PAG_SWAPPABLE   0x00000020      /* page is swappable */
#define PAG_DISCARDABLE 0x00000030      /* page is discardable */
#define PAG_TYPEMASK    0x00000030      /* type mask */


APIRET
DosQueryMemState(
    PVOID pb,
    PULONG cb,
    PULONG pFlag
    );

#endif // NOT_IMPLEMENTED


APIRET
DosSubSet(
    IN PVOID HeapAddress,
    IN ULONG Flags,
    IN ULONG NewSize
    );

/* DosSubSet flags */
#define DOSSUB_INIT             0x01    /* initialize memory object for */
                                        /* suballocation                */
#define DOSSUB_GROW             0x02    /* increase size of memory pool */
                                        /* for suballocation            */
#define DOSSUB_SPARSE_OBJ       0x04    /* indicator for DosSub to      */
                                        /* manage the commitment of     */
                                        /* pages spanned by the memory  */
                                        /* pool                         */
#define DOSSUB_SERIALIZE        0x08    /* indicates that access to the */
                                        /* memory pool is to be         */
                                        /* serialized by DosSub         */


APIRET
DosSubUnset(
    IN PVOID HeapAddress
    );


APIRET
DosSubAlloc(
    IN PVOID HeapAddress,
    OUT PVOID *BaseAddress,
    IN ULONG Size
    );

APIRET
DosSubFree(
    IN PVOID HeapAddress,
    IN PVOID BaseAddress,
    IN ULONG Size
    );

#endif // INCL_OS2V20_MEMORY

#ifdef INCL_OS2V20_SEMAPHORES

/* Semaphore API Calls */


/* Semaphore CreateAttributes */

#define DC_SEM_MAXNAMEL  256

#define DC_SEM_SHARED   0x01    /* DosCreateMutex, DosCreateEvent, and     */
                                /*   DosCreateMuxWait use it to indicate   */
                                /*   whether the semaphore is shared or    */
                                /*   private when the PSZ is null          */
#define DCMW_WAIT_ANY   0x02    /* DosCreateMuxWait option for wait on any */
                                /*   event/mutex to occur                  */
#define DCMW_WAIT_ALL   0x04    /* DosCreateMuxWait option for wait on all */
                                /*   events/mutexs to occur                */

#define SEM_INDEFINITE_WAIT     -1L
#define SEM_IMMEDIATE_RETURN     0L

APIRET
DosCreateEventSem(
    IN PSZ ObjectName,
    OUT PHEV EventHandle,
    IN ULONG CreateAttributes,
    IN BOOL32 InitialState
    );

APIRET
DosOpenEventSem(
    IN PSZ ObjectName,
    IN OUT PHEV EventHandle
    );

APIRET
DosCloseEventSem(
    IN HEV EventHandle
    );

APIRET
DosResetEventSem(
    IN HEV EventHandle,
    OUT PULONG PostCount
    );

APIRET
DosPostEventSem(
    IN HEV EventHandle
    );

APIRET
DosWaitEventSem(
    IN HEV EventHandle,
    IN ULONG Timeout
    );

APIRET
DosQueryEventSem(
    IN HEV EventHandle,
    OUT PULONG PostCount
    );

APIRET
DosCreateMutexSem(
    IN PSZ ObjectName,
    OUT PHMTX MutexHandle,
    IN ULONG CreateAttributes,
    IN BOOL32 InitialState
    );

APIRET
DosOpenMutexSem(
    IN PSZ ObjectName,
    IN OUT PHMTX MutexHandle
    );

APIRET
DosCloseMutexSem(
    IN HMTX MutexHandle
    );

APIRET
DosRequestMutexSem(
    IN HMTX MutexHandle,
    IN ULONG Timeout
    );

APIRET
DosReleaseMutexSem(
    IN HMTX MutexHandle
    );

APIRET
DosQueryMutexSem(
    IN HMTX MutexHandle,
    OUT PPID OwnerPid,
    OUT PTID OwnerTid,
    OUT PULONG OwnerRequestLevel
    );

APIRET
DosCreateMuxWaitSem(
    IN PSZ ObjectName,
    OUT PHMUX MuxWaitHandle,
    IN ULONG CountMuxWaitEntries,
    IN SEMRECORD MuxWaitEntries[],
    IN ULONG CreateAttributes
    );

APIRET
DosOpenMuxWaitSem(
    IN PSZ ObjectName,
    IN OUT PHMUX MuxWaitHandle
    );

APIRET
DosCloseMuxWaitSem(
    IN HMUX MuxWaitHandle
    );

APIRET
DosWaitMuxWaitSem(
    IN HMUX MuxWaitHandle,
    IN ULONG Timeout,
    OUT PULONG UserValue
    );

APIRET
DosAddMuxWaitSem(
    IN HMUX MuxWaitHandle,
    IN PSEMRECORD MuxWaitEntry
    );

APIRET
DosDeleteMuxWaitSem(
    IN HMUX MuxWaitHandle,
    IN HSEM MuxWaitEntrySem
    );

APIRET
DosQueryMuxWaitSem(
    IN HMUX MuxWaitHandle,
    IN OUT PULONG CountMuxWaitEntries,
    OUT SEMRECORD MuxWaitEntries[],
    OUT PULONG CreateAttributes
    );

#endif // INCL_OS2V20_SEMAPHORES

#ifdef INCL_OS2V20_TIMERS

/*** Timer Services API Calls */

APIRET
DosGetDateTime(
    OUT PDATETIME DateTime
    );

APIRET
DosSetDateTime(
    IN PDATETIME DateTime
    );


APIRET
DosAsyncTimer(
    IN ULONG MilliSeconds,
    IN HEV EventSem,
    OUT PHTIMER TimerHandle
    );

APIRET
DosStartTimer(
    IN ULONG MilliSeconds,
    IN HEV EventSem,
    OUT PHTIMER TimerHandle
    );

APIRET
DosStopTimer(
    IN HTIMER TimerHandle
    );

#endif // INCL_OS2V20_TIMERS


#ifdef INCL_OS2V20_LOADER

/*** Loader API Calls */

APIRET
DosLoadModule(
    OUT PSZ ErrorText,
    IN ULONG ErrorTextLength,
    IN PSZ ModuleName,
    OUT PHMODULE ModuleHandle
    );

APIRET
DosFreeModule(
    IN HMODULE ModuleHandle
    );

APIRET
DosQueryProcAddr(
    IN HMODULE ModuleHandle,
    IN ULONG ProcOrdinal,
    IN PSZ ProcName OPTIONAL,
    OUT PFN *ProcAddress
    );

APIRET
DosQueryProcType(
    IN HMODULE ModuleHandle,
    IN ULONG ProcOrdinal,
    IN PSZ ProcName OPTIONAL,
    OUT PULONG *ProcType
    );

#define PT_32BIT 0


APIRET
DosQueryModuleHandle(
    IN PSZ ModuleName,
    OUT PHMODULE ModuleHandle
    );

APIRET
DosQueryModuleName(
    IN HMODULE ModuleHandle,
    IN ULONG ModuleNameLength,
    OUT PSZ ModuleName
    );

APIRET
DosGetResource(
    IN HMODULE ModuleHandle,
    IN ULONG ResourceTypeId,
    IN ULONG ResourceNameId,
    OUT PVOID *ResourceBaseAddress
    );

APIRET
DosQueryResourceSize(
    IN HMODULE ModuleHandle,
    IN ULONG ResourceTypeId,
    IN ULONG ResourceNameId,
    OUT PULONG ResourceSize
    );


/* Predefined resource types */

#define RT_POINTER      1   /* mouse pointer shape */
#define RT_BITMAP       2   /* bitmap */
#define RT_MENU         3   /* menu template */
#define RT_DIALOG       4   /* dialog template */
#define RT_STRING       5   /* string tables */
#define RT_FONTDIR      6   /* font directory */
#define RT_FONT         7   /* font */
#define RT_ACCELTABLE   8   /* accelerator tables */
#define RT_RCDATA       9   /* binary data */
#define RT_MESSAGE      10  /* error msg     tables */
#define RT_DLGINCLUDE   11  /* dialog include file name */
#define RT_VKEYTBL      12  /* key to vkey tables */
#define RT_KEYTBL       13  /* key to UGL tables */
#define RT_CHARTBL      14  /* glyph to character tables */
#define RT_DISPLAYINFO  15  /* screen display information */

#define RT_FKASHORT     16  /* function key area short form */
#define RT_FKALONG      17  /* function key area long form */

#define RT_HELPTABLE    18  /* Help table for Cary Help manager */
#define RT_HELPSUBTABLE 19  /* Help subtable for Cary Help manager */

#define RT_FDDIR        20  /* DBCS uniq/font driver directory */
#define RT_FD           21  /* DBCS uniq/font driver */

#define RT_MAX          22  /* 1st unused Resource Type */


#define RF_ORDINALID    0x80000000L     /* ordinal id flag in resource table */


APIRET
DosQueryAppType(
    IN PSZ ImageFileName,
    OUT PULONG AppTypeFlags
    );

/* AppType returned in AppTypeFlags is defined as follows               */

#define FAPPTYP_NOTSPEC         0x0000
#define FAPPTYP_NOTWINDOWCOMPAT 0x0001
#define FAPPTYP_WINDOWCOMPAT    0x0002
#define FAPPTYP_WINDOWAPI       0x0003
#define FAPPTYP_BOUND           0x0008
#define FAPPTYP_DLL             0x0010
#define FAPPTYP_DOS             0x0020
#define FAPPTYP_PHYSDRV         0x0040  /* physical device driver       */
#define FAPPTYP_VIRTDRV         0x0080  /* virtual device driver        */
#define FAPPTYP_PROTDLL         0x0100  /* 'protected memory' dll       */

#endif // INCL_OS2V20_LOADER


#ifdef INCL_OS2V20_NLS

typedef struct _COUNTRYCODE {
    ULONG country;
    ULONG codepage;
} COUNTRYCODE, *PCOUNTRYCODE;

typedef struct _COUNTRYINFO {
    ULONG country;
    ULONG codepage;
    ULONG fsDateFmt;
    CHAR  szCurrency[5];
    CHAR  szThousandsSeparator[2];
    CHAR  szDecimal[2];
    CHAR  szDateSeparator[2];
    CHAR  szTimeSeparator[2];
    UCHAR fsCurrencyFmt;
    UCHAR cDecimalPlace;
    UCHAR fsTimeFmt;
    USHORT abReserved1[2];
    CHAR  szDataSeparator[2];
    USHORT abReserved2[5];
} COUNTRYINFO, *PCOUNTRYINFO;

APIRET
DosQueryCtryInfo(
    IN   ULONG         MaxCountryInfoLength,
    IN   PCOUNTRYCODE  CountryCode,
    OUT  PCOUNTRYINFO  CountryInfo,
    OUT  PULONG        ActualCountryInfoLength
    );

APIRET
DosQueryDBCSEnv(
    IN   ULONG        MaxDBCSEvLength,
    IN   PCOUNTRYCODE CountryCode,
    OUT  PCHAR        DBCSEv
    );

APIRET
DosMapCase(
    IN  USHORT       StringLength,
    IN  PCOUNTRYCODE CountryCode,
    IN OUT PUCHAR     String
    );

APIRET
DosQueryCollate(
    IN   ULONG   MaxCollateInfoLength,
    IN   PCOUNTRYCODE CountryCode,
    OUT  PUCHAR  CollateInfo,
    OUT  PULONG  ActualCollateInfoLength
    );

APIRET
DosQueryCp(
    IN   ULONG   MaxLengthCodePageList,
    OUT  ULONG   CodePages[],
    OUT  PULONG  CountCodePages
    );

APIRET
DosSetProcessCp(
    IN  ULONG  ulCodePage
    );

#endif // INCL_OS2V20_NLS


#ifdef INCL_OS2V20_EXCEPTIONS

/*** Exception API Calls */

/*
 * User Exception Handler Return Codes:
 */

#define XCPT_CONTINUE_SEARCH    0x00000000      /* exception not handled */
#define XCPT_CONTINUE_EXECUTION 0xFFFFFFFF      /* exception handled /*

/*
 * fHandlerFlags values (see ExceptionStructure):
 *
 * The user may only set (but not clear) the EH_NONCONTINUABLE flag.
 * All other flags are set by the system.
 *
 */

#define EH_NONCONTINUABLE 0x1           /* Noncontinuable exception */
#define EH_UNWINDING 0x2                /* Unwind is in progress */
#define EH_EXIT_UNWIND 0x4              /* Exit unwind is in progress */
#define EH_STACK_INVALID 0x8            /* Stack out of limits or unaligned */
#define EH_NESTED_CALL 0x10             /* Nested exception handler call */


/*
 * Unwind all exception handlers (see DosUnwindException API)
 */

#define UNWIND_ALL              0

/*
 *   Exception values are 32 bit values layed out as follows:
 *
 *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 *   +---+-+-------------------------+-------------------------------+
 *   |Sev|C|       Facility          |               Code            |
 *   +---+-+-------------------------+-------------------------------+
 *
 *   where
 *
 *       Sev - is the severity code
 *           00 - Success
 *           01 - Informational
 *           10 - Warning
 *           11 - Error
 *
 *       C - is the Customer code flag
 *
 *       Facility - is the facility code
 *
 *       Code - is the facility's status code
 *
 *   Exceptions specific to Cruiser (e.g. XCPT_SIGNAL) will be marked
 *   with a facility code of 1.
 *
 *   System defined exceptions have a facility code of zero.
 *
 *   Each exception may also have several pieces of additional information.
 *   These are stored in the ExceptionInformation fields of the
 *   ExceptionReportRecord. They are documented here with the exceptions
 *   only for ease of reference.
 */

#define XCPT_FATAL_EXCEPTION    0xC0000000
#define XCPT_SEVERITY_CODE      0xC0000000
#define XCPT_CUSTOMER_CODE      0x20000000
#define XCPT_FACILITY_CODE      0x1FFF0000
#define XCPT_EXCEPTION_CODE     0x0000FFFF

/*
 * Access flags in ExceptionInformation
 */
#define XCPT_READ_ACCESS        0
#define XCPT_WRITE_ACCESS       1
#define XCPT_EXECUTE_ACCESS     2
#define XCPT_ACCESS_UNKNOWN     3

/*
 * Signal subtypes for XCPT_SIGNAL
 */
#define XCPT_SIGNAL_INTR        1
#define XCPT_SIGNAL_KILLPROC    3
#define XCPT_SIGNAL_BREAK       4

/*
 * Portable non-fatal software generated exceptions
 */

#define XCPT_GUARD_PAGE_VIOLATION       0x80000001
      /* ExceptionInformation[ 0 ] - R/W flag  */
      /* ExceptionInformation[ 1 ] - FaultAddr */

#define XCPT_UNABLE_TO_GROW_STACK       0x80010001

/*
 * Portable fatal hardware generated exceptions
 */

#define XCPT_DATATYPE_MISALIGNMENT      0x80000002
      /* ExceptionInformation[ 0 ] - R/W flag  */
      /* ExceptionInformation[ 1 ] - Alignment */
      /* ExceptionInformation[ 2 ] - FaultAddr */

#define XCPT_BREAKPOINT                 0xC0000003
      /* ExceptionInformation[ 0 ] - R/W/E/I flags */

#define XCPT_SINGLE_STEP                0xC0000004

#define XCPT_ACCESS_VIOLATION           0xC0000005
      /* ExceptionInformation[ 0 ] - R/W flag  */
      /* ExceptionInformation[ 1 ] - FaultAddr */

#define XCPT_ILLEGAL_INSTRUCTION        0xC000001C
#define XCPT_FLOAT_DENORMAL_OPERAND     0xC0000094
#define XCPT_FLOAT_DIVIDE_BY_ZERO       0xC0000095
#define XCPT_FLOAT_INEXACT_RESULT       0xC0000096
#define XCPT_FLOAT_INVALID_OPERATION    0xC0000097
#define XCPT_FLOAT_OVERFLOW             0xC0000098
#define XCPT_FLOAT_STACK_CHECK          0xC0000099
#define XCPT_FLOAT_UNDERFLOW            0xC000009A

#define XCPT_INTEGER_DIVIDE_BY_ZERO     0xC000009B
#define XCPT_INTEGER_OVERFLOW           0xC000009C
#define XCPT_PRIVILEGED_INSTRUCTION     0xC000009D

/*
 * Portable fatal software generated exceptions
 */

#define XCPT_IN_PAGE_ERROR              0xC0000006
      /* ExceptionInformation[ 0 ] - FaultAddr */

#define XCPT_PROCESS_TERMINATE          0xC0010001
#define XCPT_ASYNC_PROCESS_TERMINATE    0xC0010002
      /* ExceptionInfo[0] - TID of 'terminator' thread */

#define XCPT_NONCONTINUABLE_EXCEPTION   0xC0000024
#define XCPT_INVALID_DISPOSITION        0xC0000025

/*
 * Non-portable fatal exceptions
 */

#define XCPT_INVALID_LOCK_SEQUENCE      0xC000001D
#define XCPT_ARRAY_BOUNDS_EXCEEDED      0xC0000093

/*
 * Misc exceptions
 */

#define XCPT_UNWIND                     0xC0000026
#define XCPT_BAD_STACK                  0xC0000027
#define XCPT_INVALID_UNWIND_TARGET      0xC0000028

/*
 * Signal Exceptions
 */

#define XCPT_SIGNAL                     0xC0010003
      /* ExceptionInfo[0] - signal number */

/*
 * Exception Context.
 *
 * This is the machine specific register contents for the thread
 * at the time of the exception. Note that only the register sets
 * specified by ContextFlags contain valid data. Conversely, only
 * registers specified in ContextFlags will be restored if an exception
 * is handled.
 */

/* XLATOFF */
#pragma pack(1)
/* XLATON */

struct _fpreg {     /* coprocessor stack register element */
   ULONG losig;
   ULONG hisig;
   USHORT signexp;
};
typedef struct _fpreg FPREG;

/* XLATOFF */
#pragma pack()
/* XLATON */


/*
 * ExceptionReportRecord
 *
 * This structure contains machine independant information about an
 * exception/unwind. No system exception will ever have more than
 * EXCEPTION_MAXIMUM_PARAMETERS parameters. User exceptions are not
 * bound to this limit.
 */

#undef EXCEPTION_MAXIMUM_PARAMETERS

#define EXCEPTION_MAXIMUM_PARAMETERS 4  /* Enough for all system exceptions. */

struct _EXCEPTIONREPORTRECORD {
   ULONG   ExceptionNum;                /* exception number */
   ULONG   fHandlerFlags;
   struct  _EXCEPTIONREPORTRECORD    *NestedExceptionReportRecord;
   PVOID   ExceptionAddress;
   ULONG   cParameters;                 /* Size of Exception Specific Info */
   ULONG   ExceptionInfo[EXCEPTION_MAXIMUM_PARAMETERS];
                                        /* Exception Specfic Info */
};
typedef struct _EXCEPTIONREPORTRECORD EXCEPTIONREPORTRECORD;
typedef struct _EXCEPTIONREPORTRECORD *PEXCEPTIONREPORTRECORD;

/*
 * ExceptionRegistrationRecord
 *
 * These are linked together to form a chain of exception handlers that
 * will be dispatched to upon receipt of an exception.
 *
 * NOTE: Exception handlers should *NOT* be Registered directly from a
 *       high level language, such as 'C'. This is the responsibility of
 *       the language runtime.
 */

struct _EXCEPTIONREGISTRATIONRECORD {
   struct _EXCEPTIONREGISTRATIONRECORD *prev_structure;
   ULONG (*ExceptionHandler)();
};
typedef struct _EXCEPTIONREGISTRATIONRECORD EXCEPTIONREGISTRATIONRECORD;
typedef struct _EXCEPTIONREGISTRATIONRECORD *PEXCEPTIONREGISTRATIONRECORD;

//
// DosSetSigExceptionFocus codes
//

#define SIG_UNSETFOCUS 0
#define SIG_SETFOCUS 1

APIRET
DosRaiseException(
    IN PEXCEPTIONREPORTRECORD ExceptionReportRecord
    );

APIRET
DosUnwindException(
    IN PEXCEPTIONREGISTRATIONRECORD ExceptionHandler,
    IN PVOID TargetIP,
    IN PEXCEPTIONREPORTRECORD ExceptionReportRecord
    );

APIRET
DosEnterMustComplete(
    OUT PULONG NestingLevel
    );

APIRET
DosExitMustComplete(
    OUT PULONG NestingLevel
    );

APIRET
DosAcknowledgeSignalException(
    IN ULONG SignalNumber
    );

APIRET
DosSendSignalException(
    IN PID ProcessId,
    IN ULONG Exception
    );

APIRET
DosSetSignalExceptionFocus(
    IN BOOL32 Flag,
    OUT PULONG NestingLevel
    );

#endif // INCL_OS2V20_EXCEPTIONS

#ifdef INCL_OS2V20_ERRORMSG

/* Error API Calls */

APIRET
DosError(
    IN ULONG ErrorFlags
    );

/* definitions for DosError - combine with | */

#define DE_DISABLE_HARD_ERRORS  0x00000000
#define DE_ENABLE_HARD_ERRORS   0x00000001
#define DE_ENABLE_EXCEPTIONS    0x00000000
#define DE_DISABLE_EXCEPTIONS   0x00000002

#define FERR_DISABLEHARDERR     0x00000000L     /* disable hard error popups */
#define FERR_ENABLEHARDERR      0x00000001L     /* enable hard error popups */
#define FERR_ENABLEEXCEPTION    0x00000000L     /* enable exception popups */
#define FERR_DISABLEEXCEPTION   0x00000002L     /* disable exception popups */


APIRET
DosErrClass(
    IN ULONG ErrorCode,
    OUT PULONG ErrorClass,
    OUT PULONG ErrorAction,
    OUT PULONG ErrorLocus
    );

/* Values for error CLASS */

#define ERRCLASS_OUTRES                 1
#define ERRCLASS_TEMPSIT                2
#define ERRCLASS_AUTH                   3
#define ERRCLASS_INTRN                  4
#define ERRCLASS_HRDFAIL                5
#define ERRCLASS_SYSFAIL                6
#define ERRCLASS_APPERR                 7
#define ERRCLASS_NOTFND                 8
#define ERRCLASS_BADFMT                 9
#define ERRCLASS_LOCKED                 10
#define ERRCLASS_MEDIA                  11
#define ERRCLASS_ALREADY                12
#define ERRCLASS_UNK                    13
#define ERRCLASS_CANT                   14
#define ERRCLASS_TIME                   15

/* Values for error ACTION */

#define ERRACT_RETRY                    1
#define ERRACT_DLYRET                   2
#define ERRACT_USER                     3
#define ERRACT_ABORT                    4
#define ERRACT_PANIC                    5
#define ERRACT_IGNORE                   6
#define ERRACT_INTRET                   7

/* Values for error LOCUS */

#define ERRLOC_UNK                      1
#define ERRLOC_DISK                     2
#define ERRLOC_NET                      3
#define ERRLOC_SERDEV                   4
#define ERRLOC_MEM                      5


APIRET
DosGetMessage(
    IN PSZ Variables[],
    IN ULONG CountVariables,
    OUT PCHAR Buffer,
    IN ULONG Length,
    IN ULONG MessageNumber,
    IN PSZ MessageFileName,
    OUT PULONG MessageLength
    );

APIRET
DosInsertMessage(
    IN PSZ Variables[],
    IN ULONG CountVariables,
    IN PCHAR Message,
    IN ULONG MessageLength,
    OUT PCHAR Buffer,
    IN ULONG Length,
    OUT PULONG ActualMessageLength
    );

APIRET
DosPutMessage(
    IN HFILE FileHandle,
    IN ULONG MessageLength,
    IN PCHAR Message
    );

APIRET
DosQueryMessageCP(
    PCHAR Buffer,
    ULONG Length,
    IN PSZ MessageFileName,
    OUT PULONG ActualLength
    );


#endif // INCL_OS2V20_ERRORMSG

#ifdef INCL_OS2V20_SESSIONMGR

/*** Session manager API Calls */


APIRET
DosStartSession(
    IN PSTARTDATA StartData,
    OUT PULONG SessionId,
    OUT PPID ProcessId
    );

APIRET
DosSetSession(
    IN ULONG SessionId,
    IN PSTATUSDATA SessionStatusData
    );

APIRET
DosSelectSession(
    IN ULONG SessionId,
    IN ULONG ulReserved
    );

APIRET
DosStopSession(
    IN ULONG StopTarget,
    IN ULONG SessionId,
    IN ULONG ulReserved
    );

#define DSS_SESSION         0x0
#define DSS_ALL_SESSIONS    0x1

#endif // INCL_OS2V20_SESSIONMSG

#ifdef INCL_OS2V20_DEVICE_SUPPORT

/*** Device support API Calls */

APIRET
DosBeep(
    IN ULONG Frequency,
    IN ULONG Duration
    );


APIRET
DosDevConfig(
    OUT PVOID DeviceInformation,
    IN ULONG DeviceInformationIndex
    );

#define DDC_NUMBER_PRINTERS         0       // USHORT
#define DDC_NUMBER_RS232_PORTS      1       // USHORT
#define DDC_NUMBER_DISKETTE_DRIVES  2       // USHORT
#define DDC_MATH_COPROCESSOR        3       // BYTE
#define DDC_PC_SUBMODEL_TYPE        4       // BYTE
#define DDC_PC_MODEL_TYPE           5       // BYTE
#define DDC_PRIMARY_DISPLAY_TYPE    6       // BYTE
#define DDC_COPROCESSORTYPE         7       // BYTE

APIRET
DosQuerySysInfo(
    IN ULONG SysInfoIndexStart,
    IN ULONG SysInfoIndexEnd,
    OUT PBYTE Buffer,
    IN ULONG Length
    );

/* Values for SysInfoIndex of DosQuerySysInfo */

#define QSV_MAX_PATH_LENGTH     1
/*
defined in os2v12.h #define Q_MAX_PATH_LENGTH       QSV_MAX_PATH_LENGTH
*/
#define QSV_MAX_TEXT_SESSIONS   2
#define QSV_MAX_PM_SESSIONS     3
#define QSV_MAX_VDM_SESSIONS    4
#define QSV_BOOT_DRIVE          5       /* 1=A, 2=B, etc. */
#define QSV_DYN_PRI_VARIATION   6       /* 0=Absolute, 1=Dynamic */
#define QSV_MAX_WAIT            7       /* seconds */
#define QSV_MIN_SLICE           8       /* milli seconds */
#define QSV_MAX_SLICE           9       /* milli seconds */
#define QSV_PAGE_SIZE           10
#define QSV_VERSION_MAJOR       11
#define QSV_VERSION_MINOR       12
#define QSV_VERSION_REVISION    13      /* Revision letter */
#define QSV_MS_COUNT            14      /* Free running millisecond counter */
#define QSV_TIME_LOW            15      /* Low dword of time in seconds */
#define QSV_TIME_HIGH           16      /* High dword of time in seconds */
#define QSV_TOTPHYSMEM          17      /* Physical memory on system */
#define QSV_TOTRESMEM           18      /* Resident memory on system */
#define QSV_TOTAVAILMEM         19      /* Available memory for all processes */
#define QSV_MAXPRMEM            20      /* Avail private mem for calling proc */
#define QSV_MAXSHMEM            21      /* Avail shared mem for calling proc */
#define QSV_TIMER_INTERVAL      22      /* Timer interval in tenths of ms */
#define QSV_MAX_COMP_LENGTH     23      /* max len of one component in a name */
#define QSV_MAXIMUM_INDEX       23

#ifdef NOT_IMPLEMENTED

APIRET
DosDevIOCtl(
    PVOID pData,
    ULONG cbData,
    PVOID pParams,
    ULONG cbParams,
    ULONG function,
    ULONG category,
    PHFILE phDevice
    );

APIRET
DosPhysicalDisk(
    ULONG function,
    PBYTE pBuf,
    ULONG cbBuf,
    PBYTE pParams,
    ULONG cbParams
    );
#endif // NOT_IMPLEMENTED

#endif // INCL_OS2V20_DEVICE_SUPPORT

#ifdef INCL_OS2V20_PIPES

/*** Pipe API Calls */

APIRET
DosCreatePipe(
    OUT PHFILE phfRead,
    OUT PHFILE phfWrite,
    IN ULONG PipeSize
    );

#endif // INCL_OS2V20_PIPES


#ifdef INCL_OS2V20_QUEUES

#define DC_QUEUES_NAMEPREFIX "\\QUEUES\\"

/*** Queue API Calls */

APIRET
DosCreateQueue(
    OUT PHQUEUE QueueHandle,
    IN ULONG QueueType,
    IN PSZ ObjectName
    );

#define QUE_FIFO            0x0
#define QUE_LIFO            0x1
#define QUE_PRIORITY        0x2

APIRET
DosOpenQueue(
    OUT PPID OwnerProcessId,
    OUT PHQUEUE QueueHandle,
    IN PSZ ObjectName
    );

APIRET
DosCloseQueue(
    IN HQUEUE QueueHandle
    );

APIRET
DosPurgeQueue(
    IN HQUEUE QueueHandle
    );

APIRET
DosQueryQueue(
    IN HQUEUE QueueHandle,
    OUT PULONG CountQueuedElements
    );


APIRET
DosPeekQueue(
    IN HQUEUE QueueHandle,
    OUT PREQUESTDATA RequestInfo,
    OUT PULONG DataLength,
    OUT PULONG Data,
    IN OUT PULONG ReadPosition,
    IN BOOL32 NoWait,
    OUT PBYTE ElementPriority,
    IN HSEM SemHandle
    );

APIRET
DosReadQueue(
    IN HQUEUE QueueHandle,
    OUT PREQUESTDATA RequestInfo,
    OUT PULONG DataLength,
    OUT PULONG Data,
    IN ULONG ReadPosition,
    IN BOOL32 NoWait,
    OUT PBYTE ElementPriority,
    IN HSEM SemHandle
    );

APIRET
DosWriteQueue(
    IN HQUEUE QueueHandle,
    IN ULONG SenderData,
    IN ULONG DataLength,
    IN PBYTE Data,
    IN ULONG ElementPriority
    );

#endif // INCL_OS2V20_QUEUES

#ifdef INCL_OS2V20_ERRORS

#ifndef _OS2V20_ERRORS_

#define _OS2V20_ERRORS_

#define NO_ERROR                        0

#define ERROR_INVALID_FUNCTION          1
#define ERROR_FILE_NOT_FOUND            2
#define ERROR_PATH_NOT_FOUND            3
#define ERROR_TOO_MANY_OPEN_FILES       4
#define ERROR_ACCESS_DENIED             5
#define ERROR_INVALID_HANDLE            6
#define ERROR_ARENA_TRASHED             7
#define ERROR_NOT_ENOUGH_MEMORY         8
#define ERROR_INVALID_BLOCK             9
#define ERROR_BAD_ENVIRONMENT           10
#define ERROR_BAD_FORMAT                11
#define ERROR_INVALID_ACCESS            12
#define ERROR_INVALID_DATA              13

#define ERROR_INVALID_DRIVE             15
#define ERROR_CURRENT_DIRECTORY         16
#define ERROR_NOT_SAME_DEVICE           17
#define ERROR_NO_MORE_FILES             18
#define ERROR_WRITE_PROTECT             19
#define ERROR_BAD_UNIT                  20
#define ERROR_NOT_READY                 21
#define ERROR_BAD_COMMAND               22
#define ERROR_CRC                       23
#define ERROR_BAD_LENGTH                24
#define ERROR_SEEK                      25
#define ERROR_NOT_DOS_DISK              26
#define ERROR_SECTOR_NOT_FOUND          27
#define ERROR_OUT_OF_PAPER              28
#define ERROR_WRITE_FAULT               29
#define ERROR_READ_FAULT                30
#define ERROR_GEN_FAILURE               31
#define ERROR_SHARING_VIOLATION         32
#define ERROR_LOCK_VIOLATION            33
#define ERROR_WRONG_DISK                34
#define ERROR_FCB_UNAVAILABLE           35
#define ERROR_SHARING_BUFFER_EXCEEDED   36
#define ERROR_CODE_PAGE_MISMATCHED      37
#define ERROR_HANDLE_EOF                38
#define ERROR_HANDLE_DISK_FULL          39
#define ERROR_NOT_SUPPORTED             50
#define ERROR_REM_NOT_LIST              51
#define ERROR_DUP_NAME                  52
#define ERROR_BAD_NETPATH               53
#define ERROR_NETWORK_BUSY              54
#define ERROR_DEV_NOT_EXIST             55
#define ERROR_TOO_MANY_CMDS             56
#define ERROR_ADAP_HDW_ERR              57
#define ERROR_BAD_NET_RESP              58
#define ERROR_UNEXP_NET_ERR             59
#define ERROR_BAD_REM_ADAP              60
#define ERROR_PRINTQ_FULL               61
#define ERROR_NO_SPOOL_SPACE            62
#define ERROR_PRINT_CANCELLED           63
#define ERROR_NETNAME_DELETED           64
#define ERROR_NETWORK_ACCESS_DENIED     65
#define ERROR_BAD_DEV_TYPE              66
#define ERROR_BAD_NET_NAME              67
#define ERROR_TOO_MANY_NAMES            68
#define ERROR_TOO_MANY_SESS             69
#define ERROR_SHARING_PAUSED            70
#define ERROR_REQ_NOT_ACCEP             71
#define ERROR_REDIR_PAUSED              72

#define ERROR_FILE_EXISTS               80
#define ERROR_DUP_FCB                   81
#define ERROR_CANNOT_MAKE               82
#define ERROR_FAIL_I24                  83
#define ERROR_OUT_OF_STRUCTURES         84
#define ERROR_ALREADY_ASSIGNED          85
#define ERROR_INVALID_PASSWORD          86
#define ERROR_INVALID_PARAMETER         87
#define ERROR_NET_WRITE_FAULT           88
#define ERROR_NO_PROC_SLOTS             89
#define ERROR_NOT_FROZEN                90
#define ERROR_SYS_COMP_NOT_LOADED       ERROR_NOT_FROZEN
#define ERROR_TSTOVFL                   91
#define ERROR_TSTDUP                    92
#define ERROR_NO_ITEMS                  93
#define ERROR_INTERRUPT                 95
#define ERROR_DEVICE_IN_USE             99
#define ERROR_TOO_MANY_SEMAPHORES       100
#define ERROR_EXCL_SEM_ALREADY_OWNED    101
#define ERROR_SEM_IS_SET                102
#define ERROR_TOO_MANY_SEM_REQUESTS     103
#define ERROR_INVALID_AT_INTERRUPT_TIME 104
#define ERROR_SEM_OWNER_DIED            105
#define ERROR_SEM_USER_LIMIT            106
#define ERROR_DISK_CHANGE               107
#define ERROR_DRIVE_LOCKED              108
#define ERROR_BROKEN_PIPE               109
#define ERROR_OPEN_FAILED               110
#define ERROR_BUFFER_OVERFLOW           111
#define ERROR_DISK_FULL                 112
#define ERROR_NO_MORE_SEARCH_HANDLES    113
#define ERROR_INVALID_TARGET_HANDLE     114
#define ERROR_PROTECTION_VIOLATION      115
#define ERROR_VIOKBD_REQUEST            116
#define ERROR_INVALID_CATEGORY          117
#define ERROR_INVALID_VERIFY_SWITCH     118
#define ERROR_BAD_DRIVER_LEVEL          119
#define ERROR_CALL_NOT_IMPLEMENTED      120
#define ERROR_SEM_TIMEOUT               121
#define ERROR_INSUFFICIENT_BUFFER       122
#define ERROR_INVALID_NAME              123
#define ERROR_INVALID_LEVEL             124
#define ERROR_NO_VOLUME_LABEL           125
#define ERROR_MOD_NOT_FOUND             126
#define ERROR_PROC_NOT_FOUND            127
#define ERROR_WAIT_NO_CHILDREN          128
#define ERROR_CHILD_NOT_COMPLETE        129
#define ERROR_DIRECT_ACCESS_HANDLE      130
#define ERROR_NEGATIVE_SEEK             131
#define ERROR_SEEK_ON_DEVICE            132
#define ERROR_IS_JOIN_TARGET            133
#define ERROR_IS_JOINED                 134
#define ERROR_IS_SUBSTED                135
#define ERROR_NOT_JOINED                136
#define ERROR_NOT_SUBSTED               137
#define ERROR_JOIN_TO_JOIN              138
#define ERROR_SUBST_TO_SUBST            139
#define ERROR_JOIN_TO_SUBST             140
#define ERROR_SUBST_TO_JOIN             141
#define ERROR_BUSY_DRIVE                142
#define ERROR_SAME_DRIVE                143
#define ERROR_DIR_NOT_ROOT              144
#define ERROR_DIR_NOT_EMPTY             145
#define ERROR_IS_SUBST_PATH             146
#define ERROR_IS_JOIN_PATH              147
#define ERROR_PATH_BUSY                 148
#define ERROR_IS_SUBST_TARGET           149
#define ERROR_SYSTEM_TRACE              150
#define ERROR_INVALID_EVENT_COUNT       151
#define ERROR_TOO_MANY_MUXWAITERS       152
#define ERROR_INVALID_LIST_FORMAT       153
#define ERROR_LABEL_TOO_LONG            154
#define ERROR_TOO_MANY_TCBS             155
#define ERROR_SIGNAL_REFUSED            156
#define ERROR_DISCARDED                 157
#define ERROR_NOT_LOCKED                158
#define ERROR_BAD_THREADID_ADDR         159
#define ERROR_BAD_ARGUMENTS             160
#define ERROR_BAD_PATHNAME              161
#define ERROR_SIGNAL_PENDING            162
#define ERROR_UNCERTAIN_MEDIA           163
#define ERROR_MAX_THRDS_REACHED         164
#define ERROR_MONITORS_NOT_SUPPORTED    165
#define ERROR_UNC_DRIVER_NOT_INSTALLED  166
#define ERROR_LOCK_FAILED               167
#define ERROR_SWAPIO_FAILED             168
#define ERROR_SWAPIN_FAILED             169
#define ERROR_BUSY                      170

#define ERROR_INVALID_SEGMENT_NUMBER    180
#define ERROR_INVALID_CALLGATE          181
#define ERROR_INVALID_ORDINAL           182
#define ERROR_ALREADY_EXISTS            183
#define ERROR_NO_CHILD_PROCESS          184
#define ERROR_CHILD_ALIVE_NOWAIT        185
#define ERROR_INVALID_FLAG_NUMBER       186
#define ERROR_SEM_NOT_FOUND             187
#define ERROR_INVALID_STARTING_CODESEG  188
#define ERROR_INVALID_STACKSEG          189
#define ERROR_INVALID_MODULETYPE        190
#define ERROR_INVALID_EXE_SIGNATURE     191
#define ERROR_EXE_MARKED_INVALID        192
#define ERROR_BAD_EXE_FORMAT            193
#define ERROR_ITERATED_DATA_EXCEEDS_64K 194
#define ERROR_INVALID_MINALLOCSIZE      195
#define ERROR_DYNLINK_FROM_INVALID_RING 196
#define ERROR_IOPL_NOT_ENABLED          197
#define ERROR_INVALID_SEGDPL            198
#define ERROR_AUTODATASEG_EXCEEDS_64K   199
#define ERROR_RING2SEG_MUST_BE_MOVABLE  200
#define ERROR_RELOC_CHAIN_XEEDS_SEGLIM  201
#define ERROR_INFLOOP_IN_RELOC_CHAIN    202
#define ERROR_ENVVAR_NOT_FOUND          203
#define ERROR_NOT_CURRENT_CTRY          204
#define ERROR_NO_SIGNAL_SENT            205
#define ERROR_FILENAME_EXCED_RANGE      206
#define ERROR_RING2_STACK_IN_USE        207
#define ERROR_META_EXPANSION_TOO_LONG   208
#define ERROR_INVALID_SIGNAL_NUMBER     209
#define ERROR_THREAD_1_INACTIVE         210
#define ERROR_INFO_NOT_AVAIL            211
#define ERROR_LOCKED                    212
#define ERROR_BAD_DYNALINK              213
#define ERROR_TOO_MANY_MODULES          214
#define ERROR_NESTING_NOT_ALLOWED       215
#define ERROR_CANNOT_SHRINK             216
#define ERROR_ZOMBIE_PROCESS            217
#define ERROR_STACK_IN_HIGH_MEMORY      218
#define ERROR_INVALID_EXITROUTINE_RING  219
#define ERROR_GETBUF_FAILED             220
#define ERROR_FLUSHBUF_FAILED           221
#define ERROR_TRANSFER_TOO_LONG         222
#define ERROR_FORCENOSWAP_FAILED        223
#define ERROR_NO_CHILDREN               228
#define ERROR_INVALID_SCREEN_GROUP      229

#define ERROR_BAD_PIPE                  230
#define ERROR_PIPE_BUSY                 231
#define ERROR_NO_DATA                   232
#define ERROR_PIPE_NOT_CONNECTED        233
#define ERROR_MORE_DATA                 234

#define ERROR_VC_DISCONNECTED           240

#define ERROR_CIRCULARITY_REQUESTED     250
#define ERROR_DIRECTORY_IN_CDS          251
#define ERROR_INVALID_FSD_NAME          252
#define ERROR_INVALID_PATH              253
#define ERROR_INVALID_EA_NAME           254
#define ERROR_EA_LIST_INCONSISTENT      255
#define ERROR_EA_LIST_TOO_LONG          256
#define ERROR_NO_META_MATCH             257
#define ERROR_FINDNOTIFY_TIMEOUT        258
#define ERROR_NO_MORE_ITEMS             259
#define ERROR_SEARCH_STRUC_REUSED       260
#define ERROR_CHAR_NOT_FOUND            261
#define ERROR_TOO_MUCH_STACK            262
#define ERROR_INVALID_ATTR              263
#define ERROR_INVALID_STARTING_RING     264
#define ERROR_INVALID_DLL_INIT_RING     265
#define ERROR_CANNOT_COPY               266
#define ERROR_DIRECTORY                 267
#define ERROR_OPLOCKED_FILE             268
#define ERROR_OPLOCK_THREAD_EXISTS      269
#define ERROR_VOLUME_CHANGED            270
#define ERROR_FINDNOTIFY_HANDLE_IN_USE  271
#define ERROR_FINDNOTIFY_HANDLE_CLOSED  272
#define ERROR_NOTIFY_OBJECT_REMOVED     273
#define ERROR_ALREADY_SHUTDOWN          274
#define ERROR_EAS_DIDNT_FIT             275
#define ERROR_EA_FILE_CORRUPT           276
#define ERROR_EA_TABLE_FULL             277
#define ERROR_INVALID_EA_HANDLE         278
#define ERROR_NO_CLUSTER                279
#define ERROR_CREATE_EA_FILE            280
#define ERROR_CANNOT_OPEN_EA_FILE       281
#define ERROR_EAS_NOT_SUPPORTED         282
#define ERROR_NEED_EAS_FOUND            283
#define ERROR_DUPLICATE_HANDLE          284
#define ERROR_DUPLICATE_NAME            285
#define ERROR_EMPTY_MUXWAIT             286
#define ERROR_MUTEX_OWNED               287
#define ERROR_NOT_OWNER                 288
#define ERROR_PARAM_TOO_SMALL           289
#define ERROR_TOO_MANY_HANDLES          290
#define ERROR_TOO_MANY_OPENS            291
#define ERROR_WRONG_TYPE                292
#define ERROR_INVALID_TID               293
#define ERROR_THREAD_NOT_TERMINATED     294
#define ERROR_INIT_ROUTINE_FAILED       295
#define ERROR_MODULE_IN_USE             296
#define ERROR_NOT_ENOUGH_WATCHPOINTS    297
#define ERROR_TOO_MANY_POSTS            298
#define ERROR_ALREADY_POSTED            299
#define ERROR_ALREADY_RESET             300
#define ERROR_SEM_BUSY                  301

#define ERROR_INVALID_PROCID            303
#define ERROR_INVALID_PDELTA            304
#define ERROR_NOT_DESCENDANT            305
#define ERROR_NOT_SESSION_MANAGER       306
#define ERROR_INVALID_PCLASS            307
#define ERROR_INVALID_SCOPE             308
#define ERROR_INVALID_THREADID          309
#define ERROR_DOSSUB_SHRINK             310
#define ERROR_DOSSUB_NOMEM              311
#define ERROR_DOSSUB_OVERLAP            312
#define ERROR_DOSSUB_BADSIZE            313
#define ERROR_DOSSUB_BADFLAG            314
#define ERROR_DOSSUB_BADSELECTOR        315
#define ERROR_MR_MSG_TOO_LONG           316
#define ERROR_MR_MID_NOT_FOUND          317
#define ERROR_MR_UN_ACC_MSGF            318
#define ERROR_MR_INV_MSGF_FORMAT        319
#define ERROR_MR_INV_IVCOUNT            320
#define ERROR_MR_UN_PERFORM             321
#define ERROR_TS_WAKEUP                 322
#define ERROR_TS_SEMHANDLE              323
#define ERROR_TS_NOTIMER                324
#define ERROR_TS_HANDLE                 326
#define ERROR_TS_DATETIME               327
#define ERROR_SYS_INTERNAL              328
#define ERROR_QUE_CURRENT_NAME          329
#define ERROR_QUE_PROC_NOT_OWNED        330
#define ERROR_QUE_PROC_OWNED            331
#define ERROR_QUE_DUPLICATE             332
#define ERROR_QUE_ELEMENT_NOT_EXIST     333
#define ERROR_QUE_NO_MEMORY             334
#define ERROR_QUE_INVALID_NAME          335
#define ERROR_QUE_INVALID_PRIORITY      336
#define ERROR_QUE_INVALID_HANDLE        337
#define ERROR_QUE_LINK_NOT_FOUND        338
#define ERROR_QUE_MEMORY_ERROR          339
#define ERROR_QUE_PREV_AT_END           340
#define ERROR_QUE_PROC_NO_ACCESS        341
#define ERROR_QUE_EMPTY                 342
#define ERROR_QUE_NAME_NOT_EXIST        343
#define ERROR_QUE_NOT_INITIALIZED       344
#define ERROR_QUE_UNABLE_TO_ACCESS      345
#define ERROR_QUE_UNABLE_TO_ADD         346
#define ERROR_QUE_UNABLE_TO_INIT        347
#define ERROR_VIO_INVALID_MASK          349
#define ERROR_VIO_PTR                   350
#define ERROR_VIO_APTR                  351
#define ERROR_VIO_RPTR                  352
#define ERROR_VIO_CPTR                  353
#define ERROR_VIO_LPTR                  354
#define ERROR_VIO_MODE                  355
#define ERROR_VIO_WIDTH                 356
#define ERROR_VIO_ATTR                  357
#define ERROR_VIO_ROW                   358
#define ERROR_VIO_COL                   359
#define ERROR_VIO_TOPROW                360
#define ERROR_VIO_BOTROW                361
#define ERROR_VIO_RIGHTCOL              362
#define ERROR_VIO_LEFTCOL               363
#define ERROR_SCS_CALL                  364
#define ERROR_SCS_VALUE                 365
#define ERROR_VIO_WAIT_FLAG             366
#define ERROR_VIO_UNLOCK                367
#define ERROR_SGS_NOT_SESSION_MGR       368
#define ERROR_SMG_INVALID_SGID          369
#define ERROR_SMG_INVALID_SESSION_ID    369
#define ERROR_SMG_NOSG                  370
#define ERROR_SMG_NO_SESSIONS           370
#define ERROR_SMG_GRP_NOT_FOUND         371
#define ERROR_SMG_SESSION_NOT_FOUND     371
#define ERROR_SMG_SET_TITLE             372
#define ERROR_KBD_PARAMETER             373
#define ERROR_KBD_NO_DEVICE             374
#define ERROR_KBD_INVALID_IOWAIT        375
#define ERROR_KBD_INVALID_LENGTH        376
#define ERROR_KBD_INVALID_ECHO_MASK     377
#define ERROR_KBD_INVALID_INPUT_MASK    378
#define ERROR_MON_INVALID_PARMS         379
#define ERROR_MON_INVALID_DEVNAME       380
#define ERROR_MON_INVALID_HANDLE        381
#define ERROR_MON_BUFFER_TOO_SMALL      382
#define ERROR_MON_BUFFER_EMPTY          383
#define ERROR_MON_DATA_TOO_LARGE        384
#define ERROR_MOUSE_NO_DEVICE           385
#define ERROR_MOUSE_INV_HANDLE          386
#define ERROR_MOUSE_INV_PARMS           387
#define ERROR_MOUSE_CANT_RESET          388
#define ERROR_MOUSE_DISPLAY_PARMS       389
#define ERROR_MOUSE_INV_MODULE          390
#define ERROR_MOUSE_INV_ENTRY_PT        391
#define ERROR_MOUSE_INV_MASK            392
#define ERROR_MOUSE_NO_DATA             393
#define ERROR_MOUSE_PTR_DRAWN           394
#define ERROR_INVALID_FREQUENCY         395
#define ERROR_NLS_NO_COUNTRY_FILE       396
#define ERROR_NLS_OPEN_FAILED           397
#define ERROR_NLS_NO_CTRY_CODE          398
#define ERROR_NO_COUNTRY_OR_CODEPAGE    398
#define ERROR_NLS_TABLE_TRUNCATED       399
#define ERROR_NLS_BAD_TYPE              400
#define ERROR_NLS_TYPE_NOT_FOUND        401
#define ERROR_VIO_SMG_ONLY              402
#define ERROR_VIO_INVALID_ASCIIZ        403
#define ERROR_VIO_DEREGISTER            404
#define ERROR_VIO_NO_POPUP              405
#define ERROR_VIO_EXISTING_POPUP        406
#define ERROR_KBD_SMG_ONLY              407
#define ERROR_KBD_INVALID_ASCIIZ        408
#define ERROR_KBD_INVALID_MASK          409
#define ERROR_KBD_REGISTER              410
#define ERROR_KBD_DEREGISTER            411
#define ERROR_MOUSE_SMG_ONLY            412
#define ERROR_MOUSE_INVALID_ASCIIZ      413
#define ERROR_MOUSE_INVALID_MASK        414
#define ERROR_MOUSE_REGISTER            415
#define ERROR_MOUSE_DEREGISTER          416
#define ERROR_SMG_BAD_ACTION            417
#define ERROR_SMG_INVALID_CALL          418
#define ERROR_SCS_SG_NOTFOUND           419
#define ERROR_SCS_NOT_SHELL             420
#define ERROR_VIO_INVALID_PARMS         421
#define ERROR_VIO_FUNCTION_OWNED        422
#define ERROR_VIO_RETURN                423
#define ERROR_SCS_INVALID_FUNCTION      424
#define ERROR_SCS_NOT_SESSION_MGR       425
#define ERROR_VIO_REGISTER              426
#define ERROR_VIO_NO_MODE_THREAD        427
#define ERROR_VIO_NO_SAVE_RESTORE_THD   428
#define ERROR_VIO_IN_BG                 429
#define ERROR_VIO_ILLEGAL_DURING_POPUP  430
#define ERROR_SMG_NOT_BASESHELL         431
#define ERROR_SMG_BAD_STATUSREQ         432
#define ERROR_QUE_INVALID_WAIT          433
#define ERROR_VIO_LOCK                  434
#define ERROR_MOUSE_INVALID_IOWAIT      435
#define ERROR_VIO_INVALID_HANDLE        436
#define ERROR_VIO_ILLEGAL_DURING_LOCK   437
#define ERROR_VIO_INVALID_LENGTH        438
#define ERROR_KBD_INVALID_HANDLE        439
#define ERROR_KBD_NO_MORE_HANDLE        440
#define ERROR_KBD_CANNOT_CREATE_KCB     441
#define ERROR_KBD_CODEPAGE_LOAD_INCOMPL 442
#define ERROR_KBD_INVALID_CODEPAGE_ID   443
#define ERROR_KBD_NO_CODEPAGE_SUPPORT   444
#define ERROR_KBD_FOCUS_REQUIRED        445
#define ERROR_KBD_FOCUS_ALREADY_ACTIVE  446
#define ERROR_KBD_KEYBOARD_BUSY         447
#define ERROR_KBD_INVALID_CODEPAGE      448
#define ERROR_KBD_UNABLE_TO_FOCUS       449
#define ERROR_SMG_SESSION_NON_SELECT    450
#define ERROR_SMG_SESSION_NOT_FOREGRND  451
#define ERROR_SMG_SESSION_NOT_PARENT    452
#define ERROR_SMG_INVALID_START_MODE    453
#define ERROR_SMG_INVALID_RELATED_OPT   454
#define ERROR_SMG_INVALID_BOND_OPTION   455
#define ERROR_SMG_INVALID_SELECT_OPT    456
#define ERROR_SMG_START_IN_BACKGROUND   457
#define ERROR_SMG_INVALID_STOP_OPTION   458
#define ERROR_SMG_BAD_RESERVE           459
#define ERROR_SMG_PROCESS_NOT_PARENT    460
#define ERROR_SMG_INVALID_DATA_LENGTH   461
#define ERROR_SMG_NOT_BOUND             462
#define ERROR_SMG_RETRY_SUB_ALLOC       463
#define ERROR_KBD_DETACHED              464
#define ERROR_VIO_DETACHED              465
#define ERROR_MOU_DETACHED              466
#define ERROR_VIO_FONT                  467
#define ERROR_VIO_USER_FONT             468
#define ERROR_VIO_BAD_CP                469
#define ERROR_VIO_NO_CP                 470
#define ERROR_VIO_NA_CP                 471
#define ERROR_INVALID_CODE_PAGE         472
#define ERROR_CPLIST_TOO_SMALL          473
#define ERROR_CP_NOT_MOVED              474
#define ERROR_MODE_SWITCH_INIT          475
#define ERROR_CODE_PAGE_NOT_FOUND       476
#define ERROR_UNEXPECTED_SLOT_RETURNED  477
#define ERROR_SMG_INVALID_TRACE_OPTION  478
#define ERROR_VIO_INTERNAL_RESOURCE     479
#define ERROR_VIO_SHELL_INIT            480
#define ERROR_SMG_NO_HARD_ERRORS        481
#define ERROR_CP_SWITCH_INCOMPLETE      482
#define ERROR_VIO_TRANSPARENT_POPUP     483
#define ERROR_CRITSEC_OVERFLOW          484
#define ERROR_CRITSEC_UNDERFLOW         485
#define ERROR_VIO_BAD_RESERVE           486
#define ERROR_INVALID_ADDRESS           487
#define ERROR_ZERO_SELECTORS_REQUESTED  488
#define ERROR_NOT_ENOUGH_SELECTORS_AVA  489
#define ERROR_INVALID_SELECTOR          490
#define ERROR_SMG_INVALID_PROGRAM_TYPE  491
#define ERROR_SMG_INVALID_PGM_CONTROL   492
#define ERROR_SMG_INVALID_INHERIT_OPT   493
#define ERROR_VIO_EXTENDED_SG           494
#define ERROR_VIO_NOT_PRES_MGR_SG       495
#define ERROR_VIO_SHIELD_OWNED          496
#define ERROR_VIO_NO_MORE_HANDLES       497
#define ERROR_VIO_SEE_ERROR_LOG         498
#define ERROR_VIO_ASSOCIATED_DC         499
#define ERROR_KBD_NO_CONSOLE            500
#define ERROR_MOUSE_NO_CONSOLE          501
#define ERROR_MOUSE_INVALID_HANDLE      502
#define ERROR_SMG_INVALID_DEBUG_PARMS   503
#define ERROR_KBD_EXTENDED_SG           504
#define ERROR_MOU_EXTENDED_SG           505
#define ERROR_SMG_INVALID_ICON_FILE     506

/* New Equates For OS/2 1.2 */
#define ERROR_TRC_PID_NON_EXISTENT      507
#define ERROR_TRC_COUNT_ACTIVE          508
#define ERROR_TRC_SUSPENDED_BY_COUNT    509
#define ERROR_TRC_COUNT_INACTIVE        510
#define ERROR_TRC_COUNT_REACHED         511
#define ERROR_NO_MC_TRACE               512
#define ERROR_MC_TRACE                  513
#define ERROR_TRC_COUNT_ZERO            514
#define ERROR_SMG_TOO_MANY_DDS          515
#define ERROR_SMG_INVALID_NOTIFICATION  516
#define ERROR_LF_INVALID_FUNCTION       517
#define ERROR_LF_NOT_AVAIL              518
#define ERROR_LF_SUSPENDED              519
#define ERROR_LF_BUF_TOO_SMALL          520
#define ERROR_LF_BUFFER_CORRUPTED       521
#define ERROR_LF_INVALID_DAEMON         522
#define ERROR_LF_INVALID_TEMPL          523
#define ERROR_LF_GENERAL_FAILURE        524
#define ERROR_LF_INVALID_ID             525
#define ERROR_LF_INVALID_HANDLE         526
#define ERROR_LF_NO_ID_AVAIL            527
#define ERROR_LF_TEMPLATE_AREA_FULL     528
#define ERROR_LF_ID_IN_USE              529
#define ERROR_MOU_NOT_INITIALIZED       530
#define ERROR_MOUINITREAL_DONE          531

/* now more 2.0 stuff */
#define ERROR_DOSSUB_CORRUPTED          532
#define ERROR_MOUSE_CALLER_NOT_SUBSYS   533
#define ERROR_ARITHMETIC_OVERFLOW       534

/* TMR device */

#define ERROR_TMR_NO_DEVICE             535
#define ERROR_TMR_INVALID_TIME          536

/* Perfview */

#define ERROR_PVW_INVALID_ENTITY        537
#define ERROR_PVW_INVALID_ENTITY_TYPE   538
#define ERROR_PVW_INVALID_SPEC          539
#define ERROR_PVW_INVALID_RANGE_TYPE    540
#define ERROR_PVW_INVALID_COUNTER_BLK   541
#define ERROR_PVW_INVALID_TEXT_BLK      542

/* PROFILER */

#define ERROR_PRF_NOT_INITIALIZED       543
#define ERROR_PRF_ALREADY_INITIALIZED   544
#define ERROR_PRF_NOT_STARTED           545
#define ERROR_PRF_ALREADY_STARTED       546
#define ERROR_PRF_TIMER_OUT_OF_RANGE    547
#define ERROR_PRF_TIMER_RESET           548

#define ERROR_TIMEOUT                   640
#define ERROR_VDM_DOWN                  641
#define ERROR_VDM_LIMIT                 642
#define ERROR_VDD_NOT_FOUND             643
#define ERROR_INVALID_CALLER            644
#define ERROR_PID_MISMATCH              645
#define ERROR_INVALID_VDD_HANDLE        646
#define ERROR_VLPT_NO_SPOOLER           647
#define ERROR_VCOM_DEVICE_BUSY          648
#define ERROR_VLPT_DEVICE_BUSY          649
#define ERROR_NESTING_TOO_DEEP          650

#define ERROR_SWAPPER_NOT_ACTIVE        32768
#define ERROR_INVALID_SWAPID            32769
#define ERROR_IOERR_SWAP_FILE           32770
#define ERROR_SWAP_TABLE_FULL           32771
#define ERROR_SWAP_FILE_FULL            32772
#define ERROR_CANT_INIT_SWAPPER         32773
#define ERROR_SWAPPER_ALREADY_INIT      32774
#define ERROR_PMM_INSUFFICIENT_MEMORY   32775
#define ERROR_PMM_INVALID_FLAGS         32776
#define ERROR_PMM_INVALID_ADDRESS       32777
#define ERROR_PMM_LOCK_FAILED           32778
#define ERROR_PMM_UNLOCK_FAILED         32779
#define ERROR_PMM_MOVE_INCOMPLETE       32780
#define ERROR_UCOM_DRIVE_RENAMED        32781
#define ERROR_UCOM_FILENAME_TRUNCATED   32782
#define ERROR_UCOM_BUFFER_LENGTH        32783
#define ERROR_MON_CHAIN_HANDLE          32784
#define ERROR_MON_NOT_REGISTERED        32785
#define ERROR_SMG_ALREADY_TOP           32786
#define ERROR_PMM_ARENA_MODIFIED        32787
#define ERROR_SMG_PRINTER_OPEN          32788
#define ERROR_PMM_SET_FLAGS_FAILED      32789
#define ERROR_INVALID_DOS_DD            32790
#define ERROR_BLOCKED                   32791
#define ERROR_NOBLOCK                   32792
#define ERROR_INSTANCE_SHARED           32793
#define ERROR_NO_OBJECT                 32794
#define ERROR_PARTIAL_ATTACH            32795
#define ERROR_INCACHE                   32796
#define ERROR_SWAP_IO_PROBLEMS          32797
#define ERROR_CROSSES_OBJECT_BOUNDARY   32798
#define ERROR_LONGLOCK                  32799
#define ERROR_SHORTLOCK                 32800
#define ERROR_UVIRTLOCK                 32801
#define ERROR_ALIASLOCK                 32802
#define ERROR_ALIAS                     32803
#define ERROR_NO_MORE_HANDLES           32804
#define ERROR_SCAN_TERMINATED           32805
#define ERROR_TERMINATOR_NOT_FOUND      32806
#define ERROR_NOT_DIRECT_CHILD          32807
#define ERROR_DELAY_FREE                32808
#define ERROR_GUARDPAGE                 32809
#define ERROR_SWAPERROR                 32900
#define ERROR_LDRERROR                  32901
#define ERROR_NOMEMORY                  32902
#define ERROR_NOACCESS                  32903
#define ERROR_CPSIO_CODE_PAGE_INVALID   65026
#define ERROR_CPSIO_NO_SPOOLER          65027
#define ERROR_CPSIO_FONT_ID_INVALID     65028
#define ERROR_CPSIO_INTERNAL_ERROR      65033
#define ERROR_CPSIO_INVALID_PTR_NAME    65034
#define ERROR_CPSIO_NOT_ACTIVE          65037
#define ERROR_CPSIO_PID_FULL            65039
#define ERROR_CPSIO_PID_NOT_FOUND       65040
#define ERROR_CPSIO_READ_CTL_SEQ        65043
#define ERROR_CPSIO_READ_FNT_DEF        65045
#define ERROR_CPSIO_WRITE_ERROR         65047
#define ERROR_CPSIO_WRITE_FULL_ERROR    65048
#define ERROR_CPSIO_WRITE_HANDLE_BAD    65049
#define ERROR_CPSIO_SWIT_LOAD           65074
#define ERROR_CPSIO_INV_COMMAND         65077
#define ERROR_CPSIO_NO_FONT_SWIT        65078
#define ERROR_ENTRY_IS_CALLGATE         65079

#define ERROR_USER_DEFINED_BASE         0xFF00

#endif // _OS2V20_ERRORS_

#endif // INCL_OS2V20_ERRORS

//
// The following error codes are defined by the OS/2 Emulation Subsystem
// server as a means of communicating with the stub procedures in the OS/2
// Client DLL.
//

#define ERROR_SS_RETRY                  0xF000
#define ERROR_SS_UNKNOWN_STATUS         0xF800

APIRET
DosNullApiCall(
    IN LONG CountArguments,
    IN PCHAR *Arguments OPTIONAL
    );

