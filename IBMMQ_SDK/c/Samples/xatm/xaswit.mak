#
# Name: XASWIT.MAK
#
# Description: Windows NT makefile for DB2, Oracle, Informix and Sybase XA switch load files
#
# <copyright notice="copyright-lm-source-program"
#            pids="5724-H72"
#            years="1996,2019"
#            crc="4293609679" >
#
# Licensed Materials - Property of IBM
#
# 5724-H72
#
# (C) Copyright IBM Corp. 1996, 2019
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with
# IBM Corp.
#
# </copyright>
#
# To make the DB2 XA switch load file, run the command:-
#                nmake -f xaswit.mak db2swit.dll
#
# To make the DB2 XA Static Registration switch load file, run the command:-
#                nmake -f xaswit.mak db2swits.dll
#
# To make the Sybase XA switch load file, run the command:-
#                nmake -f xaswit.mak sybswit.dll
#
# To make the Oracle XA switch load file, run the command:-
#                nmake -f xaswit.mak oraswit.dll
#
# To make the Oracle XA Dynamic Registration switch load file, run the command:-
#                nmake -f xaswit.mak oraswitd.dll
#
# To make the Informix XA switch load file, run the command:-
#                nmake -f xaswit.mak infswit.dll
#
# Note: The above commands will build a debug version of the switch load files,
#       to build a non-debug version invoke nmake with the nodebug=1 argument,
#       for example:-
#                nmake nodebug=1 -f xaswit.mak db2swit.dll
#
# Note: To build a 32-bit switch load file you can set the appropriate environment
#       by issuing: 'vcvarsall x86'; to build a 64-bit switch load file issue:
#       'vcvarsall amd64'. CPU variable is automatically set based on the
#       'Platform' environment variable. It is set to x64 if 'Platform' is set to
#       'X64' else it is set to 'x86'.
#
# Note: When building both the 32-bit and 64-bit switch load files delete the
#       old .obj and .lib files or use the -a argument, for example:-
#                nmake nodebug=1 -a -f xaswit.mak db2swit.dll
#
# Note: If your database library has a different name, or is on a different
#       drive, or is in a different directory to the one listed in the xxxLIBPATH
#       statement, you will need to alter that statement.
#

!if (defined(PROCESSOR_ARCHITEW6432) || "$(PROCESSOR_ARCHITECTURE)" == "AMD64")
MQ_DEFAULT_INSTALLATION_PATH=C:\Program Files (x86)\IBM\IBM MQ
MQ_DEFAULT_DATA_PATH=C:\ProgramData\IBM\IBM MQ
!else
MQ_DEFAULT_INSTALLATION_PATH=C:\Program Files\IBM\IBM MQ
MQ_DEFAULT_DATA_PATH=C:\ProgramData\IBM\IBM MQ
!endif

#-------------------------------------------------------------------------------
# If the default MQ/DB2/Informix installation directory has not been overridden,
# work it out.
#
# The default product install location differs depending on whether the
# operating system is 32-bit or 64-bit. We can determine this because on 64-bit
# operating systems, either PROCESSOR_ARCHITEW6432 is defined, or
# PROCESSOR_ARCHITECTURE is set to AMD64.
#
# The PROCESSOR_ARCHITECTURE environment variable being set to AMD64 indicates
# that a 64-bit compiler is in use. If this is the case, we build to the 64-bit
# exits directory (exits64) as opposed to the 32-bit one (exits).
#
# Note: If you have installed MQ/DB2/Informix into a different location to the
#       default, or otherwise want to override the location to which the product
#       exit is built, you can do so by setting the appropriate macro, either on
#       the command line, or before this point in the make file.
#
#-------------------------------------------------------------------------------
MQ_INSTALLATION_PATH=$(MQ_DEFAULT_INSTALLATION_PATH)
MQ_DATA_PATH=$(MQ_DEFAULT_DATA_PATH)

!if !defined(MQEXITDIR)
!if ("$(PLATFORM)" != "X64")
MQEXITDIR="$(MQ_DATA_PATH)\exits"
BIT="WIN32"
!else
MQEXITDIR="$(MQ_DATA_PATH)\exits64"
BIT="WIN64"
!endif
!endif

#------------------------------------------------------------------------------
# If CPU has not been set then set it now.
#------------------------------------------------------------------------------
!if !defined(MACHINE_TYPE)
!if ("$(PLATFORM)" != "X64")
MACHINE_TYPE=x86
!else
MACHINE_TYPE=x64
!endif
!endif

!if !defined(DB2LIBPATH)
!if ("$(PLATFORM)" != "X64")
DB2LIBPATH="C:\Program Files\IBM\SQLLIB\lib\Win32"
!else
DB2LIBPATH="C:\Program Files\IBM\SQLLIB\lib"
!endif
!endif

!if !defined(INFLIBPATH)
!if ("$(PLATFORM)" != "X64")
INFLIBPATH="C:\IBM\Informix\Client-SDK\32-bit\lib"
!else
INFLIBPATH="C:\IBM\Informix\Client-SDK\64-bit\lib"
!endif
!endif

