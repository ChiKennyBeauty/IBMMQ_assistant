const static char sccsid[] = "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=cmd/servers/amqodqoa.c";
/*********************************************************************/
/*                                                                   */
/* Module Name: AMQODQOA.C                                           */
/*                                                                   */
/* Description: Common source for the IBM MQ Dead letter             */
/* handler.                                                          */
/*   <copyright                                                      */
/*   notice="lm-source-program"                                      */
/*   pids="5724-H72"                                                 */
/*   years="1994,2016"                                               */
/*   crc="" >                                                        */
/*   Licensed Materials - Property of IBM                            */
/*                                                                   */
/*   5724-H72                                                        */
/*                                                                   */
/*   (C) Copyright IBM Corp. 1994, 2016 All Rights Reserved.         */
/*                                                                   */
/*   US Government Users Restricted Rights - Use, duplication or     */
/*   disclosure restricted by GSA ADP Schedule Contract with         */
/*   IBM Corp.                                                       */
/*   </copyright>                                                    */
/*                                                                   */
/*********************************************************************/
/* Function:                                                         */
/* This file contains the functions that the dead letter queue       */
/* handler uses to track which messages have been processed and      */
/* how many attempts have been made against each message.            */
/*                                                                   */
/*********************************************************************/
/* Change History                                                    */
/*  pn Reason    Rls  Date     Orig.   Comments                      */
/*  -- --------  ---  ------   ----    ------------------            */
/* £A1=PHY50066  520  010522   CTTNG:  Initialise Retry Count struct */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <cmqc.h>
#include <amqodqha.h>

/*-------------------------------------------------------------------*/
/* The dead letter handler keeps a record of all the messages which  */
/* it has tried to process, but has failed.                          */
/*                                                                   */
/* The structure odqMsg_t describes the state data associated with   */
/* each such message.                                                */
/* It is possible for the dead letter queue handler to build up      */
/* a large number of odqMsg_t objects if some rule is causing        */
/* DLQ handler actions to fail, and the subsequent action is to      */
/* ignore the message. For this reason we use just the MsgId and     */
/* CorrelId to identify the message (rather than the entire MQMD)    */
/* limiting the size of odqMsg_t to around 100 bytes.                */
/*-------------------------------------------------------------------*/
typedef struct _odqMsg_t
{
struct _odqMsg_t *next;
MQBYTE24         MsgId;
MQBYTE24         CorrelId;
MQLONG           RetryCount;
} odqMsg_t ;

static odqMsg_t *odqMsgHead ;


#define FUNCTION_ID odqtodqInquireRetryCount

/*********************************************************************/
/*                                                                   */
/* Function: odqInquireRetryCount                                    */
/*                                                                   */
/* Description: Find out how many times this message has been retried*/
/*                                                                   */
/* Intended Function: This function determines if the current        */
/*                    instance of the DLQ handler has seen this      */
/*                    message before, and returns the number         */
/*                    of previous attempts at processing the msg.    */
/*                                                                   */
/* Input Parameters: MsgId                                           */
/*                   CorrelId                                        */
/*                                                                   */
/* Output Parameters: RetryCount                                     */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
odqResp odqInquireRetryCount(
                             /*IN*/
                               const MQBYTE24 MsgId,
                               const MQBYTE24 CorrelId,
                             /*OUT*/
                               MQLONG *RetryCount
                           )
{
 odqResp rc=odq_Ok ;
 odqMsg_t *pMsg;

/*-------------------------------------------------------------------*/
/* Return the retry count associated with the msg being              */
/* processed by the dead letter handler. If we have no record        */
/* of the message then the retry count is 0.                         */
/*                                                                   */
/*-------------------------------------------------------------------*/
 odq_fnc_entry( FUNCTION_ID ) ;
 *RetryCount = 0 ;
 for( pMsg = odqMsgHead ;
      pMsg != NULL ;
      pMsg = pMsg -> next )
 {
    if( memcmp( pMsg->MsgId, MsgId, MQ_MSG_ID_LENGTH) == 0 &&
        memcmp( pMsg->CorrelId, CorrelId, MQ_CORREL_ID_LENGTH) == 0 )
    {
      *RetryCount = pMsg->RetryCount ;
      pMsg->RetryCount ++ ;
      break ;
    }
 }
 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return rc ;
}
#undef FUNCTION_ID
#define FUNCTION_ID odqtodqAddMsg

