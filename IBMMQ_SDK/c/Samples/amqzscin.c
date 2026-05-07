const static char sccsid[] = "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/oltp/pc/winnt/amqzscin.c";
/******************************************************************************/
/*                                                                            */
/* Module Name: amqzscin.c                                                    */
/*                                                                            */
/* Description: IBM MQ XA switch program for CICS                   */
/*              XA Initialisation                                             */
/*                                                                            */
/*   <copyright                                                               */
/*   notice="lm-source-program"                                               */
/*   pids="5724-H72"                                                          */
/*   years="1994,2018"                                                        */
/*   crc="3341698670" >                                                       */
/*   Licensed Materials - Property of IBM                                     */
/*                                                                            */
/*   5724-H72,                                                                */
/*                                                                            */
/*   (C) Copyright IBM Corp. 1994, 2018 All Rights Reserved.                  */
/*                                                                            */
/*   US Government Users Restricted Rights - Use, duplication or              */
/*   disclosure restricted by GSA ADP Schedule Contract with                  */
/*   IBM Corp.                                                                */
/*   </copyright>                                                             */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Function:                                                                  */
/*                                                                            */
/* amqzscin.c is the XA Initialisation routine for CICS.                      */
/* It contains the following functions:                                       */
/*  CICS_XA_Init  -  CICS entry point for XA initialisation                   */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/* Note that the pre-build switch load file (Server: mqmc4swi.dll; Client:    */
/* mqcc4swi.dll) is built for CICS V4.3, to build a CICS V5.0 switch load     */
/* file, do the following:                                                    */
/*                                                                            */
/* For IBM WebSphere MQ V5.3 Server:                                */
/*                                                                            */
/* create a file called amqzsc5.def with the following in:                    */
/*                                                                            */
/*    LIBRARY AMQZSC5                                                         */
/*    EXPORTS                                                                 */
/*    CICS_XA_Init                                                            */
/*                                                                            */
/* Then issue the following compiler statement if using Microsoft Visual C++  */
/* 6.0:                                                                       */
/*                                                                            */
/*    cl -c -Gz -GD amqzscin.c -Ic:\opt\encina\include                        */
/*                                                                            */
/* or the following compiler statement if running Microsoft Visual C++ .NET   */
/* 7.x:                                                                       */
/*                                                                            */
/*    cl -c -Gz amqzscin.c -Ic:\opt\encina\include                            */
/*                                                                            */
/* and then the following link statement:                                     */
/*                                                                            */
/*    link -nod -dll -out:amqzsc5.dll -def:amqzsc5.def amqzscin.obj \         */
/*         c:\opt\cics\lib\regxa_swxa.obj \                                   */
/*         c:\opt\cics\lib\libcicsrt.lib \                                    */
/*         c:\opt\encina\lib\libEncina.lib \                                  */
/*         c:\opt\encina\lib\libEncServer.lib \                               */
/*         "c:\Program Files\dce\dcelocal\lib\libdce.lib" \                   */
/*         "c:\Program Files\dce\dcelocal\lib\pthreads.lib" \                 */
/*         "c:\Program Files\IBM\WebSphere MQ\Tools\Lib\mqmcics4.lib" \       */
/*         msvcrt.lib kernel32.lib                                            */
/*                                                                            */
/* For IBM MQ Extended Transactional Client:                                  */
/*                                                                            */
/* create a file called amqczsc5.def with the following in:                   */
/*                                                                            */
/*    LIBRARY AMQCZSC5                                                        */
/*    EXPORTS                                                                 */
/*    CICS_XA_Init                                                            */
/*                                                                            */
/* Then issue the following compiler statement if using Microsoft Visual C++  */
/* 6.0:                                                                       */
/*                                                                            */
/*    cl -c -Gz -GD amqzscin.c -Ic:\opt\encina\include                        */
/*                                                                            */
/* or the following compiler statement if running Microsoft Visual C++ .NET   */
/* 7.x:                                                                       */
/*                                                                            */
/*    cl -c -Gz amqzscin.c -Ic:\opt\encina\include                            */
/*                                                                            */
/* and then the following link statement:                                     */
/*                                                                            */
/*    link -nod -dll -out:amqczsc5.dll -def:amqczsc5.def amqzscin.obj \       */
/*         c:\opt\cics\lib\regxa_swxa.obj \                                   */
/*         c:\opt\cics\lib\libcicsrt.lib \                                    */
/*         c:\opt\encina\lib\libEncina.lib \                                  */
/*         c:\opt\encina\lib\libEncServer.lib \                               */
/*         "c:\Program Files\dce\dcelocal\lib\libdce.lib" \                   */
/*         "c:\Program Files\dce\dcelocal\lib\pthreads.lib" \                 */
/*         "c:\Program Files\IBM\WebSphere MQ\Tools\Lib\mqccics4.lib" \       */
/*         msvcrt.lib kernel32.lib                                            */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

