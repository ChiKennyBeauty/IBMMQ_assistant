const static char sccsid[] = "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=cmd/servers/amqodqla.c";
/*********************************************************************/
/*                                                                   */
/* Module Name: AMQODQLA.C                                           */
/*                                                                   */
/* Description: Common source for the IBM MQ Dead letter   */
/*              handler.                                             */
/*   <copyright                                                      */
/*   notice="lm-source-program"                                      */
/*   pids="5724-H72,"                                                */
/*   years="1994,2023"                                               */
/*   crc="2422279613" >                                              */
/*   Licensed Materials - Property of IBM                            */
/*                                                                   */
/*   5724-H72,                                                       */
/*                                                                   */
/*   (C) Copyright IBM Corp. 1994, 2023 All Rights Reserved.         */
/*                                                                   */
/*   US Government Users Restricted Rights - Use, duplication or     */
/*   disclosure restricted by GSA ADP Schedule Contract with         */
/*   IBM Corp.                                                       */
/*   </copyright>                                                    */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/* Function:                                                         */
/*                                                                   */
/* This file contains the functions used by the DLQ handler to       */
/* browse the DLQ and to take the required action for each           */
/* message on the DLQ.                                               */
/*                                                                   */
/* odqProcessQueue is called by amqodqka.c once the input has        */
/* been successfully parsed and a successful connection made to      */
/* the queue manager.                                                */
/* odqProcessQueue performs a browse of the queue calling            */
/* the routines in amqodqna.c to determine the action for each       */
/* message retrieved from the DLQ, and then attempting to perform    */
/* the requested action.                                             */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <cmqc.h>
#include <amqodqha.h>

/*--------------------------------------------------------------------*/
/* We keep track of the time we last found a message that had not     */
/* already been identified as 'not processable' so that we can        */
/* time out the DLQ handler if WAIT seconds elapse and no new         */
/* messages have been identified.                                     */
/*--------------------------------------------------------------------*/
static time_t LastMessageProcessed;
/*--------------------------------------------------------------------*/
/* We keep track of the earliest message requiring retry so that      */
/* after RETRYINT seconds have elapsed we can reposition the          */
/* browse cursor to this message.                                     */
/*--------------------------------------------------------------------*/
static MQBYTE24 RetryMsgId ;
static MQBYTE24 RetryCorrelId ;


/*--------------------------------------------------------------------*/
/*                                                                    */
/*   I N T E R N A L    F U N C T I O N     P R O T O T Y P E S       */
/*                                                                    */
/*--------------------------------------------------------------------*/

static odqResp odqStartBrowse(
                              /*IN*/
                                const char    *dlqname,
                              /*OUT*/
                                MQHOBJ *Hobj,
                                MQHMSG *Hmsg
                             );
static odqResp odqEndBrowse(
                            /*IN*/
                              MQHOBJ *Hobj,
                              MQHMSG *Hmsg
                            /*OUT*/
                           );

static odqResp odqGetNext(
                          /*IN*/
                            const MQHOBJ  *Hobj,
                            const MQHMSG  *Hmsg,
                          /*IN/OUT*/
                                  MQMD    *MsgDesc,
                                  MQLONG  *BufferLength,
                                  MQBYTE **Buffer,
                          /*OUT*/
                                  MQDLH   *pConvDlh,
                                  MQLONG  *pMsgLength
                         );
static odqResp odqProcessMessage(
                                 /*IN*/
                                   const MQHOBJ  *Hobj,
                                   const MQHMSG  *Hmsg,
                                   const MQDLH   *dlh,
                                   const MQLONG  MessageLength,
                                   const MQBYTE  *Message,
                                         MQMD    *MsgDesc
                                 /*OUT*/
                                );
typedef enum
{
COMMIT,
BACKOUT
} syncpoint_action_t;

static odqResp odqSyncpoint(
                            /*IN*/
                              const syncpoint_action_t action
                            /*OUT*/
                           );



/*--------------------------------------------------------------------*/
/*                                                                    */
/*           E X E C U T A B L E     C O D E                          */
/*                                                                    */
/*--------------------------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/* Function: odqtodqProcessQueue                                     */
/*                                                                   */
/* Description: Process the nominated DLQ.                           */
/*                                                                   */
/* Intended Function: This function controls the processing of the   */
/*                    queue nominated as the dead letter queue.      */
/*                    The queue will be browsed until the queue has  */
/*                    been empty for the period specified by the     */
/*                    WAIT parameter in the rules table.             */
/*                                                                   */
/* Input Parameters: name of the queue to process                    */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
#define FUNCTION_ID odqtodqProcessQueue
/*-------------------------------------------------------------------*/
/* odqProcessQueue:                                                  */
/* This routine controls the processing of the nominated queue.      */
/*                                                                   */
/*   determine if we're processing the default DLQ                   */
/*   Start the browse                                                */
/*   While no severe errors                                          */
/*       Start a UOW (implicit)                                      */
/*       Get the next DLQ message                                    */
/*       Process the DLQ message                                     */
/*       End the UOW                                                 */
/*   End while                                                       */
/*   End the browse                                                  */
/*                                                                   */
/*-------------------------------------------------------------------*/
odqResp odqProcessQueue( /*IN*/
                           const char    *dlqname
                         /*OUT*/
                       )
{
 odqResp rc ;
 MQHOBJ Hobj ;
 MQMD   MsgDesc = {MQMD_DEFAULT};
 MQLONG BufferLength=0 ;
 MQLONG MsgLength ;
 MQBYTE *pBuffer=NULL ;
 MQHMSG Hmsg = MQHM_UNUSABLE_HMSG;
 MQDLH  ConvDlh = {MQDLH_DEFAULT};

 odq_fnc_entry( FUNCTION_ID ) ;

 time( &LastMessageProcessed );

 rc = odqStartBrowse(/*IN*/
                       dlqname,
                     /*OUT*/
                       &Hobj,
                       &Hmsg
                    );

 if ( odq_Error >= rc )
 {
   while( odq_Error >= rc )
   {
      /* Prevent MDE being inserted in front of DLH. */
      /* Use a version 2 MQMD in case the message is */
      /* Segmented/Grouped                   IC29805 */
      MsgDesc.Version = MQMD_VERSION_2;

      rc = odqGetNext(/*IN*/
                       &Hobj,
                       &Hmsg,
                       &MsgDesc,
                       &BufferLength,
                       &pBuffer,
                       /*OUT*/
                       &ConvDlh,
                       &MsgLength
                       );
      /*--------------------------------------------------------------*/
      /* If the queue remains empty for the interval specified via    */
      /* the WAIT parameter then the browse will return               */
      /* odq_TerminateTimeout and we terminate the DLQ handler.       */
      /*--------------------------------------------------------------*/
      if ( odq_TerminateTimeout == rc )
      {
        /*------------------------------------------------------------*/
        /* Reset rc to prevent odqProcessQueue from returning an      */
        /* error after the wait interval has expired which in turn    */
        /* prevents a CL error message being issued.                  */
        /*------------------------------------------------------------*/
        rc=odq_Ok;
        break ;
      }
      if (odq_Error >= rc )
      {
        rc = odqProcessMessage(/*IN*/
                                 &Hobj,
                                 &Hmsg,
                                 &ConvDlh,
                                 MsgLength,
                                 pBuffer,
                                 &MsgDesc
                                /*OUT*/
                               );
        if (odq_Ok == rc )
        {
          rc = odqSyncpoint(/*IN*/
                             COMMIT
                             /*OUT*/
                            );
          /*----------------------------------------------------------*/
          /* If we've successfully syncpointed then we've completed   */
          /* the action associated with this message and we can       */
          /* delete our state (retry count) associated with this      */
          /* message.                                                 */
          /*----------------------------------------------------------*/
          if (odq_Ok == rc )
          {
            rc = odqDeleteMsg( /*IN*/
                              MsgDesc.MsgId,
                              MsgDesc.CorrelId
                          );
          }
          else
          /*----------------------------------------------------------*/
          /* If we failed to commit the change then this is a         */
          /* potential retry point so check to see if we've already   */
          /* set a retry point.                                       */
          /*----------------------------------------------------------*/
          {
            if (!memcmp(RetryMsgId,
                        MQMI_NONE,
                        sizeof(RetryMsgId)) &&
                !memcmp(RetryCorrelId,
                        MQCI_NONE,
                        sizeof(RetryCorrelId)))
            {
              memcpy(RetryMsgId,
                     MsgDesc.MsgId,
                     sizeof(RetryMsgId));
              memcpy(RetryCorrelId,
                     MsgDesc.CorrelId,
                     sizeof(RetryCorrelId));
            }
          }
        }
        else
        {
          /*----------------------------------------------------------*/
          /* If we've suffered from a 'soft' error then we've not been*/
          /* able to deal with the message, and we need to put the    */
          /* message back onto the DLQ by backing out, but no real    */
          /* error has occurred.                                      */
          /*----------------------------------------------------------*/
          int rc1 ;
          rc1 = odqSyncpoint(/*IN*/
                               BACKOUT
                              /*OUT*/
                             );
          if (odq_Warning >= rc )
          {
            rc = rc1 ;
          }
          /*----------------------------------------------------------*/
          /* Because the message still exists on the DLQ we can't     */
          /* delete our state associated with the message.            */
          /*----------------------------------------------------------*/
        }
      }
   }
   odqEndBrowse(/*IN*/
                  &Hobj,
                  &Hmsg
                /*OUT*/
               );
 }
 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return rc ;

}


