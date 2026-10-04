/*++ BUILD Version: 0002    // Increment this if a change has global effects

Copyright (c) 1989  Microsoft Corporation

Module Name:

    console.h

Abstract:

    This module contains the public data structures, data types,
    and procedures exported by the NT console subsystem.

Author:

    Therese Stowell (thereses) 26-Oct-1990

Revision History:

--*/


typedef struct _COORD {
    SHORT X;
    SHORT Y;
} COORD, *PCOORD;

typedef struct _SMALL_RECT {
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
} SMALL_RECT, *PSMALL_RECT;

typedef struct _KEY_EVENT_RECORD {
    BOOL bKeyDown;
    WORD wRepeatCount;
    WORD wVirtualKeyCode;
    WORD wVirtualScanCode;
    union {
        WCHAR UnicodeChar;
        CHAR   AsciiChar;
    } uChar;
    DWORD dwControlKeyState;
} KEY_EVENT_RECORD, *PKEY_EVENT_RECORD;

//
// ControlKeyState flags
//

#define RIGHT_ALT_PRESSED     0x0001 // the right alt key is pressed.
#define LEFT_ALT_PRESSED      0x0002 // the left alt key is pressed.
#define RIGHT_CTRL_PRESSED    0x0004 // the right ctrl key is pressed.
#define LEFT_CTRL_PRESSED     0x0008 // the left ctrl key is pressed.
#define SHIFT_PRESSED         0x0010 // the shift key is pressed.
#define NUMLOCK_ON            0x0020 // the numlock light is on.
#define SCROLLLOCK_ON         0x0040 // the scrolllock light is on.
#define CAPSLOCK_ON           0x0080 // the capslock light is on.
#define ENHANCED_KEY          0x0100 // the key is enhanced.

typedef struct _MOUSE_EVENT_RECORD {
    COORD dwMousePosition;
    DWORD dwButtonState;
    DWORD dwControlKeyState;
    DWORD dwEventFlags;
} MOUSE_EVENT_RECORD, *PMOUSE_EVENT_RECORD;

//
// ButtonState flags
//

#define FROM_LEFT_1ST_BUTTON_PRESSED    0x0001
#define RIGHTMOST_BUTTON_PRESSED        0x0002
#define FROM_LEFT_2ND_BUTTON_PRESSED    0x0004
#define FROM_LEFT_3RD_BUTTON_PRESSED    0x0008
#define FROM_LEFT_4TH_BUTTON_PRESSED    0x0010

//
// EventFlags
//

#define MOUSE_MOVED   0x0001
#define DOUBLE_CLICK  0x0002

typedef struct _WINDOW_INFO_RECORD {
    SMALL_RECT Window;
} WINDOW_INFO_RECORD, *PWINDOW_INFO_RECORD;

typedef struct _INPUT_RECORD {
    WORD EventType;
    union {
        KEY_EVENT_RECORD KeyEvent;
        MOUSE_EVENT_RECORD MouseEvent;
        WINDOW_INFO_RECORD WindowInfoEvent;
    } Event;
} INPUT_RECORD, *PINPUT_RECORD;

//
//  EventType flags:
//

#define KEY_EVENT         0x0001 // Event contains key event record
#define MOUSE_EVENT       0x0002 // Event contains mouse event record
#define WINDOW_INFO_EVENT 0x0004 // Event contains window change event record

typedef struct _CHAR_INFO {
    union {
        WCHAR UnicodeChar;
        CHAR   AsciiChar;
    } Char;
    WORD Attributes;
} CHAR_INFO, *PCHAR_INFO;

//
// Attributes flags:
//

#define FOREGROUND_BLUE      0x0001 // text color contains blue.
#define FOREGROUND_GREEN     0x0002 // text color contains green.
#define FOREGROUND_RED       0x0004 // text color contains red.
#define FOREGROUND_INTENSITY 0x0008 // text color is intensified.
#define BACKGROUND_BLUE      0x0010 // background color contains blue.
#define BACKGROUND_GREEN     0x0020 // background color contains green.
#define BACKGROUND_RED       0x0040 // background color contains red.
#define BACKGROUND_INTENSITY 0x0080 // background color is intensified.


typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
    COORD dwSize;
    COORD dwCursorPosition;
    WORD  wAttributes;
    SMALL_RECT srWindow;
    COORD dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO, *PCONSOLE_SCREEN_BUFFER_INFO;

typedef struct _CONSOLE_CURSOR_INFO {
    DWORD  dwSize;
    BOOL   bVisible;
} CONSOLE_CURSOR_INFO, *PCONSOLE_CURSOR_INFO;

typedef struct _CONSOLE_FONT_INFO {
    DWORD  nFont;
    COORD  dwFontSize;
} CONSOLE_FONT_INFO, *PCONSOLE_FONT_INFO;


//
// typedef for ctrl-c handler routines
//

typedef
VOID
(*PHANDLER_ROUTINE)(
    IN ULONG CtrlType
    );

#define CTRL_C_EVENT 0
#define CTRL_BREAK_EVENT 1

//
//  Input Mode flags:
//

