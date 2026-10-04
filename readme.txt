r3000jazz — Running the 1991 Windows NT/MIPS Toolchain on Modern Windows
This project started as an R3000/Jazz emulation experiment and accidentally became something considerably stranger:
The original pre-release Microsoft Windows NT MIPS compiler and linker toolchain can now run directly on modern x64 Windows and compile new NT/MIPS executables.

The recovered compiler binaries are native MIPS R3000 Windows NT executables from 1991. Modern Windows cannot execute them, and they predate the public release of Windows NT.
mips-tool provides a small user-mode NT/MIPS compatibility environment around the project's R3000/R3010 CPU core. It loads the original MIPS PE executables, emulates their CPU instructions, implements the subset of early Win32/NT services they require, and recursively runs child MIPS tools when the compiler invokes them.
No recompilation or source code for the historical compiler is required.
What works
The original compiler driver now controls its real compiler pipeline:
              CC.EXE
                |
                v
             CFE.EXE
                |
                v
             UOPT.EXE
                |
                v
             UGEN.EXE
                |
                v
              AS1.EXE
                |
                v
        MIPS ECOFF object
                |
                v
          MIP2COFF.EXE
                |
                v
        NT/MIPS COFF object
                |
                v
             LINK.EXE
                |
                v
             COFF.EXE
                |
                v
         Windows NT/MIPS PE

These are the original MIPS binaries executing R3000 code, not ports of the compiler passes.
The compatibility layer implements enough of their early Win32 environment for file I/O, virtual memory, environment handling, temporary files, process creation, child-process waiting, command lines and other runtime services.
CreateProcess is particularly important: when CC.EXE asks Windows to execute CFE.EXE, mips-tool transparently starts another MIPS personality instance. The historical compiler therefore drives its own toolchain exactly as intended.
The proof
A normal historical NMAKE recipe can now build a program:
Microsoft (R) Program Maintenance Utility   Version 1.40
Copyright (c) Microsoft Corp 1988-93. All rights reserved.

        mips-tool cc.exe -c -std -G0 -O -o wincon.obj ...
        mips-tool mip2coff.exe wincon.obj
        mips-tool link.exe -debug:full -subsystem:console ...
Microsoft Windows NT Linker Version 2.2

The resulting executable can immediately be run through the same compatibility environment:
C:\>mips-tool wincon.exe
printf says hi
win32 says hi

That executable was produced from C source by the recovered 1991 MIPS compiler.
An earlier linker experiment reproduced a historical reference WINCON.EXE byte-for-byte except for its four-byte COFF timestamp, providing a particularly strong validation of the emulated execution and toolchain behavior.
Building
The current host target is Windows with MinGW/MSYS2.
make clean
make all

The most useful executable is:
mips-tool.exe

The historical MIPS tools themselves are not embedded. Place the recovered tools where they can be found by the compiler, for example:
bin/
    mips-tool.exe
    CC.EXE
    CFE.EXE
    UOPT.EXE
    UGEN.EXE
    AS0.EXE
    AS1.EXE
    MIP2COFF.EXE
    LINK.EXE
    COFF.EXE
    ERR.CC

h/
    ...

lib/
    BASE.LIB
    CONSOLE.LIB
    NTDLL.LIB
    WINCRT.LIB
    ...

The original environment variables are useful:
set INCLUDE=h
set LIB=lib
set TMP=c:\temp
set TEMP=c:\temp

Then, for example:
mips-tool cc.exe -c hello.c

Runner options go before the guest executable name. Everything after the executable is passed unchanged to the historical program.
mips-tool -v cc.exe -c hello.c
mips-tool -vv cc.exe -c hello.c

-v enables useful loader/process diagnostics.
-vv enables the full host-call trace and is intended for archaeology/debugging.
Normal execution is deliberately quiet so that:
mips-tool program.exe

behaves much like running the program natively.
A note about MIP2COFF
This toolchain has one particularly easy trap.
AS1.EXE does not directly produce the final NT object format. Its output is MIPS ECOFF and must be passed through:
mips-tool mip2coff.exe foo.obj

before being given to the NT linker.
Skipping this step can be spectacularly confusing: COFF.EXE recognizes enough of the structure to get surprisingly far before interpreting ECOFF symbol information as NT COFF data and eventually wandering into nonsense.
Ask how we found that one.
Phoon
A useful stress test has been the classic phoon program.
It already exercises considerably more of the recovered compiler than the tiny WINCON test. A recent build successfully passed:
CFE
UOPT
UGEN
AS1
MIP2COFF
multi-object COFF linking

including a large compilation in which CFE executed more than 60 million emulated R3000 instructions.
The current Phoon link reaches a perfectly legitimate linker error:
astro.obj() : warning 0516: asin is undefined
astro.obj() : warning 0516: atan is undefined
astro.obj() : warning 0516: atan2 is undefined
astro.obj() : warning 0516: cos is undefined
astro.obj() : warning 0516: floor is undefined
astro.obj() : warning 0516: sin is undefined
astro.obj() : warning 0516: sqrt is undefined
astro.obj() : warning 0516: tan is undefined

phoon.exe() : error 0120: 8 Undefined Externals

That is good news.
The compiler, object converter and multi-object linker are working. The next missing piece is the historical MIPS math runtime/libm equivalent and, inevitably, more R3010 floating-point coverage.
Why 64 MiB?
During linker debugging, the user-mode address-space jar was expanded from 32 MiB to 64 MiB.
That experiment exposed an important toolchain detail rather than merely fixing the crash: the offending objects had accidentally been fed to the NT linker in AS1's intermediate ECOFF form without first running MIP2COFF.
The larger address space remains useful for running these comparatively heavyweight development tools.
What this is not
This is not Windows NT emulation.
It does not boot NT, emulate the NT kernel, or attempt to implement Win32 generally.
mips-tool is deliberately a narrow user-mode compatibility personality:
1991 NT/MIPS application
          |
          v
     R3000/R3010
      emulation
          |
          v
   early Win32/NT API
      personality
          |
          v
     modern host OS

The API surface exists because the recovered development tools need it.
That narrow scope is what makes the experiment practical.
Why?
Because the alternative was leaving some fascinating software trapped inside a thirty-five-year-old operating system.
These tools come from the period when Microsoft was building the first versions of Windows NT for RISC machines. They contain a mixture of MIPS/Unix compiler ancestry, early Win32 assumptions, transitional object formats and pre-release NT conventions.
Running them exposes those layers directly.
Along the way we've encountered things such as:
- Unix-style /comp.config configuration probing;
- /usr/include compiler ancestry;
- Unix-style scratch files such as /2;
- mixed C:\TEMP/foo paths;
- ECOFF → NT COFF conversion;
- early Win32 CreateProcess orchestration;
- compiler passes spawning compiler passes;
- a linker that can execute tens of millions of R3000 instructions before anyone notices the emulator had an arbitrary safety limit.
It is software archaeology where the artifacts run.
Status
The important milestone is now live-proven on Windows:
C source
   ↓
original 1991 Microsoft MIPS compiler
   ↓
original optimizer/code generator/assembler
   ↓
MIP2COFF
   ↓
original Microsoft NT linker
   ↓
new NT/MIPS executable
   ↓
R3000 user-mode compatibility environment
   ↓
working program

And, separately, those generated binaries can be taken back toward the resurrected Jazz/NT environment that motivated this work in the first place.
Credits
This project stands on decades of compiler, emulator, Windows NT and MIPS archaeology by many people.
The current compatibility work is less about replacing the historical software than giving it somewhere to run again.