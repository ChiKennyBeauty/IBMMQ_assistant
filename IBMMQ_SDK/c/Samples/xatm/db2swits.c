static const char sccsid[] = "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/xatm/db2switns.c";
/******************************************************************************/
/*                                                                            */
/* Module name: db2swits.c                                                    */
/*                                                                            */
/* Description: MQ XA Static Registration switch program for DB2              */
/*                                                                            */
/* <copyright notice="copyright-lm-source-program"                            */
/*            pids="5724-H72"                                                 */
/*            years="2011,2016"                                               */
/*            crc="1484397430" >                                              */
/*                                                                            */
/* Licensed Materials - Property of IBM                                       */
/*                                                                            */
/* 5724-H72                                                                   */
/*                                                                            */
/* (C) Copyright IBM Corp. 2011, 2016                                         */
/*                                                                            */
/* US Government Users Restricted Rights - Use, duplication or                */
/* disclosure restricted by GSA ADP Schedule Contract with                    */
/* IBM Corp.                                                                  */
/*                                                                            */
/* </copyright>                                                               */
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

extern __declspec(dllimport) struct xa_switch_t db2xa_switch_static;

/******************************************************************************/
/*                                                                            */
/* Function name:  MQStart                                                    */
/*                                                                            */
/* Description: The queue manager calls this function to access the XA switch */
/*              of DB2                                                        */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Input Parameters:  None                                                    */
/*                                                                            */
/* Output Parameters: None                                                    */
/*                                                                            */
/* Returns:           Pointer to DB2 XA switch                                */
/*                                                                            */
/******************************************************************************/
__declspec(dllexport) struct xa_switch_t * MQENTRY MQStart(void)
{
   return(&db2xa_switch_static);
}

/******************************************************************************/
/* End of db2swits.c                                                          */
/******************************************************************************/