# Uncomment only one set of ORALIBPATH statements
# Note the first default database instance Oracle creates has been used (db_1)
# for the following releases of Oracle, this may need changing if this instance
# name does not exist in the Oracle release sub directory.
!if !defined(ORALIBPATH)
!if (defined(PROCESSOR_ARCHITEW6432) || "$(PROCESSOR_ARCHITECTURE)" == "AMD64")
!if ("$(PROCESSOR_ARCHITECTURE)" != "AMD64")
# The following line is for Oracle 10gR1
# ORALIBPATH="C:\oracle\product\10.1.0\client_1\RDBMS\XA"
# The following line is for Oracle 10gR2
# ORALIBPATH="C:\oracle\product\10.2.0\client_1\RDBMS\XA"
# The following line is for Oracle 11gR1
# Note UserId will be the user identifier for the user which installed Oracle
# ORALIBPATH="C:\app\UserId\product\11.1.0\client_1\RDBMS\XA"
# The following line is for Oracle 12cR1 - note there is no 32-bit Oracle server
# Note UserId will be the user identifier for the user which installed Oracle
# ORALIBPATH="C:\app\UserId\product\12.1.0\client_1\RDBMS\XA"
# The following line is for Oracle 12cR2 - note there is no 32-bit Oracle server
# Note UserId will be the user identifier for the user which installed Oracle
# ORALIBPATH="C:\app\UserId\product\12.2.0\client_1\rdbms\xa"
# The following line is for Oracle 18c - note there is no 32-bit Oracle server
# Note UserId will be the user identifier for the user which installed Oracle
# ORALIBPATH="C:\app\UserId\product\18.3.0\client_1\rdbms\xa"
# The following line is for Oracle 19c - note there is no 32-bit Oracle server
# Note UserId will be the user identifier for the user which installed Oracle
ORALIBPATH="C:\app\UserId\product\19.3.0\client_1\rdbms\xa"
!else
# The following line is for Oracle 10gR1
# ORALIBPATH="C:\oracle\product\10.1.0\db_1\RDBMS\XA"
# The following line is for Oracle 10gR2
# ORALIBPATH="C:\oracle\product\10.2.0\db_1\RDBMS\XA"
# The following line is for Oracle 11gR1
# Note UserId will be the user identifier for the user which installed Oracle
# ORALIBPATH="C:\app\UserId\product\11.1.0\db_1\RDBMS\XA"
# The following line is for Oracle 12cR1
# Note UserId will be the user identifier for the user which installed Oracle
# ORALIBPATH="C:\app\UserId\product\12.1.0\dbhome_1\RDBMS\XA"
# The following line is for Oracle 12cR2
# Note UserId will be the user identifier for the user which installed Oracle
# ORALIBPATH="C:\app\UserId\product\12.2.0\dbhome_1\rdbms\xa"
# The following line is for Oracle 18c
# Note UserId will be the user identifier for the user which installed Oracle
# ORALIBPATH="C:\app\UserId\product\18.3.0\dbhome_1\rdbms\xa"
# The following line is for Oracle 19c
# Note UserId will be the user identifier for the user which installed Oracle
ORALIBPATH="C:\app\UserId\product\19.3.0\dbhome_1\rdbms\xa"
!endif
!else
# The following line is for Oracle 10gR1
# ORALIBPATH="C:\oracle\product\10.1.0\db_1\RDBMS\XA"
# The following line is for Oracle 10gR2
# ORALIBPATH="C:\oracle\product\10.2.0\db_1\RDBMS\XA"
# The following line is for Oracle 11gR1
# Note UserId will be the user identifier for the user which installed Oracle
# ORALIBPATH="C:\app\UserId\product\11.1.0\db_1\RDBMS\XA"
# The following line is for Oracle 12cR1 - note there is no 32-bit Oracle server
# Note UserId will be the user identifier for the user which installed Oracle
# ORALIBPATH="C:\app\UserId\product\12.1.0\client_1\RDBMS\XA"
# The following line is for Oracle 12cR2 - note there is no 32-bit Oracle server
# Note UserId will be the user identifier for the user which installed Oracle
# ORALIBPATH="C:\app\UserId\product\12.2.0\client_1\rdbms\xa"
# The following line is for Oracle 18c - note there is no 32-bit Oracle server
# Note UserId will be the user identifier for the user which installed Oracle
# ORALIBPATH="C:\app\UserId\product\18.3.0\client_1\rdbms\xa"
# The following line is for Oracle 19c - note there is no 32-bit Oracle server
# Note UserId will be the user identifier for the user which installed Oracle
ORALIBPATH="C:\app\UserId\product\19.3.0\client_1\rdbms\xa"
!endif
!endif

all: db2swit.dll db2swits.dll sybswit.dll oraswitd.dll oraswitd.dll infswit.dll

!if ("$(nodebug)" != "1")
DBGFLAG=-Zi -MDd
!else
DBGFLAG=-MD
!endif

