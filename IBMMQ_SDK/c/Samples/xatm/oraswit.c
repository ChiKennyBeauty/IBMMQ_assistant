/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/xatm/oraswitn.c */
/******************************************************************************/
/*                                                                            */
/* Module name: oraswit.c                                                     */
/*                                                                            */
/* Description: MQ XA switch program for Oracle                               */
/*                                                                            */
/*  <copyright                                                                */
/*  notice="lm-source-program"                                                */
/*  pids="5724-H72,"                                                          */
/*  years="2000,2016"                                                         */
/*  crc="2391191458" >                                                        */
/*  Licensed Materials - Property of IBM                                      */
/*                                                                            */
/*  5724-H72,                                                                 */
/*                                                                            */
/*  (C) Copyright IBM Corp. 2000, 2016 All Rights Reserved.                   */
/*                                                                            */
/*  US Government Users Restricted Rights - Use, duplication or               */
/*  disclosure restricted by GSA ADP Schedule Contract with                   */
/*  IBM Corp.                                                                 */
/*  </copyright>                                                              */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#include <cmqc.h>                                /* MQ header                 */
#include "xa.h"                                  /* MQ supplied XA header     */

/******************************************************************************/
/* External data declarations                                                 */
/******************************************************************************/

extern __declspec(dllimport) struct xa_switch_t xaosw;

/******************************************************************************/
/*                                                                            */
/* Function name:  MQStart                                                    */
/*                                                                            */
/* Description: The queue manager calls this function to access the XA switch */
/*              of Oracle                                                     */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Input Parameters:  None                                                    */
/*                                                                            */
/* Output Parameters: None                                                    */
/*                                                                            */
/* Returns:           Pointer to Oracle XA switch                             */
/*                                                                            */
/******************************************************************************/
__declspec(dllexport) struct xa_switch_t * MQENTRY MQStart(void)
{
   return(&xaosw);
}

/******************************************************************************/
/* End of oraswit.c                                                           */
/******************************************************************************/
