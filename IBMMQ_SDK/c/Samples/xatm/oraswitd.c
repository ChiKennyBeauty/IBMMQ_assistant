/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/xatm/oraswitnd.c */
/******************************************************************************/
/*                                                                            */
/* Module name: oraswitd.c                                                    */
/*                                                                            */
/* Description: MQ XA Dynamic Registration switch program for Oracle          */
/*                                                                            */
/*   <copyright                                                               */
/*   notice="lm-source-program"                                               */
/*   pids="5724-H72,"                                                         */
/*   years="2000,2016"                                                        */
/*   crc="3179809424" >                                                       */
/*   Licensed Materials - Property of IBM                                     */
/*                                                                            */
/*   5724-H72,                                                                */
/*                                                                            */
/*   (C) Copyright IBM Corp. 2000, 2016 All Rights Reserved.                  */
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

/******************************************************************************/
/* External data declarations                                                 */
/******************************************************************************/

extern __declspec(dllimport) struct xa_switch_t xaoswd;

/******************************************************************************/
/*                                                                            */
/* Function name:  MQStart                                                    */
/*                                                                            */
/* Description: The queue manager calls this function to access the XA        */
/*              Dynamic Registration switch of Oracle                         */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Input Parameters:  None                                                    */
/*                                                                            */
/* Output Parameters: None                                                    */
/*                                                                            */
/* Returns:           Pointer to Oracle XA Dynamic Registration switch        */
/*                                                                            */
/******************************************************************************/
__declspec(dllexport) struct xa_switch_t * MQENTRY MQStart(void)
{
   return(&xaoswd);
}

/******************************************************************************/
/* End of oraswitd.c                                                          */
/******************************************************************************/