#undef FUNCTION_ID
#define FUNCTION_ID odqtodqSyncpoint

/*********************************************************************/
/*                                                                   */
/* Function: odqSyncpoint                                            */
/*                                                                   */
/* Description: End the current UOW                                  */
/*                                                                   */
/* Intended Function: This function terminates the current unit of   */
/*                    work, either committing the UOW to harden      */
/*                    removing the message from the DLQ, or backing  */
/*                    out to replace the message on the DLQ.         */
/*                                                                   */
/* Input Parameters: COMMIT/BACKOUT indicator                        */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
static odqResp odqSyncpoint(
                            /*IN*/
                              const syncpoint_action_t action
                            /*OUT*/
                           )
{
 odqResp rc ;
 MQLONG CompCode;
 MQLONG Reason ;
 odq_fnc_entry( FUNCTION_ID ) ;
 if (COMMIT == action )
 {
   odqMQCMIT( odqGlobal.odqHConn,
              &CompCode,
              &Reason ) ;
 }
 else
 {
   odqMQBACK(  odqGlobal.odqHConn,
               &CompCode,
               &Reason ) ;
 }
 switch( CompCode )
 {
   case MQCC_OK:
        rc = odq_Ok ;
        break ;
   default:
        switch( Reason )
        {
     /*---------------------------------------------------------------*/
     /* The following errors can occur without implying any problem   */
     /* in the DLQ handler. Upon receipt of one of these errors then  */
     /* we will issue a message and terminate the DLQ handler.        */
     /*---------------------------------------------------------------*/
          case MQRC_CONNECTION_BROKEN:
               rc =  odq_ConnectionBroken ;
               break ;
          case MQRC_Q_MGR_STOPPING:
          case MQRC_RESOURCE_PROBLEM:
          case MQRC_STORAGE_NOT_AVAILABLE:
               odqFFDC( /*IN*/
                          FUNCTION_ID,
                          odq_SyncpointErr,
                          CompCode,
                          Reason,
                          NULL
                         /*OUT*/
                      );
               rc =  odq_SyncpointErr ;
               break ;
     /*---------------------------------------------------------------*/
     /* Any other error is unexpected and implies an error in this    */
     /* code, or an error in the queue manager.                       */
     /*---------------------------------------------------------------*/
          default:
               odqFFDC( /*IN*/
                          FUNCTION_ID,
                          odq_UnexpectedSyncpointErr,
                          CompCode,
                          Reason,
                          NULL
                        /*OUT*/
                      );
            rc =  odq_UnexpectedSyncpointErr ;
        }
 }
 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return rc ;
}


#undef FUNCTION_ID

#define FUNCTION_ID odqtodqStartBrowse

