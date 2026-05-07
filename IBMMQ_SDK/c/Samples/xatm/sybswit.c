/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/xatm/sybswitn.c */
/******************************************************************************/
/*                                                                            */
/* Module name: sybswit.c                                                     */
/*                                                                            */
/* Description: MQ XA switch program for Sybase on Windows.                   */
/*              This file is used as an intermediary between MQ and           */
/*              Sybase for XA calls. It translates between the two            */
/*              function calling conventions (__stdcall and __cdecl).         */
/*                                                                            */
/*   <copyright                                                               */
/*   notice="lm-source-program"                                               */
/*   pids="5724-H72,"                                                         */
/*   years="1998,2016"                                                        */
/*   crc="3851441696" >                                                       */
/*   Licensed Materials - Property of IBM                                     */
/*                                                                            */
/*   5724-H72,                                                                */
/*                                                                            */
/*   (C) Copyright IBM Corp. 1998, 2016 All Rights Reserved.                  */
/*                                                                            */
/*   US Government Users Restricted Rights - Use, duplication or              */
/*   disclosure restricted by GSA ADP Schedule Contract with                  */
/*   IBM Corp.                                                                */
/*   </copyright>                                                             */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#include <cmqc.h>                                /* MQ header                 */
#include "xa.h"                                  /* MQ supplied XA header     */


/*********************************/
/* On NT __STDC__ is not defined */
/*********************************/
struct stdcall_xa_switch_t               /* as defined/used by Sybase on NT   */
{
  char name[RMNAMESZ];                   /* name of resource manager          */
  long flags;                            /* resource manager specific options */
  long version;                          /* must be 0                         */

  int (__stdcall * xa_open_entry)();     /* xa_open function pointer          */
  int (__stdcall * xa_close_entry)();    /* xa_close function pointer         */
  int (__stdcall * xa_start_entry)();    /* xa_start function pointer         */
  int (__stdcall * xa_end_entry)();      /* xa_end function pointer           */
  int (__stdcall * xa_rollback_entry)(); /* xa_rollback function pointer      */
  int (__stdcall * xa_prepare_entry)();  /* xa_prepare function pointer       */
  int (__stdcall * xa_commit_entry)();   /* xa_commit function pointer        */
  int (__stdcall * xa_recover_entry)();  /* xa_recover function pointer       */
  int (__stdcall * xa_forget_entry)();   /* xa_forget function pointer        */
  int (__stdcall * xa_complete_entry)(); /* xa_complete function pointer      */
};


/******************************************************************************/
/* External data declarations                                                 */
/******************************************************************************/
/* specify that the Sybase XA switch uses __stdcall conventions */
extern __declspec( dllimport ) struct stdcall_xa_switch_t sybase_TXS_xa_switch;


/* Function Prototypes (called by queue manager) */
int __cdecl intermediate_xa_open_entry(char * a, int b, long c);
int __cdecl intermediate_xa_close_entry(char * a, int b, long c);
int __cdecl intermediate_xa_start_entry(XID *, int, long);
int __cdecl intermediate_xa_end_entry(XID *, int, long);
int __cdecl intermediate_xa_rollback_entry(XID *, int, long);
int __cdecl intermediate_xa_prepare_entry(XID *, int, long);
int __cdecl intermediate_xa_commit_entry(XID *, int, long);
int __cdecl intermediate_xa_recover_entry(XID *, long, int, long);
int __cdecl intermediate_xa_forget_entry(XID *, int, long);
int __cdecl intermediate_xa_complete_entry(int *, int *, int, long);


/* This intermediate switch is of type declared in xa.h - __cdecl funs */
struct xa_switch_t intermediate_xa_switch =
{
  "SYBASE_XA_SERVER",
  TMNOMIGRATE,
  0,
  intermediate_xa_open_entry,
  intermediate_xa_close_entry,
  intermediate_xa_start_entry,
  intermediate_xa_end_entry,
  intermediate_xa_rollback_entry,
  intermediate_xa_prepare_entry,
  intermediate_xa_commit_entry,
  intermediate_xa_recover_entry,
  intermediate_xa_forget_entry,
  intermediate_xa_complete_entry
};


/******************************************************************************/
/*                                                                            */
/* Function name:  MQStart                                                    */
/*                                                                            */
/* Description: The queue manager calls this function to access the XA switch */
/*              of Sybase                                                     */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Input Parameters:  None                                                    */
/*                                                                            */
/* Output Parameters: None                                                    */
/*                                                                            */
/* Returns:           Pointer to Sybase XA switch (now the intermediate swit) */
/*                                                                            */
/******************************************************************************/
__declspec(dllexport) struct xa_switch_t * MQENTRY MQStart(void)
{
   return( &intermediate_xa_switch );
}

int __cdecl intermediate_xa_open_entry(char * a, int b, long c)
{
   return( sybase_TXS_xa_switch.xa_open_entry( a, b, c ) );
}

int __cdecl intermediate_xa_close_entry(char * a, int b, long c)
{
   return( sybase_TXS_xa_switch.xa_close_entry( a, b, c ) );
}

int __cdecl intermediate_xa_start_entry(XID *a, int b, long c)
{
   return( sybase_TXS_xa_switch.xa_start_entry( a, b, c ) );
}

int __cdecl intermediate_xa_end_entry(XID *a, int b, long c)
{
   return( sybase_TXS_xa_switch.xa_end_entry( a, b, c ) );
}

int __cdecl intermediate_xa_rollback_entry(XID *a, int b, long c)
{
   return( sybase_TXS_xa_switch.xa_rollback_entry( a, b, c ) );
}

int __cdecl intermediate_xa_prepare_entry(XID *a, int b, long c)
{
   return( sybase_TXS_xa_switch.xa_prepare_entry( a, b, c ) );
}

int __cdecl intermediate_xa_commit_entry(XID *a, int b, long c)
{
   return( sybase_TXS_xa_switch.xa_commit_entry( a, b, c ) );
}

int __cdecl intermediate_xa_recover_entry(XID *a, long b, int c, long d)
{
   return( sybase_TXS_xa_switch.xa_recover_entry( a, b, c, d ) );
}

int __cdecl intermediate_xa_forget_entry(XID *a, int b, long c)
{
   return( sybase_TXS_xa_switch.xa_forget_entry( a, b, c ) );
}

int __cdecl intermediate_xa_complete_entry(int *a, int *b, int c, long d)
{
   return( sybase_TXS_xa_switch.xa_complete_entry( a, b, c,d ) );
}

/***************************************************************************/
/* End of sybswit.c                                                        */
/***************************************************************************/
