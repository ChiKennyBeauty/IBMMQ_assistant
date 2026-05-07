/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/xatm/infswitn.c */
/******************************************************************************/
/*                                                                            */
/* Module name: infswit.c                                                     */
/*                                                                            */
/* Description: MQ XA switch program for Informix                             */
/*                                                                            */
/*   <copyright                                                               */
/*   notice="lm-source-program"                                               */
/*   pids="5724-H72,"                                                         */
/*   years="2004,2016"                                                        */
/*   crc="343010680" >                                                        */
/*   Licensed Materials - Property of IBM                                     */
/*                                                                            */
/*   5724-H72,                                                                */
/*                                                                            */
/*   (C) Copyright IBM Corp. 2004, 2016 All Rights Reserved.                  */
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
/* External function declarations                                             */
/******************************************************************************/

extern __declspec(dllimport) struct xa_switch_t * MQENTRY fn_xa_switch(void);

/******************************************************************************/
/*                                                                            */
/* Function name:  MQStart                                                    */
/*                                                                            */
/* Description: The queue manager calls this function to access the XA switch */
/*              of Informix                                                   */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Input Parameters:  None                                                    */
/*                                                                            */
/* Output Parameters: None                                                    */
/*                                                                            */
/* Returns:           Pointer to Informix XA switch                           */
/*                                                                            */
/******************************************************************************/
__declspec(dllexport) struct xa_switch_t * MQENTRY MQStart(void)
{
   struct xa_switch_t *pXASwitch;                /* ptr to XA switch          */

   pXASwitch = fn_xa_switch();                   /* call function to get ptr  */

   return(pXASwitch);
}

/******************************************************************************/
/* End of infswit.c                                                           */
/******************************************************************************/