/*********************************************************************/
/*                                                                   */
/* Function: odqStartBrowse                                          */
/*                                                                   */
/* Description: Open the nominated queue for browsing                */
/*                                                                   */
/* Intended Function: This function opens the queue nominated as     */
/*                    the dead letter queue for browsing.            */
/*                    If the open should fail then produce the       */
/*                    necessary diagnostic materials.                */
/*                                                                   */
/* Input Parameters: queue name                                      */
/*                                                                   */
/* Output Parameters: queue handle                                   */
/*                    message handle                                 */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
static odqResp odqStartBrowse(
                              /*IN*/
                                const char    *dlqname,
                              /*OUT*/
                                MQHOBJ *Hobj,
                                MQHMSG *Hmsg
                             )
{
 MQOD Obj = {MQOD_DEFAULT};
 MQCMHO CrtMsgHOpts = {MQCMHO_DEFAULT};
 /*-------------------------------------------------------------------*/
 /* We're expecting to be long running so we don't want to proceed if */
 /* the queue manager is quiescing.                                   */
 /* We use MQGMO_LOCK and MQGMO_SYNCPOINT to perform any necessary    */
 /* locking so we can afford to open the queue for shared input. If   */
 /* any other processes read the queue then they should also use these*/
 /* options, or else open the queue exclusively.                      */
 /*-------------------------------------------------------------------*/
 const MQLONG Options=MQOO_BROWSE +
                      MQOO_FAIL_IF_QUIESCING +
                      MQOO_SAVE_ALL_CONTEXT +
                      MQOO_INPUT_AS_Q_DEF ;
 MQLONG CompCode;
 MQLONG Reason ;
 odqResp rc ;

 odq_fnc_entry( FUNCTION_ID ) ;

 /*-------------------------------------------------------------------*/
 /* Create a message handle so that the properties of the message can */
 /* be retrieved when MQGET is called.                                */
 /*-------------------------------------------------------------------*/
 odqMQCRTMH( odqGlobal.odqHConn,
             &CrtMsgHOpts,
             Hmsg,
             &CompCode,
             &Reason ) ;

 switch( CompCode )
 {
   case MQCC_OK:
        rc = odq_Ok ;
        break;
   default:
        switch( Reason )
        {
     /*-------------------------------------------------------------*/
     /* If the queue manager is shutting down then we also end.     */
     /*-------------------------------------------------------------*/
          case MQRC_CONNECTION_BROKEN:
               rc =  odq_ConnectionBroken ;
               break ;
     /*-------------------------------------------------------------*/
     /* The following errors can occur without implying any problem */
     /* in the DLQ handler. Upon receipt of one of these errors     */
     /* then we will issue a message and terminate the DLQ handler. */
     /*-------------------------------------------------------------*/
          case MQRC_HANDLE_NOT_AVAILABLE:
          case MQRC_Q_MGR_STOPPING:
          case MQRC_RESOURCE_PROBLEM:
          case MQRC_STORAGE_NOT_AVAILABLE:
               odqFFDC( /*IN*/
                          FUNCTION_ID,
                          odq_StartBrowseErr,
                          CompCode,
                          Reason,
                          dlqname,
                          MQ_Q_NAME_LENGTH,
                          "DLQ Name",
                          NULL
                        /*OUT*/
                      );
               rc =  odq_StartBrowseErr ;
               break ;
     /*-------------------------------------------------------------*/
     /* Any other error is unexpected and implies an error in this  */
     /* code, or an error in the queue manager.                     */
     /*-------------------------------------------------------------*/
          default:
            odqFFDC( /*IN*/
                       FUNCTION_ID,
                       odq_UnexpectedStartBrowseErr,
                       CompCode,
                       Reason,
                       dlqname,
                       MQ_Q_NAME_LENGTH,
                       "DLQ Name",
                       NULL
                     /*OUT*/
                   );
            rc =  odq_UnexpectedStartBrowseErr ;
        }
 }

 if(rc == odq_Ok)
 {
   strncpy(Obj.ObjectName, dlqname, MQ_Q_MGR_NAME_LENGTH ) ;
   odqMQOPEN( odqGlobal.odqHConn,
              &Obj,
              Options,
              Hobj,
              &CompCode,
              &Reason ) ;

   switch( CompCode )
   {
     case MQCC_OK:
          rc = odq_Ok ;
          break;
     default:
          switch( Reason )
          {
       /*-------------------------------------------------------------*/
       /* If the queue manager is shutting down then we also end.     */
       /*-------------------------------------------------------------*/
            case MQRC_CONNECTION_BROKEN:
                 rc =  odq_ConnectionBroken ;
                 break ;
       /*-------------------------------------------------------------*/
       /* The following errors can occur without implying any problem */
       /* in the DLQ handler. Upon receipt of one of these errors     */
       /* then we will issue a message and terminate the DLQ handler. */
       /*-------------------------------------------------------------*/
            case MQRC_ALIAS_BASE_Q_TYPE_ERROR:
            case MQRC_HANDLE_NOT_AVAILABLE:
            case MQRC_NOT_AUTHORIZED:
            case MQRC_OBJECT_DAMAGED:
            case MQRC_Q_MGR_QUIESCING:
            case MQRC_Q_MGR_STOPPING:
            case MQRC_RESOURCE_PROBLEM:
            case MQRC_SECURITY_ERROR:
            case MQRC_TOKEN_TIMESTAMP_NOT_VALID:
            case MQRC_STORAGE_NOT_AVAILABLE:
            case MQRC_UNKNOWN_ALIAS_BASE_Q:
            case MQRC_UNKNOWN_OBJECT_NAME:
            case MQRC_OBJECT_IN_USE:
                 odqFFDC( /*IN*/
                            FUNCTION_ID,
                            odq_StartBrowseErr,
                            CompCode,
                            Reason,
                            dlqname,
                            MQ_Q_NAME_LENGTH,
                            "DLQ Name",
                            NULL
                          /*OUT*/
                        );
                 rc =  odq_StartBrowseErr ;
                 break ;
       /*-------------------------------------------------------------*/
       /* Any other error is unexpected and implies an error in this  */
       /* code, or an error in the queue manager.                     */
       /*-------------------------------------------------------------*/
            default:
              odqFFDC( /*IN*/
                         FUNCTION_ID,
                         odq_UnexpectedStartBrowseErr,
                         CompCode,
                         Reason,
                         dlqname,
                         MQ_Q_NAME_LENGTH,
                         "DLQ Name",
                         NULL
                       /*OUT*/
                     );
              rc =  odq_UnexpectedStartBrowseErr ;
          }
   }
 }
 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return ( rc ) ;

}




#undef FUNCTION_ID

#define FUNCTION_ID odqtodqEndBrowse