/*********************************************************************/
/*                                                                   */
/* Function: odqAddMsg                                               */
/*                                                                   */
/* Description: Remember that we have seen a message                 */
/*                                                                   */
/* Intended Function: This function remembers that the current       */
/*                    instance of the DLQ handler has attempted      */
/*                    to process a particular message.               */
/*                                                                   */
/* Input Parameters: MsgId                                           */
/*                   CorrelId                                        */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
odqResp odqAddMsg(
                  /*IN*/
                    const MQBYTE24 MsgId,
                    const MQBYTE24 CorrelId
                  /*OUT*/
                 )
{
 odqResp rc=odq_Ok ;
 odqMsg_t *pMsg ;

/*-------------------------------------------------------------------*/
/* Remember that an attempt has been made to process a message.      */
/*                                                                   */
/*-------------------------------------------------------------------*/
 odq_fnc_entry( FUNCTION_ID ) ;
 rc = odqGetMem( sizeof(*pMsg),
                 "MsgHistory",
                 (void **)&pMsg);
 if( odq_Error > rc )
 {
  memcpy(pMsg->MsgId, MsgId, MQ_MSG_ID_LENGTH ) ;
  memcpy(pMsg->CorrelId, CorrelId, MQ_CORREL_ID_LENGTH ) ;
  pMsg->RetryCount = 1 ;
  pMsg->next = odqMsgHead ;
  odqMsgHead = pMsg ;
 }

 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return rc ;
}
#undef FUNCTION_ID
#define FUNCTION_ID odqtodqDeleteMsg

/*********************************************************************/
/*                                                                   */
/* Function: odqDeleteMsg                                            */
/*                                                                   */
/* Description: Forget a message which has been removed from the DLQ */
/*                                                                   */
/* Intended Function: This function deletes any history associated   */
/*                    with a message that has been removed from the  */
/*                    dead letter queue by this instance of the      */
/*                    DLQ handler.                                   */
/*                                                                   */
/* Input Parameters: MsgId                                           */
/*                   CorrelId                                        */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
odqResp odqDeleteMsg(
                     /*IN*/
                       const MQBYTE24 MsgId,
                       const MQBYTE24 CorrelId
                     /*OUT*/
                    )
{
 odqMsg_t *pMsg ;
 odqMsg_t *pMsg_prev=NULL ;
 odqResp rc=odq_Ok ;

/*-------------------------------------------------------------------*/
/* Remove our memory of some message which has now been dealt        */
/* with successfully by the DLQ handler.                             */
/*                                                                   */
/*-------------------------------------------------------------------*/
 odq_fnc_entry( FUNCTION_ID ) ;
 for( pMsg = odqMsgHead ;
      pMsg != NULL;
      pMsg = pMsg -> next )
 {
    if( memcmp( pMsg->MsgId, MsgId, MQ_MSG_ID_LENGTH) == 0 &&
        memcmp( pMsg->CorrelId, CorrelId, MQ_CORREL_ID_LENGTH) == 0 )
    {
      if(NULL == pMsg_prev)
      {
        odqMsgHead = pMsg->next ;
      }
      else
      {
        pMsg_prev->next = pMsg->next ;
      }
      odqFreeMem("MsgHistory", pMsg) ;
      break ;
    }
    pMsg_prev = pMsg ;
 }
 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return rc ;
}
#undef FUNCTION_ID

#define FUNCTION_ID odqtodqInitialiseRetryCount

/**************************************************************/
/* Function: odqInitialiseRetryCount                     @A1A */
/* Description: Initialise the Retry Count structure          */
/*                                                            */
/* Input parameters:  None                                    */
/* Output parameters: None                                    */
/* Returns:                                                   */
/**************************************************************/
void odqInitialiseRetryCount()
{
  odq_fnc_entry( FUNCTION_ID );

  odqMsgHead = NULL;

  odq_fnc_retcode( FUNCTION_ID, odq_Ok);
}
#undef FUNCTION_ID