/******************************************************************************/
/* Select compiler specific header files                                      */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <cmqc.h>

/******************************************************************************/
/* Select XA header file                                                      */
/******************************************************************************/
#include <tmxa/xa.h>

/******************************************************************************/
/* NB: From Visual Studio 2015 stdin, stdout and stderr are declared inline.  */
/*     Pre-compiled objects or libraries built using an earlier VS will       */
/*     therefore produce link errors when used with VS2015+.                  */
/*                                                                            */
/*     See: https://msdn.microsoft.com/en-us/library/bb531344.aspx#BK_CRT     */
/*                                                                            */
/*     The following code redefines the required _iob_func() for these        */
/*     pre-compiled objects or libraries.                                     */
/******************************************************************************/
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
  #ifdef __cplusplus
    extern "C" {
  #endif
  FILE _iob[3] = { NULL };
  FILE * __cdecl __iob_func(void) {
    _iob[0] = *stdin; _iob[1] = *stdout; _iob[2] = *stderr;
  return _iob; }
  #ifdef __cplusplus
    }
  #endif
#endif

/******************************************************************************/
/* External Data declarations                                                 */
/******************************************************************************/
#define CALLCONV __cdecl
#define DLLIMPORT __declspec(dllimport)
extern DLLIMPORT struct xa_switch_t MQRMIXASwitch;
extern DLLIMPORT struct xa_switch_t MQRMIXASwitchDynamic;
extern struct xa_switch_t RegXA_xa_switch;
extern struct xa_switch_t *cics_xa_switch;

/******************************************************************************/
/* Data defined in this module                                                */
/******************************************************************************/

/******************************************************************************/
/* Local function declarations                                                */
/******************************************************************************/

extern void MQENTRY AMQCICSINIT(void *);
extern void CALLCONV cics_xa_init(void);

/******************************************************************************/
/* Function Definitions                                                       */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* Function Name:  amqzscin                                                   */
/*                                                                            */
/* Description: CICS XA initialisation.                                       */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Function:                                                                  */
/*                                                                            */
/*  amqzscin is called by CICS to initialise XA support for IBM MQ            */
/*                                                                            */
/* Input Parameters:  None                                                    */
/*                                                                            */
/* Output Parameters: None                                                    */
/*                                                                            */
/* InOut Parameters:  None                                                    */
/*                                                                            */
/* Returns:           RegXA_xa_switch                                         */
/*                                                                            */
/******************************************************************************/
struct xa_switch_t * CALLCONV CICS_XA_Init(void)
{
  /****************************************************************************/
  /* Set CICS pointer to our structure                                        */
  /****************************************************************************/
  cics_xa_switch = &MQRMIXASwitchDynamic;

  /****************************************************************************/
  /* Initialise CICS support                                                  */
  /****************************************************************************/
  cics_xa_init();

  /****************************************************************************/
  /* Tell MQ that CICS is available.                                          */
  /* This call will cause MQ to callback to CICS on any MQCONN to determine   */
  /* the CICS UserId and transaction name, so that it can be used in          */
  /* authority checking and context.                                          */
  /* For compatability with OS/2, a dummy parameter is given.                 */
  /****************************************************************************/
  AMQCICSINIT(NULL);

  /****************************************************************************/
  /* Return from function                                                     */
  /****************************************************************************/
  return(&RegXA_xa_switch);

}

/******************************************************************************/
/*                                                                            */
/* End of amqzscin.c                                                          */
/*                                                                            */
/******************************************************************************/
/* MQMBID Start */
#ifndef __MQMBID_H
  #define __MQMBID_H
  #if defined(__MQMBID_H)
   /* Pointless code to prevent compiler warnings about __MQMBID_H being unused */
  #endif
  #if defined(_AIX)
   #if defined(__clang__)
    static char * __attribute__((used)) MQMBID(void) { return (char *)"@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/oltp/pc/winnt/amqzscin.c"; }
   #else
    #pragma comment(user, "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/oltp/pc/winnt/amqzscin.c")
   #endif
  #elif defined(_LINUX_2) || defined(linux) || defined(__linux) || defined(AMQ_MACOS) || defined(DARWIN) || defined(__APPLE__)
    static char * __attribute__((used)) MQMBID(void) { return (char *)"@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/oltp/pc/winnt/amqzscin.c"; }
  #elif defined(WIN32) || defined(_WIN32)
    #pragma optimize("", off)
    #if defined(__cplusplus)
    static __inline char * MQMBIDFn(char * s) { return s; };
    const static char * MQMBID=MQMBIDFn((char *)"@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/oltp/pc/winnt/amqzscin.c");
    #else
    const static char MQMBID[]="@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/oltp/pc/winnt/amqzscin.c";
    #endif
    #pragma optimize("", on)
  #else
    static char * MQMBID(void) { return (char *)"@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/oltp/pc/winnt/amqzscin.c"; }
  #endif
#endif
/* MQMBID End */