/*********************************************************************/
/*                                                                   */
/* Function: odqEndBrowse                                            */
/*                                                                   */
/* Description: End the browse of the dead letter queue              */
/*                                                                   */
/* Intended Function: Close the queue that was opened as the dead    */
/*                    letter queue. If the close should fail then    */
/*                    produce the necessary diagnostic materials.    */
/*                                                                   */
/* Input Parameters: queue handle                                    */
/*                   message handle                                  */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
static odqResp odqEndBrowse(
                            /*IN*/
                              MQHOBJ *Hobj,
                              MQHMSG *Hmsg
                            /*OUT*/
                           )
{
 const MQLONG Options=0 ;
       MQLONG CompCode ;
       MQLONG Reason ;
 MQDMHO DltMsgHOpts = {MQDMHO_DEFAULT};
 odqResp rc ;

 odq_fnc_entry( FUNCTION_ID ) ;

 odqMQCLOSE(odqGlobal.odqHConn,
            Hobj,
            Options,
            &CompCode,
            &Reason );

 switch( CompCode )
 {
   case MQCC_OK:
        rc = odq_Ok ;
        break;
   default:
     switch( Reason )
     {
     /*---------------------------------------------------------------*/
     /* If the queue manager is shutting down then we also end.       */
     /*---------------------------------------------------------------*/
     case MQRC_CONNECTION_BROKEN:
               rc =  odq_ConnectionBroken ;
               break ;
     /*---------------------------------------------------------------*/
     /* The following errors can occur without implying any problem   */
     /* in the DLQ handler. Upon receipt of one of these errors then  */
     /* we will issue a message and terminate the DLQ handler.        */
     /*---------------------------------------------------------------*/
     case MQRC_Q_MGR_STOPPING:
     case MQRC_RESOURCE_PROBLEM:
     case MQRC_STORAGE_NOT_AVAILABLE:
          odqFFDC( /*IN*/
                     FUNCTION_ID,
                     odq_EndBrowseErr,
                     CompCode,
                     Reason,
                     NULL
                   /*OUT*/
                 );
          rc =  odq_EndBrowseErr ;
          break ;
     /*---------------------------------------------------------------*/
     /* Any other error is unexpected and implies an error in this    */
     /* code, or an error in the queue manager.                       */
     /*---------------------------------------------------------------*/
     default:
          odqFFDC( /*IN*/
                     FUNCTION_ID,
                     odq_UnexpectedEndBrowseErr,
                     CompCode,
                     Reason,
                     NULL
                   /*OUT*/
                 );
          rc =  odq_UnexpectedEndBrowseErr ;
     }
 }

 /*-------------------------------------------------------------------*/
 /* Delete the message handle used to retrieve the message properties */
 /*-------------------------------------------------------------------*/
 if(rc == odq_Ok)
 {
   odqMQDLTMH(odqGlobal.odqHConn,
              Hmsg,
              &DltMsgHOpts,
              &CompCode,
              &Reason );

   switch( CompCode )
   {
     case MQCC_OK:
          rc = odq_Ok ;
          break;
     default:
       switch( Reason )
       {
       /*-------------------------------------------------------------*/
       /* If the queue manager is shutting down then we also end.     */
       /*-------------------------------------------------------------*/
       case MQRC_CONNECTION_BROKEN:
                 rc =  odq_ConnectionBroken ;
                 break ;
       /*-------------------------------------------------------------*/
       /* The following errors can occur without implying any problem */
       /* in the DLQ handler. Upon receipt of one of these errors     */
       /* then we will issue a message and terminate the DLQ handler. */
       /*-------------------------------------------------------------*/
       case MQRC_Q_MGR_STOPPING:
       case MQRC_RESOURCE_PROBLEM:
       case MQRC_STORAGE_NOT_AVAILABLE:
            odqFFDC( /*IN*/
                       FUNCTION_ID,
                       odq_EndBrowseErr,
                       CompCode,
                       Reason,
                       NULL
                     /*OUT*/
                   );
            rc =  odq_EndBrowseErr ;
            break ;
       /*-------------------------------------------------------------*/
       /* Any other error is unexpected and implies an error in this  */
       /* code, or an error in the queue manager.                     */
       /*-------------------------------------------------------------*/
       default:
            odqFFDC( /*IN*/
                       FUNCTION_ID,
                       odq_UnexpectedEndBrowseErr,
                       CompCode,
                       Reason,
                       NULL
                     /*OUT*/
                   );
            rc =  odq_UnexpectedEndBrowseErr ;
       }
   }
 }
 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return rc;
}




#undef FUNCTION_ID

#define FUNCTION_ID odqtodqGetNext

