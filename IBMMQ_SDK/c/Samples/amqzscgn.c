const static char sccsid[] = "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/oltp/pc/winnt/amqzscgn.c";
/*********************************************************************/
/*                                                                   */
/* Module Name: amqzscgn.c                                           */
/*                                                                   */
/* Description: IBM MQ sample CICS GLobal User Exit (GLUE) program   */
/*              for the CICS Task termination user exit (UE014015)   */
/*   <copyright                                                      */
/*   notice="lm-source-program"                                      */
/*   pids="5724-H72,"                                                */
/*   years="1996,2016"                                               */
/*   crc="2849090112" >                                              */
/*   Licensed Materials - Property of IBM                            */
/*                                                                   */
/*   5724-H72,                                                       */
/*                                                                   */
/*   (C) Copyright IBM Corp. 1996, 2016 All Rights Reserved.         */
/*                                                                   */
/*   US Government Users Restricted Rights - Use, duplication or     */
/*   disclosure restricted by GSA ADP Schedule Contract with         */
/*   IBM Corp.                                                       */
/*   </copyright>                                                    */
/*********************************************************************/
/*                                                                   */
/* Function:                                                         */
/*                                                                   */
/* This sample is the IBM MQ CICS Task termination user exit         */
/* (UE014015) program.                                               */
/*                                                                   */
/* It is based on the samples in the CICS documentation, and         */
/* contains the following function:                                  */
/*  cics_UE_entry  -  CICS UE entry                                  */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/* Use the cl command to build amqzscgn.obj by compiling using at    */
/* least the following:                                              */
/* cl -c -I<YourCicsPath>\include -Gz -LD amqzscgn.c                 */
/*                                                                   */
/* Create a module definition file named mqmc1415.def containing     */
/* 3 lines of:                                                       */
/*   LIBRARY MQMC1415                                                */
/*   EXPORTS                                                         */
/*   cics_UE_entry=cics_UE_entry@8                                   */
/*                                                                   */
/* Use the lib command to build an export file & import library      */
/* using at least the following:                                     */
/* lib -def:mqmc1415.def -out:mqmc1415.lib                           */
/*                                                                   */
/* If the lib command is successful then an mqmc1415.exp will also   */
/* have been built.                                                  */
/*                                                                   */
/* Use the link command to build mqmc1415.dll by linking using at    */
/* least the following:                                              */
/* link -dll -nod -out:mqmc1415.dll amqzscgn.obj mqmc1415.exp        */
/*   mqmcics4.lib msvcrt.lib kernel32.lib                            */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/* Includes                                                          */
/*********************************************************************/

/*********************************************************************/
/* Select compiler specific header files                             */
/*********************************************************************/

#include <stdio.h>
#include <errno.h>

/*********************************************************************/
/* Select CICS include files                                         */
/*********************************************************************/

#include <cicstype.h>
#include <cicsue.h>

/*********************************************************************/
/* Select MQI header file                                            */
/*********************************************************************/

#include <cmqc.h>

/*********************************************************************/
/* Data defined in this module                                       */
/*********************************************************************/

/*********************************************************************/
/* Local function declarations                                       */
/*********************************************************************/
extern void MQENTRY AMQ2PHASETASKDETACH(void);
extern void MQENTRY AMQ2PHASETASKABEND(void);

/*********************************************************************/
/* Function Definitions                                              */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/* Function Name:  cics_UE_entry                                     */
/*                                                                   */
/* Description: CICS Task termination user exit (UE014015)           */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/* Function:                                                         */
/*                                                                   */
/*  When the task termination exit number 15 is enabled on CICS,     */
/*  CICS calls cics_UE_entry whenever a task terminates.             */
/*                                                                   */
/* Input Parameters:  UE_Header(cics_UE_Header_t *)                  */
/*                    - Exit generic information                     */
/*                                                                   */
/*                    UE_Specific(cics_UE014015_t *)                 */
/*                    - Exit specific information                    */
/*                                                                   */
/* Output Parameters: None                                           */
/*                                                                   */
/* InOut Parameters:  None                                           */
/*                                                                   */
/* Returns:           UE_HeaderVersion                               */
/*                    UE_SpecificVersion                             */
/*                    UE_Normal                                      */
/*                                                                   */
/*********************************************************************/
cics_UE_Return_t cics_UE_entry(cics_UE_Header_t *UE_Header,
                               cics_UE014015_t  *UE_Specific)
{
 cics_UE_Return_t rc = UE_HeaderVersion;

 /********************************************************************/
 /* Check we understand the versions of the structures               */
 /********************************************************************/
 if (UE_Header->UE_Version == cics_UE_HEADER_VERSION)
 {
    rc = UE_SpecificVersion;
    if (UE_Specific->UE_Version == cics_UE014015_VERSION)
    {
     /****************************************************************/
     /* Determine the type of task termination                       */
     /****************************************************************/
     if (UE_Specific->UE_Terminationtype == UE_Normaltermination)
     {
       /**************************************************************/
       /* Normal termination - inform MQ.                            */
       /* MQ will take the appropriate action including performing   */
       /* an MQDISC.                                                 */
       /**************************************************************/
       AMQ2PHASETASKDETACH();
     }
     else
     {
       /**************************************************************/
       /* Abnormal termination - inform MQ.                          */
       /* MQ will take the appropriate action including performing   */
       /* an MQDISC.                                                 */
       /**************************************************************/
       AMQ2PHASETASKABEND();
     }

     rc = UE_Normal;
    }
 }

 return rc;
}
/*********************************************************************/
/*                                                                   */
/* End of amqzscgn.c                                                 */
/*                                                                   */
/*********************************************************************/
/* MQMBID Start */
#ifndef __MQMBID_H
  #define __MQMBID_H
  #if defined(__MQMBID_H)
   /* Pointless code to prevent compiler warnings about __MQMBID_H being unused */
  #endif
  #if defined(_AIX)
   #if defined(__clang__)
    static char * __attribute__((used)) MQMBID(void) { return (char *)"@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/oltp/pc/winnt/amqzscgn.c"; }
   #else
    #pragma comment(user, "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/oltp/pc/winnt/amqzscgn.c")
   #endif
  #elif defined(_LINUX_2) || defined(linux) || defined(__linux) || defined(AMQ_MACOS) || defined(DARWIN) || defined(__APPLE__)
    static char * __attribute__((used)) MQMBID(void) { return (char *)"@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/oltp/pc/winnt/amqzscgn.c"; }
  #elif defined(WIN32) || defined(_WIN32)
    #pragma optimize("", off)
    #if defined(__cplusplus)
    static __inline char * MQMBIDFn(char * s) { return s; };
    const static char * MQMBID=MQMBIDFn((char *)"@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/oltp/pc/winnt/amqzscgn.c");
    #else
    const static char MQMBID[]="@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/oltp/pc/winnt/amqzscgn.c";
    #endif
    #pragma optimize("", on)
  #else
    static char * MQMBID(void) { return (char *)"@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/oltp/pc/winnt/amqzscgn.c"; }
  #endif
#endif
/* MQMBID End */