#-------------------------------------------------------------------------------
# Compiler flags
#-------------------------------------------------------------------------------
CFLAGS=$(DBGFLAG) -nologo -Od -W3 -DWINVER=0x0600 -D_WIN32_WINNT=0x0600 -D $(BIT) \
       -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D "_WINDLL" -D "_USRDLL"

#-------------------------------------------------------------------------------
# Linker flags
#-------------------------------------------------------------------------------
LFLAGS=-MANIFEST -DLL -SUBSYSTEM:WINDOWS -fixed:no -machine:$(MACHINE_TYPE)

#-------------------------------------------------------------------------------
# DB2 XA switch load file
#-------------------------------------------------------------------------------
DB2LIBS=db2api.lib

DB2LIBFLAGS=$(DB2LIBPATH)\$(DB2LIBS)

db2swit.dll:db2swit.c
 cl  $(CFLAGS) -Fe$@ $**                                            \
    -link $(LFLAGS) -out:$(MQEXITDIR)\db2swit.dll $(DB2LIBFLAGS) && \
    mt.exe -nologo -manifest $(MQEXITDIR)\db2swit.dll.manifest      \
    -outputresource:$(MQEXITDIR)\db2swit.dll

#-------------------------------------------------------------------------------
# DB2 XA Static Registration switch load file
#-------------------------------------------------------------------------------
db2swits.dll:db2swits.c
 cl $(CFLAGS) -Fe$@ $**                                               \
    -link $(LFLAGS) -out:$(MQEXITDIR)\db2swits.dll $(DB2LIBFLAGS) &&  \
    mt.exe -nologo -manifest $(MQEXITDIR)\db2swits.dll.manifest       \
    -outputresource:$(MQEXITDIR)\db2swits.dll

#-------------------------------------------------------------------------------
# Sybase XA switch file
#-------------------------------------------------------------------------------

# Uncomment only one pair of SYBLIBS/SYBLIBPATH statements

# The following line is for Sybase 12.5
# SYBLIBS=libxadtm.lib
# SYBLIBPATH=C:\sybase\OCS-12_5\lib

# The following line is for Sybase 15
SYBLIBS=libsybxadtm.lib
SYBLIBPATH=C:\sybase\OCS-15_0\lib

SYBLIBFLAGS=$(SYBLIBPATH)\$(SYBLIBS)

sybswit.dll:sybswit.c
 cl $(CFLAGS) -Fe$@ $**                                              \
    -link $(LFLAGS) -out:$(MQEXITDIR)\sybswit.dll $(SYBLIBFLAGS) &&  \
    mt.exe -nologo -manifest $(MQEXITDIR)\sybswit.dll.manifest       \
    -outputresource:$(MQEXITDIR)\sybswit.dll

#-------------------------------------------------------------------------------
# Oracle XA switch load file
#-------------------------------------------------------------------------------

# Uncomment only one ORALIBS statement

# The following line is for Oracle 9
# ORALIBS=oraxa9.lib

# The following line is for Oracle 10
# ORALIBS=oraxa10.lib

# The following line is for Oracle 11
# ORALIBS=oraxa11.lib

# The following line is for Oracle 12
# ORALIBS=oraxa12.lib

# The following line is for Oracle 18
# ORALIBS=oraxa18.lib

# The following line is for Oracle 19
ORALIBS=oraxa19.lib

ORALIBFLAGS=$(ORALIBPATH)\$(ORALIBS)

oraswit.dll:oraswit.c
 cl $(CFLAGS) -Fe$@ $**                                              \
    -link $(LFLAGS) -out:$(MQEXITDIR)\oraswit.dll $(ORALIBFLAGS) &&  \
    mt.exe -nologo -manifest $(MQEXITDIR)\oraswit.dll.manifest       \
    -outputresource:$(MQEXITDIR)\oraswit.dll

#-------------------------------------------------------------------------------
# Oracle XA Dynamic Registration switch load file
#-------------------------------------------------------------------------------
oraswitd.dll:oraswitd.c
 cl $(CFLAGS) -Fe$@ $**                                              \
    -link $(LFLAGS) -out:$(MQEXITDIR)\oraswitd.dll $(ORALIBFLAGS) && \
    mt.exe -nologo -manifest $(MQEXITDIR)\oraswitd.dll.manifest      \
    -outputresource:$(MQEXITDIR)\oraswitd.dll

#-------------------------------------------------------------------------------
# Informix XA switch load file
#-------------------------------------------------------------------------------
INFLIBS=isqlt09a.lib

INFLIBFLAGS=$(INFLIBPATH)\$(INFLIBS)

infswit.dll:infswit.c
 cl $(CFLAGS) -Fe$@ $**                                              \
    -link $(LFLAGS) -out:$(MQEXITDIR)\infswit.dll $(INFLIBFLAGS) &&  \
    mt.exe -nologo -manifest $(MQEXITDIR)\infswit.dll.manifest       \
    -outputresource:$(MQEXITDIR)\infswit.dll

#-------------------------------------------------------------------------------