#define ENABLE_LINE_INPUT   0x0001
#define ENABLE_ECHO_INPUT   0x0002
#define ENABLE_WINDOW_INPUT 0x0004
#define ENABLE_MOUSE_INPUT  0x0008

//
// Output Mode flags:
//

#define ENABLE_LINE_OUTPUT  0x0001
#define ENABLE_WRAP_AT_EOL_OUTPUT  0x0002

//
// direct API definitions.
//

BOOL
PeekConsoleInput(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    );

BOOL
ReadConsoleInput(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    );

BOOL
WriteConsoleInput(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsWritten
    );

BOOL
ReadConsoleOutput(
    HANDLE hConsoleOutput,
    PCHAR_INFO lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpReadRegion
    );

BOOL
WriteConsoleOutput(
    HANDLE hConsoleOutput,
    PCHAR_INFO lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpWriteRegion
    );

BOOL
ReadConsoleOutputCharacter(
    HANDLE hConsoleOutput,
    LPSTR lpCharacter,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfCharsRead
    );

BOOL
ReadConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    LPWORD lpAttribute,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfAttrsRead
    );

BOOL
WriteConsoleOutputCharacter(
    HANDLE hConsoleOutput,
    LPSTR lpCharacter,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    );

BOOL
WriteConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    LPWORD lpAttribute,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfAttrsWritten
    );

BOOL
FillConsoleOutputCharacter(
    HANDLE hConsoleOutput,
    CHAR   cCharacter,
    DWORD  nLength,
    COORD  dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    );

BOOL
FillConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    WORD   wAttribute,
    DWORD  nLength,
    COORD  dwWriteCoord,
    LPDWORD lpNumberOfAttrsWritten
    );

BOOL
GetConsoleMode(
    HANDLE hConsoleHandle,
    LPDWORD lpMode
    );

BOOL
GetConsoleNumberOfInputEvents(
    HANDLE hConsoleInput,
    LPDWORD lpNumberOfEvents
    );

BOOL
GetConsoleScreenBufferInfo(
    HANDLE hConsoleOutput,
    PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo
    );

BOOL
GetConsoleCursorInfo(
    HANDLE hConsoleOutput,
    PCONSOLE_CURSOR_INFO lpConsoleCursorInfo
    );

BOOL
GetConsoleNumberOfMouseButtons(
    LPDWORD lpNumberOfMouseButtons
    );

BOOL
SetConsoleMode(
    HANDLE hConsoleHandle,
    DWORD dwMode
    );

BOOL
SetConsoleActiveScreenBuffer(
    HANDLE hConsoleOutput
    );

BOOL
FlushConsoleInputBuffer(
    HANDLE hConsoleInput
    );

BOOL
SetConsoleMaximumWindowSize(
    HANDLE hConsoleOutput,
    COORD dwWindowSize
    );

BOOL
SetConsoleScreenBufferSize(
    HANDLE hConsoleOutput,
    COORD dwSize
    );

BOOL
SetConsoleCursorPosition(
    HANDLE hConsoleOutput,
    COORD dwCursorPosition
    );

BOOL
SetConsoleCursorInfo(
    HANDLE hConsoleOutput,
    PCONSOLE_CURSOR_INFO lpConsoleCursorInfo
    );

BOOL
ScrollConsoleScreenBuffer(
    HANDLE hConsoleOutput,
    PSMALL_RECT lpScrollRectangle,
    PSMALL_RECT lpClipRectangle,
    COORD dwDestinationOrigin,
    PCHAR_INFO lpFill
    );

BOOL
SetConsoleWindowInfo(
    HANDLE hConsoleOutput,
    BOOL bAbsolute,
    PSMALL_RECT lpConsoleWindow
    );

BOOL
SetConsoleTextAttribute(
    HANDLE hConsoleOutput,
    WORD wAttributes
    );

BOOL
SetConsoleCtrlHandler(
    IN PHANDLER_ROUTINE HandlerRoutine,
    IN BOOL Add
    );
    
BOOL
AddAlias(
    LPSTR Source,
    LPSTR Target
    );

DWORD
GetAlias(
    LPSTR lpSource,
    LPSTR lpTarget,
    DWORD cchTarget
    );

BOOL
AllocConsole( VOID );

BOOL
FreeConsole( VOID );

BOOL
SetConsoleFont(
    HANDLE hConsoleOutput,
    DWORD nFont
    );

BOOL
GetConsoleCurrentFont(
    HANDLE hConsoleOutput,
    BOOL bMaximumWindow,
    PCONSOLE_FONT_INFO lpConsoleCurrentFont
    );

COORD
GetConsoleFontSize(
    HANDLE hConsoleOutput,
    DWORD nFont
    );

DWORD
GetConsoleFontInfo(
    HANDLE hConsoleOutput,
    BOOL bMaximumWindow,
    DWORD nLength,
    PCONSOLE_FONT_INFO lpConsoleFontInfo
    );

DWORD
GetConsoleNumberOfFonts(
    VOID
    );


DWORD
GetConsoleTitle(
    LPSTR lpConsoleTitle,
    DWORD nSize
    );

BOOL
SetConsoleTitle(
    LPSTR lpConsoleTitle
    );