/*********************************************************************/
/*                                                                   */
/* Function: odqGetNext                                              */
/*                                                                   */
/* Description: Get the next dlq message to be processed             */
/*                                                                   */
/* Intended Function: This function does an MQGET within syncpoint   */
/*                    to get the next message to be processed by the */
/*                    DLQ handler.                                   */
/*                                                                   */
/* Input Parameters: queue handle                                    */
/*                   message handle                                  */
/*                                                                   */
/* Output Parameters: message descriptor                             */
/*                    message buffer length                          */
/*                    message buffer address                         */
/*                    converted DLH structure                        */
/*                    message length                                 */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
static odqResp odqGetNext(
                          /*IN*/
                            const MQHOBJ   *Hobj,
                            const MQHMSG   *Hmsg,
                          /*IN/OUT*/
                            MQMD     *MsgDesc,
                            MQLONG   *BufferLength,
                            MQBYTE  **Buffer,
                          /*OUT*/
                            MQDLH    *pConvDlh,
                            MQLONG   *pMsgLength
                         )
{
 MQGMO         GetMsgOpts={MQGMO_DEFAULT};
 time_t        now ;
 /*----------------------------------------------------------------*/
 /* LastRetryTime_x indicates if LastRetryTime is set.             */
 /*----------------------------------------------------------------*/
 static int    LastRetryTime_x ;
 static time_t LastRetryTime;
 /*----------------------------------------------------------------*/
 /* LastRescanTime_x indicates if LastRescanTime is set.           */
 /*----------------------------------------------------------------*/
 static int    LastRescanTime_x ;
 static time_t LastRescanTime;

 double        SecondsSinceRetry=999999999;
 double        SecondsSinceRescan=999999999;
 MQLONG        CompCode=MQCC_OK;
 MQLONG        Reason;
 int           RescanFrequency =
               odqGlobal.odqPatternHead->Parm[RetryInt].i * 10;
 odqResp       rc=odq_Ok ;

/*-------------------------------------------------------------------*/
/* This routine does not perform a straight FIFO browse of the queue */
/* but attempts to keep retrying messages which were browsed         */
/* previously, but were not committed for some reason.               */
/* A further complication is caused by browse not seeing messages    */
/* which have been added but not committed, and messages which are   */
/* added in an unexpected sequence (e.g. priority).                  */
/* Every RETRYINT seconds the browse is repositioned to the earliest */
/* known retryable message.                                          */
/* Every RescanFrequency seconds the browse is repositioned to the   */
/* beginning of the queue to check for messages added out of         */
/* sequence, or missed due to the browse not seeing uncommitted      */
/* messages.                                                         */
/* If the DLQ contains only messages which the rule table indicates  */
/* should be ignored, and no new messages arrive for WAIT seconds    */
/* then the DLQ handler ends.                                        */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
 odq_fnc_entry( FUNCTION_ID ) ;

 /*------------------------------------------------------------------*/
 /* Retrieve the properties of the message using a message handle.   */
 /*------------------------------------------------------------------*/
 GetMsgOpts.Version   = MQGMO_VERSION_4;
 GetMsgOpts.MsgHandle = *Hmsg;

 while(1)                                      /* forever while( 1 ) */
 {
   time( &now ) ;
   if( LastRetryTime_x )
   {
     SecondsSinceRetry = difftime(now, LastRetryTime) ;
   }
   if( LastRescanTime_x )
   {
     SecondsSinceRescan = difftime(now, LastRescanTime) ;
   }
   /*---------------------------------------------------------------*/
   /* If it's time we did a complete rescan to pick up any          */
   /* 'unexpected' (fifo or missed due to syncpoint) messages       */
   /* then reposition the cursor to the beginning of the queue.     */
   /*---------------------------------------------------------------*/
   if( SecondsSinceRescan >= RescanFrequency )
   {
     /***************************************************************/
     /* Because we're asking for the first message on the queue then*/
     /* if there is none available then there cannot be an earlier  */
     /* message that requires retry. Consequently we can use        */
     /* the WaitInterval to control whether the DLQ handler will    */
     /* terminate.                                                  */
     /* MQWI_UNLIMITED - The application will not terminate until   */
     /*                  some asynchronous event occurs requesting  */
     /*                  the application to end.                    */
     /*  0             - The application will end as soon as the    */
     /*                  dead letter queue is empty.                */
     /*                                                             */
     /*  >0            - The application will wait for up to the    */
     /*                  specified number of seconds for new input. */
     /*                  If none arrives within the time specified  */
     /*                  then the handler will terminate.           */
     /***************************************************************/
       GetMsgOpts.Options = MQGMO_BROWSE_FIRST+
                            MQGMO_WAIT+
                            MQGMO_LOCK+
                            MQGMO_CONVERT+
                            MQGMO_ACCEPT_TRUNCATED_MSG+
                            MQGMO_FAIL_IF_QUIESCING+
                            MQGMO_PROPERTIES_IN_HANDLE ;
       if(Specified == *(odqGlobal.odqPatternHead->ParmSpecified+Wait)&&
           odqGlobal.odqPatternHead->Parm[Wait].i >= 0 )
       {
         GetMsgOpts.WaitInterval = (MQLONG) max(0,
              (odqGlobal.odqPatternHead->Parm[Wait].i
                - difftime(now, LastMessageProcessed))*1000) ;
       }
       else
       {
         GetMsgOpts.WaitInterval = MQWI_UNLIMITED ;
       }
       /**************************************************************/
       /*  MsgId and CorrelId are selectors that must be cleared     */
       /*  to get messages in sequence, and they are set each MQGET  */
       /**************************************************************/
       memcpy(MsgDesc->MsgId, MQMI_NONE, sizeof(MsgDesc->MsgId));
       memcpy(MsgDesc->CorrelId, MQCI_NONE, sizeof(MsgDesc->CorrelId));

       /**************************************************************/
       /* If an earlier get should fail (for example truncated msg)  */
       /* then data conversion may not have been attempted and the   */
       /* Encoding and CodedCharSetId in the MQMD may be those of    */
       /* the message that failed the get, rather than those of the  */
       /* queue manager. Reset these fields to the Encoding and      */
       /* CCSID of the queue manager.                                */
       /**************************************************************/
       MsgDesc -> Encoding = MQENC_NATIVE ;
       MsgDesc -> CodedCharSetId = MQCCSI_Q_MGR ;

       odqMQGET( odqGlobal.odqHConn,
                 *Hobj,
                 MsgDesc,
                 &GetMsgOpts,
                 MQDLH_LENGTH_1,
                 pConvDlh,
                 pMsgLength,
                 &CompCode,
                 &Reason ) ;

       time( &LastRescanTime ) ;
       LastRescanTime_x ++ ;
       /**************************************************************/
       /*  A complete rescan will include retry so reset the last    */
       /*  retry time to indicate it was done.                       */
       /**************************************************************/
       time( &LastRetryTime ) ;
       LastRetryTime_x ++ ;
       /**************************************************************/
       /*  If we timed out then there are no messages on the queue,  */
       /*  and it's time to shut up shop, otherwise lets see if any  */
       /*  of the messages on the queue are processible.             */
       /**************************************************************/
       break ;
   }
   /*---------------------------------------------------------------*/
   /* It's not time for a complete rescan yet, let's see if we      */
   /* should reposition for a retry.                                */
   /*---------------------------------------------------------------*/
   if(SecondsSinceRetry > odqGlobal.odqPatternHead->Parm[RetryInt].i &&
      (memcmp(RetryMsgId, MQMI_NONE, sizeof(MsgDesc->MsgId)) ||
       memcmp(RetryCorrelId, MQCI_NONE, sizeof(MsgDesc->CorrelId))) )
   {
     GetMsgOpts.Options = MQGMO_BROWSE_FIRST+
                          MQGMO_LOCK+
                          MQGMO_CONVERT+
                          MQGMO_ACCEPT_TRUNCATED_MSG+
                          MQGMO_FAIL_IF_QUIESCING+
                          MQGMO_PROPERTIES_IN_HANDLE ;
     /**************************************************************/
     /*  MsgId and CorrelId must be set to point at the required   */
     /*  message.                                                  */
     /**************************************************************/
     memcpy(MsgDesc->MsgId,
            RetryMsgId,
            sizeof(MsgDesc->MsgId));
     memcpy(MsgDesc->CorrelId,
            RetryCorrelId,
            sizeof(MsgDesc->CorrelId));

     memcpy(RetryMsgId, MQMI_NONE, sizeof(MsgDesc->MsgId)) ;
     memcpy(RetryCorrelId, MQCI_NONE, sizeof(MsgDesc->CorrelId)) ;

     /**************************************************************/
     /* If an earlier get should fail (for example truncated msg)  */
     /* then data conversion may not have been attempted and the   */
     /* Encoding and CodedCharSetId in the MQMD may be those of    */
     /* the message that failed the get, rather than those of the  */
     /* queue manager. Reset these fields to the Encoding and      */
     /* CCSID of the queue manager.                                */
     /**************************************************************/
     MsgDesc -> Encoding = MQENC_NATIVE ;
     MsgDesc -> CodedCharSetId = MQCCSI_Q_MGR ;

     odqMQGET( odqGlobal.odqHConn,
               *Hobj,
               MsgDesc,
               &GetMsgOpts,
               MQDLH_LENGTH_1,
               pConvDlh,
               pMsgLength,
               &CompCode,
               &Reason ) ;

     /*---------------------------------------------------------------*/
     /* If the get fails then the message has been removed by some    */
     /* other process. We have to go back to the start of the queue   */
     /* and retry from the begining.                                  */
     /*---------------------------------------------------------------*/
     if ( MQCC_FAILED == CompCode &&
          MQRC_NO_MSG_AVAILABLE == Reason )
     {
       LastRescanTime_x = 0 ;
       SecondsSinceRescan = 999999999 ;
     }
     else
     {
       time( &LastRetryTime ) ;
       LastRetryTime_x ++ ;
       break ;
     }
   }
   else
   {
     GetMsgOpts.Options = MQGMO_WAIT+
                          MQGMO_BROWSE_NEXT+
                          MQGMO_LOCK+
                          MQGMO_CONVERT+
                          MQGMO_ACCEPT_TRUNCATED_MSG+
                          MQGMO_FAIL_IF_QUIESCING+
                          MQGMO_PROPERTIES_IN_HANDLE ;
     if( memcmp(RetryMsgId, MQMI_NONE, sizeof(MsgDesc->MsgId)) ||
        memcmp(RetryCorrelId, MQCI_NONE, sizeof(MsgDesc->CorrelId)) )
     {
        GetMsgOpts.WaitInterval = (MQLONG) max( 0,
                            (odqGlobal.odqPatternHead->Parm[RetryInt].i
                              - SecondsSinceRetry )*1000) ;
     }
     else
     {
       /***************************************************************/
       /* Wait either for the next rescan time, or for the WAIT time  */
       /* to expire (which ever comes first).                         */
       /***************************************************************/
       if(Specified == *(odqGlobal.odqPatternHead->ParmSpecified+Wait)&&
           odqGlobal.odqPatternHead->Parm[Wait].i >= 0  &&
           odqGlobal.odqPatternHead->Parm[Wait].i
                - difftime(now, LastMessageProcessed) <
            RescanFrequency  - SecondsSinceRescan )
        {
          GetMsgOpts.WaitInterval = (MQLONG) max( 0 ,
           (odqGlobal.odqPatternHead->Parm[Wait].i -
            difftime(now,LastMessageProcessed))*1000);
        }
        else
        {
          GetMsgOpts.WaitInterval = (MQLONG) max( 0,
              (RescanFrequency  - SecondsSinceRetry )*1000) ;
        }
     }
     /*****************************************************************/
     /*  MsgId and CorrelId are selectors that must be cleared        */
     /*  to get messages in sequence, and they are set each MQGET     */
     /*****************************************************************/
     memcpy(MsgDesc->MsgId, MQMI_NONE, sizeof(MsgDesc->MsgId));
     memcpy(MsgDesc->CorrelId, MQCI_NONE, sizeof(MsgDesc->CorrelId));

     /**************************************************************/
     /* If an earlier get should fail (for example truncated msg)  */
     /* then data conversion may not have been attempted and the   */
     /* Encoding and CodedCharSetId in the MQMD may be those of    */
     /* the message that failed the get, rather than those of the  */
     /* queue manager. Reset these fields to the Encoding and      */
     /* CCSID of the queue manager.                                */
     /**************************************************************/
     MsgDesc -> Encoding = MQENC_NATIVE ;
     MsgDesc -> CodedCharSetId = MQCCSI_Q_MGR ;

     odqMQGET( odqGlobal.odqHConn,
               *Hobj,
               MsgDesc,
               &GetMsgOpts,
               MQDLH_LENGTH_1,
               pConvDlh,
               pMsgLength,
               &CompCode,
               &Reason ) ;
     /*---------------------------------------------------------------*/
     /* If there is no new message available but there is a retryable */
     /* message then we must be past the retry time and we can just   */
     /* iterate the while loop.                                       */
     /* If there is no new message available and no retryable message */
     /* then we can't rely upon having reached the time of the next   */
     /* complete rescan as we may have a very small WAIT time.        */
     /* Make sure we have done a rescan before allowing the dlq       */
     /* dlq handler to end.                                           */
     /*---------------------------------------------------------------*/
     if ( MQCC_FAILED == CompCode &&
          MQRC_NO_MSG_AVAILABLE == Reason )
     {
       if( memcmp(RetryMsgId, MQMI_NONE, sizeof(MsgDesc->MsgId)) ||
          memcmp(RetryCorrelId, MQCI_NONE, sizeof(MsgDesc->CorrelId)) )
       {
         continue ;
       }
       if(Specified == *(odqGlobal.odqPatternHead->ParmSpecified+Wait)&&
          odqGlobal.odqPatternHead->Parm[Wait].i >= 0  &&
          difftime(LastRescanTime, LastMessageProcessed) > 0 &&
          odqGlobal.odqPatternHead->Parm[Wait].i
                < difftime(now, LastMessageProcessed) )
       {
         break ;
       }
       else
       /*-------------------------------------------------------------*/
       /* Force a complete rescan.                                    */
       /*-------------------------------------------------------------*/
       {
         LastRescanTime_x = 0 ;
         SecondsSinceRescan = 999999999 ;
       }
     }
     else
     {
       /*-------------------------------------------------------------*/
       /* If there are no retryable messages then we can restart the  */
       /* retry clock, otherwise if this message is retryable then it */
       /* will be retried too early.                                  */
       /*-------------------------------------------------------------*/
       if( !memcmp(RetryMsgId, MQMI_NONE, sizeof(MsgDesc->MsgId)) &&
          !memcmp(RetryCorrelId, MQCI_NONE, sizeof(MsgDesc->CorrelId)) )
       {
         time( &LastRetryTime ) ;
         LastRetryTime_x ++ ;
       }
       break ;
     }
   }
 }

 /*---------------------------------------------------------------*/
 /* If we didn't establish a browse cursor then we can't get      */
 /* the message under the browse cursor.                          */
 /*---------------------------------------------------------------*/
 if ( MQCC_FAILED == CompCode )
 {
   switch( Reason )
   {
     case MQRC_NO_MSG_AVAILABLE:
          rc = odq_TerminateTimeout ;
          break ;
     /*---------------------------------------------------------------*/
     /* If the queue manager is shutting down then we also end.       */
     /*---------------------------------------------------------------*/
     case MQRC_Q_MGR_QUIESCING:
     case MQRC_Q_MGR_STOPPING:
     case MQRC_CONNECTION_BROKEN:
          rc =  odq_ConnectionBroken ;
          break ;
     /*---------------------------------------------------------------*/
     /* The following errors can occur without implying any problem   */
     /* in the DLQ handler. Upon receipt of one of these errors then  */
     /* we will issue a message and terminate the DLQ handler.        */
     /*---------------------------------------------------------------*/
     case MQRC_GET_INHIBITED:
     case MQRC_OBJECT_CHANGED:
     case MQRC_Q_DELETED:
     case MQRC_RESOURCE_PROBLEM:
     case MQRC_STORAGE_NOT_AVAILABLE:
          odqFFDC( /*IN*/
                     FUNCTION_ID,
                     odq_GetNextErr,
                     CompCode,
                     Reason,
                     NULL
                   /*OUT*/
                 );
          rc =  odq_GetNextErr ;
          break ;
     /*---------------------------------------------------------------*/
     /* Any other error is unexpected and implies an error in this    */
     /* code, or an error in the queue manager.                       */
     /*---------------------------------------------------------------*/
      default:
         odqFFDC( /*IN*/
                    FUNCTION_ID,
                    odq_UnexpectedGetNextErr,
                    CompCode,
                    Reason,
                    NULL
                  /*OUT*/
               );
         rc =  odq_UnexpectedGetNextErr ;
   }
 }

 /*------------------------------------------------------------------*/
 /* if the buffer was not big enough for the current message then    */
 /* free the current buffer and allocate a new buffer big enough to  */
 /* contain the message.                                             */
 /*                                                                  */
 /*------------------------------------------------------------------*/
 if( odq_Ok == rc )
 {
   odqFreeMem( "MessageBuffer", *Buffer);
   rc = odqGetMem( *pMsgLength,
                   "MessageBuffer",
                   (void **)Buffer ) ;

   *BufferLength = *pMsgLength;
 }

 if( odq_Ok == rc )
 /*------------------------------------------------------------------*/
 /* We've got the browse cursor positioned over a message which      */
 /* we've got locked, and we've got a big enough buffer into which   */
 /* we're going to receive the message so now we go ahead and        */
 /* perform a destructive GET (within syncpoint).                    */
 /*                                                                  */
 /*------------------------------------------------------------------*/
 {
   GetMsgOpts.Options = MQGMO_MSG_UNDER_CURSOR +
                        MQGMO_SYNCPOINT +
                        MQGMO_PROPERTIES_IN_HANDLE;

   /**************************************************************/
   /* The message which caused the truncated msg response code   */
   /* will not have been subject to data conversion and so the   */
   /* fields in the MQMD indicate the CCSID/Encoding of the      */
   /* message, and not those of the queue manager.               */
   /* Reset these fields so that we get the message in our own   */
   /* CCSID/Encoding.                                            */
   /**************************************************************/
   MsgDesc -> Encoding = MQENC_NATIVE ;
   MsgDesc -> CodedCharSetId = MQCCSI_Q_MGR ;

   odqMQGET( odqGlobal.odqHConn,
             *Hobj,
             MsgDesc,
             &GetMsgOpts,
             *BufferLength,
             *Buffer,
             pMsgLength,
             &CompCode,
             &Reason ) ;

   switch( CompCode )
   {
     case MQCC_OK:
        break;

     default:
        switch( Reason )
        {
     /*---------------------------------------------------------------*/
     /* If the queue manager is shutting down then we also end.       */
     /*---------------------------------------------------------------*/
          case MQRC_Q_MGR_QUIESCING:
          case MQRC_Q_MGR_STOPPING:
          case MQRC_CONNECTION_BROKEN:
               rc =  odq_ConnectionBroken ;
               break ;
     /*---------------------------------------------------------------*/
     /* The following errors can occur without implying any problem   */
     /* in the DLQ handler. Upon receipt of one of these errors then  */
     /* we will issue a message and terminate the DLQ handler.        */
     /*---------------------------------------------------------------*/
          case MQRC_GET_INHIBITED:
          case MQRC_OBJECT_CHANGED:
          case MQRC_Q_DELETED:
          case MQRC_RESOURCE_PROBLEM:
          case MQRC_STORAGE_NOT_AVAILABLE:
               odqFFDC( /*IN*/
                          FUNCTION_ID,
                          odq_GetNextErr,
                          CompCode,
                          Reason,
                          NULL
                        /*OUT*/
                      );
               rc =  odq_GetNextErr ;
               break ;
     /*---------------------------------------------------------------*/
     /* Any other error is unexpected and implies an error in this    */
     /* code, or an error in the queue manager.                       */
     /*---------------------------------------------------------------*/
           default:
              odqFFDC( /*IN*/
                         FUNCTION_ID,
                         odq_UnexpectedGetNextErr,
                         CompCode,
                         Reason,
                         NULL
                       /*OUT*/
                    );
              rc =  odq_UnexpectedGetNextErr ;
        }
   }
 }
 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return ( rc ) ;
}
#undef FUNCTION_ID

