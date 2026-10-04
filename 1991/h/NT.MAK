# Win32 NMAKE definitions


!IF "$(CPU)" == "i386"

# Debug switches are default for current release
#
# These switches allow for source level debugging
# with NTSD for local and global variables.


CPUTYPE=1
cdebug = -Zd -Zi -Od
cvtomfdebug = -g

cc = cl386
cflags	= -c -As -G3d -MT -W3 -Di386=1 $(cdebug)

cvtobj = cvtomf $(cvtomfdebug) 
!ENDIF

!IF "$(CPU)" == "MIPS"
#declarations for use on self hosted MIPS box.

CPUTYPE=2
cc = cc
cflags	= -c -std -G0 -O -o $(*B).obj -EL -DMIPS=1 -D_MIPS_=1 -D_M_MRX000=1
cvtobj = mip2coff
!ENDIF

!IFNDEF CPUTYPE
!ERROR  Must specify CPU Environment Variable (i386 or MIPS )
!ENDIF


#Universal declaration

cvars = -DWIN32
linkdebug = -debug:full
link = link $(linkdebug)


# link flags - must be specified after $(link)
#
# natflags : creating a native application
# conflags : creating a character based console application
# guiflags : creating a GUI based "Windows" application
# os2flags : creating an OS/2 application

natflags =  -subsystem:native -entry:NtProcessStartup
conflags =  -subsystem:console -entry:BaseProcessStartup
guiflags =  -subsystem:windows -entry:BaseProcessStartup
os2flags =  -subsystem:os2 -entry:Os2ProcessStartup

# Link libraries - system import and C runtime libraries
#
# natlibs : libraries to link with for a "native" application
# conlibs : libraries to link with for a console application
# guilibs : libraries to link with for a "Windows" application
# os2libs : libraries to link with for an "OS/2" application
#
# note : $(LIB) is set in environment variables

natlibs = $(LIB)\base.lib $(LIB)\ntdll.lib $(LIB)\gre.lib	 \
	  $(LIB)\ntcrt.lib

conlibs = $(LIB)\base.lib $(LIB)\console.lib $(LIB)\ntdll.lib	 \
	  $(LIB)\wincrt.lib

guilibs = $(LIB)\gre.lib $(LIB)\user.lib $(LIB)\userrtl.lib	 \
	  $(LIB)\base.lib $(LIB)\ntdll.lib $(LIB)\wincrt.lib

os2libs = $(LIB)\base.lib $(LIB)\ntdll.lib $(LIB)\os2dll.lib	 \
	  $(LIB)\os2crt.lib


