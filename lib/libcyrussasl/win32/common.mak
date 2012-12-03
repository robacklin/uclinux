#Can this be autogenerated?
#Keep in sync with include/sasl.h and win32/include/config.h
SASL_VERSION_MAJOR=2
SASL_VERSION_MINOR=1
SASL_VERSION_STEP=23

# Uncomment the following line, if you want to use Visual Studio 6
#VCVER=6

# Define compiler/linker/etc.

CPP=cl.exe /nologo
LINK32=link.exe /nologo
LINK32DLL=$(LINK32) /dll
LINK32EXE=$(LINK32)

SYS_LIBS=ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib

# Define the minimal Windows OS you want to run on:40 (NT), 50 (W2K), 51 (XP)
# Default is no restrictions. Currently we only check for 51 or later.
#TARGET_WIN_SYSTEM=51

!IF "$(TARGET_WIN_SYSTEM)" == ""
!IF "$(VERBOSE)" != "0"
!MESSAGE Applications and libraries should run on any Win32 system.
!ENDIF
TARGET_WIN_SYSTEM=0
!ENDIF

# prefix variable is currently only being used by install target
!IF "$(prefix)" == ""
prefix=C:\CMU
!IF "$(VERBOSE)" != "0"
!MESSAGE Default installation directory is $(prefix).
!ENDIF 
!ENDIF

!IF "$(CFG)" == ""
CFG=Release
!IF "$(VERBOSE)" != "0"
!MESSAGE No configuration specified. Defaulting to $(CFG).
!ENDIF
!ENDIF 

!IF "$(DB_LIB)" == ""
DB_LIB=libdb41s.lib
!IF "$(VERBOSE)" != "0"
!MESSAGE Defaulting SleepyCat library name to $(DB_LIB).
!ENDIF
!ENDIF

!IF "$(DB_INCLUDE)" == ""
DB_INCLUDE=c:\work\isode\db\build_win32
!IF "$(VERBOSE)" != "0"
!MESSAGE Defaulting SleepyCat include path to $(DB_INCLUDE).
!ENDIF
!ENDIF

!IF "$(DB_LIBPATH)" == ""
DB_LIBPATH=c:\work\isode\db\build_win32\Release_static
!IF "$(VERBOSE)" != "0"
!MESSAGE Defaulting SleepyCat library path to $(DB_LIBPATH).
!ENDIF
!ENDIF

!IF "$(OPENSSL_INCLUDE)" == ""
OPENSSL_INCLUDE="D:\openssl\engine-0.9.6g-md3\include"
!IF "$(VERBOSE)" != "0"
!MESSAGE Defaulting OpenSSL Include path to $(OPENSSL_INCLUDE).
!ENDIF
!ENDIF

!IF "$(OPENSSL_LIBPATH)" == ""
OPENSSL_LIBPATH="D:\openssl\engine-0.9.6g-md3\lib"
!IF "$(VERBOSE)" != "0"
!MESSAGE Defaulting OpenSSL library path to $(OPENSSL_LIBPATH).
!ENDIF
!ENDIF

!IF "$(GSSAPI_INCLUDE)" == ""
GSSAPI_INCLUDE="C:\Program Files\CyberSafe\Developer Pack\ApplicationSecuritySDK\include"
!IF "$(VERBOSE)" != "0"
!MESSAGE Defaulting GSSAPI Include path to $(GSSAPI_INCLUDE).
!ENDIF
!ENDIF

!IF "$(GSSAPI_LIBPATH)" == ""
GSSAPI_LIBPATH="C:\Program Files\CyberSafe\Developer Pack\ApplicationSecuritySDK\lib"
!IF "$(VERBOSE)" != "0"
!MESSAGE Defaulting GSSAPI library path to $(GSSAPI_LIBPATH).
!ENDIF
!ENDIF

!IF "$(SQLITE_INCLUDE)" == ""
SQLITE_INCLUDES=/I"C:\work\open_source\sqllite\sqlite\src" /I"C:\work\open_source\sqllite\sqlite\win32"
!IF "$(VERBOSE)" != "0"
!MESSAGE Defaulting SQLITE_INCLUDES includes to $(SQLITE_INCLUDES).
!ENDIF
!ENDIF

!IF "$(SQLITE_LIBPATH)" == ""
SQLITE_LIBPATH="C:\work\open_source\sqllite\sqlite\objs"
!IF "$(VERBOSE)" != "0"
!MESSAGE Defaulting SQLITE library path to $(SQLITE_LIBPATH).
!ENDIF
!ENDIF

!IF "$(LDAP_LIB_BASE)" == ""
LDAP_LIB_BASE = c:\work\open_source\openldap\openldap-head\ldap\Debug
!IF "$(VERBOSE)" != "0"
!MESSAGE Defaulting LDAP library path to $(LDAP_LIB_BASE).
!ENDIF
!ENDIF

!IF "$(LDAP_INCLUDE)" == ""
LDAP_INCLUDE = c:\work\open_source\openldap\openldap-head\ldap\include
!IF "$(VERBOSE)" != "0"
!MESSAGE Defaulting LDAP include path to $(LDAP_INCLUDE).
!ENDIF
!ENDIF

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF


!IF  "$(CFG)" == "Release"

!IF "$(CODEGEN)" == ""
!IF "$(STATIC)" == "yes"
CODEGEN=/MT
!ELSE
CODEGEN=/MD
!ENDIF 
!IF "$(VERBOSE)" != "0"
!MESSAGE Codegeneration defaulting to $(CODEGEN).
!ENDIF 
!ENDIF 

!IF "$(VCVER)" != "6"
ENABLE_WIN64_WARNINGS=/Wp64
!ENDIF

CPP_PROJ= $(CODEGEN) /W3 /GX /O2 $(ENABLE_WIN64_WARNINGS) /Zi /D "NDEBUG" $(CPPFLAGS) /FD /c 

LINK32_FLAGS=/incremental:no /debug /machine:I386

!ELSEIF  "$(CFG)" == "Debug"

!IF "$(CODEGEN)" == ""
!IF "$(STATIC)" == "yes"
CODEGEN=/MTd
!ELSE
CODEGEN=/MDd
!ENDIF 
!IF "$(VERBOSE)" != "0"
!MESSAGE Codegeneration defaulting to $(CODEGEN).
!ENDIF 
!ENDIF 

CPP_PROJ=$(CODEGEN) /W3 /Gm /GX /ZI /Od /D "_DEBUG" $(CPPFLAGS) /FD /GZ /c 

LINK32_FLAGS=/incremental:yes /debug /machine:I386 /pdbtype:sept 

!ENDIF

LINK32DLL_FLAGS=$(LINK32_FLAGS) $(SYS_LIBS) $(EXTRA_LIBS)

# Assume we are only building console applications
LINK32EXE_FLAGS=/subsystem:console $(LINK32_FLAGS) $(SYS_LIBS) $(EXTRA_LIBS)