#define FUNCTION_ID odqtodqProcessMessage

/*********************************************************************/
/*                                                                   */
/* Function: odqProcessMessage                                       */
/*                                                                   */
/* Description: Attempt to invoke a matching rule for a message      */
/*                                                                   */
/* Intended Function: This function attempts to find a matching      */
/*                    rule for a message and to take the action      */
/*                    associated with that rule.                     */
/*                                                                   */
/* Input Parameters: queue handle  (used for context passing)        */
/*                   message handle                                  */
/*                   converted dead letter header                    */
/*                   message length (including DLH)                  */
/*                   message buffer address (including DLH)          */
/*                   message descriptor                              */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
static odqResp odqProcessMessage(
                          /*IN*/
                            const MQHOBJ  *Hobj,
                            const MQHMSG  *Hmsg,
                            const MQDLH   *dlh,
                            const MQLONG  MessageLength,
                            const MQBYTE  *Message,
                                  MQMD    *MsgDesc
                          /*OUT*/
                         )
{
  odqResp rc=odq_RetryNow;
  MQLONG CompCode=0 ;
  MQLONG Reason=0 ;
  MQLONG RetryCount;
  MQMD md ;
  MQOD od={MQOD_DEFAULT};
  MQPMO pmo={MQPMO_DEFAULT};
  const MQBYTE *PutMsg ;
  MQLONG PutLength;
  odqPattern_t *pPat ;

/*-------------------------------------------------------------------*/
/* We've been passed a used message, and the MQDLH describing why    */
/* the message was written to the DLQ.                               */
/* Build a default MQOD,MQMD,MQPMO for retrying the PUT and          */
/* call a user supplied function to determine what action should     */
/* be performed for this message, and then attempt to take that      */
/* action.                                                           */
/*-------------------------------------------------------------------*/
 odq_fnc_entry( FUNCTION_ID ) ;

/*--------------------------------------------------------------------*/
/* We want to coordinate the processing of the message with the       */
/* removal of the message form the DLQ.                               */
/* As we're executing as some form of mover we use                    */
/* MQPMO_PASS_ALL_CONTEXT to preserve the message context.            */
/*--------------------------------------------------------------------*/
 pmo.Version = MQPMO_VERSION_3 ;
 pmo.Options = MQPMO_SYNCPOINT + MQPMO_PASS_ALL_CONTEXT ;
 pmo.OriginalMsgHandle = *Hmsg ;
 pmo.Action = MQACTP_FORWARD ;

/*--------------------------------------------------------------------*/
/* Check that the message we've taken of the DLQ is prefixed          */
/* by an MQDLH.                                                       */
/* If not then assume that it should be ignored by this process, but  */
/* out a message warning the user that we hit a message we did not    */
/* expect.                                                            */
/*--------------------------------------------------------------------*/
if( !dlh || (memcmp(MQDLH_STRUC_ID,dlh->StrucId,4) != 0) ||
   (MQDLH_VERSION_1 != dlh->Version)              ||
    (MessageLength < MQDLH_LENGTH_1 )         )
{
  odqFFDC(/*IN*/
            FUNCTION_ID,
            odq_InvalidDLQHeader,
            0,
            0,
            dlh,
            sizeof(MQDLH),
            "MQDLH",
            NULL
          /*OUT*/
         );
        rc =  odq_InvalidDLQHeader ;
}
else
{
/*--------------------------------------------------------------------*/
/* Find out if we've seen this message already, and if so how often.  */
/*--------------------------------------------------------------------*/
  if (odq_Error > rc )
  {
    rc = odqInquireRetryCount( /*IN*/
                                 MsgDesc->MsgId,
                                 MsgDesc->CorrelId,
                               /*OUT*/
                                 &RetryCount
                             );
  }


/*--------------------------------------------------------------------*/
/* Try to find a rule that matches this message.                      */
/*--------------------------------------------------------------------*/
  if (odq_Error > rc )
  {
    rc = odqMatchMessage( dlh,
                          MsgDesc,
                          RetryCount,
                          &pPat);

  }

/*--------------------------------------------------------------------*/
/* If we found a matching rule then try to take the sepcified action. */
/*--------------------------------------------------------------------*/
  if (odq_PatternMatches == rc )
  {
    if(pPat->Parm[PutAut].i)
    {
      pmo.Options += MQPMO_ALTERNATE_USER_AUTHORITY ;
      memcpy(od.AlternateUserId,MsgDesc->UserIdentifier,MQ_USER_ID_LENGTH) ;
    }

    memcpy(&md,MsgDesc,sizeof(md));
    if (pPat->Parm[Header].i &&
        pPat->Parm[Action].i == odqFORWARD )
    {
      /* Forward the whole message unconverted including DLH */
      PutMsg = Message;
      PutLength = MessageLength;
    }
    else
    {
      /*--------------------------------------------------------------*/
      /* If we're not forwarding the message complete with MQDLH then */
      /* we need to set the md up to reflect the original message.    */
      /*--------------------------------------------------------------*/
      strncpy(md.Format,
              dlh->Format,
              MQ_FORMAT_LENGTH);
      md.Encoding = dlh->Encoding ;
      md.CodedCharSetId = dlh->CodedCharSetId ;
      PutMsg = Message + MQDLH_LENGTH_1;
      PutLength = MessageLength - MQDLH_LENGTH_1;
    }

    switch(pPat->Parm[Action].i)
    {
      case odqRETRY:
      case odqFORWARD:
           /*---------------------------------------------------------*/
           /* Remember that we've done something useful.              */
           /*---------------------------------------------------------*/
           time( &LastMessageProcessed );
           if (odqRETRY == pPat->Parm[Action].i)
           {
           strncpy(od.ObjectName,
                   dlh->DestQName,
                   MQ_Q_NAME_LENGTH);
           strncpy(od.ObjectQMgrName,
                   dlh->DestQMgrName,
                   MQ_Q_MGR_NAME_LENGTH);
           }
           else
           {
             if (pPat->Parm[ForwardQ].s[0] == '&')
             {
                if ((pPat->Parm[ForwardQ].s[1] == 'r') ||
                     (pPat->Parm[ForwardQ].s[1] == 'R'))
                {
                  strncpy(od.ObjectName,
                          md.ReplyToQ,
                          MQ_Q_NAME_LENGTH);
                }
                else
                {
                  strncpy(od.ObjectName,
                          dlh->DestQName,
                          MQ_Q_NAME_LENGTH);
                }
             }
             else
             {
                strncpy(od.ObjectName,
                        pPat->Parm[ForwardQ].s,
                        MQ_Q_NAME_LENGTH);
             }

             if( Specified == pPat->ParmSpecified[ForwardQMgr])
             {
               if (pPat->Parm[ForwardQMgr].s[0] == '&')
               {
                 if ((pPat->Parm[ForwardQMgr].s[1] == 'r') ||
                      (pPat->Parm[ForwardQMgr].s[1] == 'R'))
                 {
                   strncpy(od.ObjectQMgrName,
                           md.ReplyToQMgr,
                           MQ_Q_MGR_NAME_LENGTH);
                 }
                 else
                 {
                   strncpy(od.ObjectQMgrName,
                           dlh->DestQMgrName,
                           MQ_Q_MGR_NAME_LENGTH);
                 }
               }
               else
               {
                 strncpy(od.ObjectQMgrName,
                         pPat->Parm[ForwardQMgr].s,
                         MQ_Q_MGR_NAME_LENGTH);
               }
             }
             else
             {
               strncpy(od.ObjectQMgrName,
                       "",
                       MQ_Q_MGR_NAME_LENGTH);
             }
           }
           pmo.Context = *Hobj ;
           odqMQPUT1( odqGlobal.odqHConn,
                      &od,
                      &md,
                      &pmo,
                      PutLength,
                      (PMQVOID) PutMsg,
                      &CompCode,
                      &Reason ) ;
           if( MQCC_OK == CompCode )
           {
             rc =  odq_Ok ;
           }
           else
           {
             if (!memcmp(RetryMsgId,
                         MQMI_NONE,
                         sizeof(MsgDesc->MsgId)) &&
                 !memcmp(RetryCorrelId,
                         MQCI_NONE,
                         sizeof(MsgDesc->CorrelId)))
             {
               memcpy(RetryMsgId,
                      MsgDesc->MsgId,
                      sizeof(MsgDesc->MsgId));
               memcpy(RetryCorrelId,
                      MsgDesc->CorrelId,
                      sizeof(MsgDesc->CorrelId));
             }
             /*-------------------------------------------------------*/
             /* If we failed for the same reason as the message was   */
             /* put to the DLQ and we are trying to write to the same */
             /* queue then we assume that no new error has occurred   */
             /* and don't issue a message. This is handy for          */
             /* conditions such as Q_FULL and PUT_INHIBITED.          */
             /* This behavior is not so good for something like       */
             /* MQRC_UNKNOWN_OBJECT_Q_MGR but its matched by the      */
             /* uselessness of trying to use RETRY for these types    */
             /* of errors.                                            */
             /*-------------------------------------------------------*/
             if((dlh->Reason == Reason ) &&
                (0 == strncmp(od.ObjectName,
                              dlh->DestQName,
                              MQ_Q_NAME_LENGTH)) &&
                ( 0== strncmp(od.ObjectQMgrName,
                              dlh->DestQMgrName,
                              MQ_Q_MGR_NAME_LENGTH)) )
             {
               rc = odq_RetryLater ;
             }
             else
             {
             /*-------------------------------------------------------*/
             /* Otherwise inform the operator that we could not take  */
             /* the requested action.                                 */
             /*-------------------------------------------------------*/
               odqFFDC(/*IN*/
                         FUNCTION_ID,
                         odq_PutFailure,
                         pPat->LineNo,
                         Reason,
                         NULL
                       /*OUT*/
                      );
               rc = odq_PutFailure ;
             }
           }
           break ;
        case odqDISCARD:
           /*---------------------------------------------------------*/
           /* Remember that we've done something useful.              */
           /*---------------------------------------------------------*/
           time( &LastMessageProcessed );
           rc =  odq_Ok ;
           break ;
        case odqIGNORE:
        default:
           /*---------------------------------------------------------*/
           /* Because odqIGNORE can't fail and we don't need to       */
           /* record a retry count for this message.                  */
           /* We do however need to keep a record of the message so   */
           /* that we can recognise when new messages arrive, and     */
           /* thus implement WAIT correctly.                          */
           /* We can't afford ever to delete this history or the next */
           /* time we see the message then we'll go back to the first */
           /* action for the message. This is a potential problem as  */
           /* we can build up storage remembering messages that were  */
           /* ignored.                                                */
           /*---------------------------------------------------------*/
           rc =  odq_IgnoreMessage ;
           break;
    } /* switch(action) */
 }
 else
 {
   if ( odq_Error > rc )
   {
     rc =  odq_RetryLater ;
   }
 }
 /*-------------------------------------------------------------*/
 /* If we failed to take the requested action and this is our   */
 /* first attempt with this message then make a note of the     */
 /* message for subsequent retries.                             */
 /*-------------------------------------------------------------*/
 if(( rc == odq_RetryLater ||
      rc == odq_PutFailure ||
      rc == odq_IgnoreMessage
     ) &&
       RetryCount == 0 )
 {
   odqResp rc1 ;
   /*---------------------------------------------------------*/
   /* If this is an old message that the DLQ handler has been */
   /* ignoring then it doesn't count as something useful      */
   /* done by the DLQ handler. However if new messages are    */
   /* arriving but we're not interested in them then we still */
   /* think it's worth hanging around for a while longer.     */
   /*---------------------------------------------------------*/
   time( &LastMessageProcessed );
   rc1 = odqAddMsg( /*IN*/
                     MsgDesc->MsgId,
                     MsgDesc->CorrelId
                  );
   if( odq_Error <= rc1 )
   {
     rc = rc1 ;
   }
 }
}
odq_fnc_retcode( FUNCTION_ID, rc ) ;
return rc;
}

