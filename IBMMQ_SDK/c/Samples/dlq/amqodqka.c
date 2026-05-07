const static char sccsid[] = "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=cmd/servers/amqodqka.c";
/*********************************************************************/
/*                                                                   */
/* Module Name: AMQODQKA.C                                           */
/*                                                                   */
/* Description: Common source for the IBM MQ Dead letter   */
/* handler.                                                          */
/*   <copyright                                                      */
/*   notice="lm-source-program"                                      */
/*   pids="5724-H72,"                                                */
/*   years="1994,2023"                                               */
/*   crc="" >                                                        */
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
/* Function:                                                         */
/* This file is the mainline routine for the runmqdlq command which  */
/* initiates the IBM MQ dead letter queue handler.         */
/*                                                                   */
/* The IBM MQ dead letter queue (DLQ) handler is a batch   */
/* MQ application which can be used to help automate the */
/* processing of messages written to the DLQ.                        */
/*                                                                   */
/* Syntax:                                                           */
/* The main source of input to the IBM MQ DLQ handler is from */
/* stdin, however two positional parameters are supported to allow   */
/* the stdin file to be written in a way which will allow a single   */
/* source file for stdin to apply to multiple queue managers.        */
/* Where a parameter is specified both as a positional parameter     */
/* and within stdin then the positional parameter takes precedence   */
/* and the parameter within stdin is ignored.                        */
/*                                                                   */
/*                                                                   */
/*   runmqdlq <-u userid> <queue_name <queue_mgr_name> < <InputFile> */
/*                                                                   */
/* Where                                                             */
/*     queue_name     Is the name of the dead letter queue to        */
/*                    process.                                       */
/*                    Default: use INPUTQ from InputFile             */
/*                                                                   */
/*     queue_mgr_name Is the name of the queue manager owning the    */
/*                    above dead letter queue.                       */
/*                    Default: use INPUTQM from InputFile            */
/*                                                                   */
/*                                                                   */
/*     note: If the queue_name is specified as ' ' then the          */
/*           DEADQ defined for the specified queue manager           */
/*           is used as the queue_name.                              */
/*           If the queue_mgr_name is specified as ' '               */
/*           then the default queue manager name for the node        */
/*           is used as the queue_mgr_name.                          */
/*                                                                   */
/* The -u <userid> parameter can be used for                         */
/* authentication of the connection. If it is set, the first line    */
/* of stdin must be the user's password. This parameter can only     */
/* be passed on the command line, not in the rules file.             */
/*                                                                   */
/* The DLQ handler receives input through stdin. The first line      */
/* after any password describes the environment in which the DLQ     */
/* handler is to execute and may contain the following               */
/* parameters:                                                       */
/*                                                                   */
/*  INPUTQM: The name of the queue manager whose dead letter queue   */
/*           is to be processed.                                     */
/*           Default: default queue manager (' ').                   */
/*                                                                   */
/*  INPUTQ:  The name of the dead letter queue to be processed.      */
/*           Default: The DEADQ defined for the INPUTQM.             */
/*                    (can be explicitly specified as ' ')           */
/*                                                                   */
/*  RETRYINT: The interval after which messages which could not be   */
/*           dealt with successfully by the DLQ handler will be      */
/*           retried by the DLQ handler.                             */
/*           Default: 60 (seconds).                                  */
/*                                                                   */
/*  WAIT(YES|NO|interval): The interval that the DLQ handler         */
/*           will wait for new messages after the DLQ becomes        */
/*           empty. Once the DLQ becomes empty the DLQ handler       */
/*           will wait for this interval for new work to arrive, if  */
/*           no new work arrives then the DLQ handler will terminate.*/
/*           YES => wait indefinitely.                               */
/*           NO  => terminate as soon as the queue is empty          */
/*           i   => wait for i seconds after the queue is empty.     */
/*           Default: YES                                            */
/*                                                                   */
/*                                                                   */
/*  Subsequent lines in the input contain templates used to match    */
/*  messages on the DLQ and an action associated with each           */
/*  template.                                                        */
/*                                                                   */
/*  Message Identifying Keywords:                                    */
/*                                                                   */
/*  FORMAT   The format name of the message put to the DLQ.          */
/*           Default: FORMAT(*)                                      */
/*                                                                   */
/*  DESTQ    The destination queue name of the message put to the    */
/*           DLQ.                                                    */
/*           Default: DESTQ(*)                                       */
/*                                                                   */
/*  DESTQM   The queue manager name of the destination queue of the  */
/*           message put to the DLQ.                                 */
/*           Default: DESTQM(*)                                      */
/*                                                                   */
/*  REPLYQ   The reply queue name of the message put to the DLQ.     */
/*           Default: REPLYQ(*)                                      */
/*                                                                   */
/*  REPLYQM  The queue manager name of the reply queue of the        */
/*           message put to the DLQ.                                 */
/*           Default: REPLYQM(*)                                     */
/*                                                                   */
/*  USERID   The UserIdentifier of the message put to the DLQ.       */
/*           Default: USERID(*)                                      */
/*                                                                   */
/*  APPLIDAT The ApplIdentityData of the message put to the DLQ.     */
/*           Default APPLIDAT(*)                                     */
/*                                                                   */
/*  APPLTYPE The PutApplType of the message put to the DLQ.          */
/*           Default: APPLTYPE(*)                                    */
/*                                                                   */
/*  APPLNAME The PutApplName of the message put to the DLQ.          */
/*           Default: APPLNAME(*)                                    */
/*                                                                   */
/*  REASON   The reason that the message was put to the DLQ          */
/*           Default: REASON(*)                                      */
/*                                                                   */
/*  MSGTYPE  The message type of the message put to the DLQ.         */
/*           Default: MSGTYPE(*)                                     */
/*                                                                   */
/*  FEEDBACK The feedback code of the message put to the DLQ         */
/*           Default: FEEDBACK(*)                                    */
/*                                                                   */
/*  PERSIST  The persistence of the message                          */
/*           Default: PERSIST(*)                                     */
/*                                                                   */
/*  Patterns for keywords which have MQCHAR values may include the   */
/*  wildcards '?' and '*' where '?' matches an single character      */
/*  other than a trailing blank, and * matches zero or more          */
/*  adjacent characters.                                             */
/*                                                                   */
/*  Patterns for keywords which have numeric values can use the '*'  */
/*  character by itself to indicate that any numeric value is        */
/*  acceptable but cannot use '*' or '?' as part of a numeric string.*/
/*                                                                   */
/*  Trailing blanks in patterns and in the corresponding fields are  */
/*  not considered significant although leading and embedded blanks  */
/*  are.                                                             */
/*                                                                   */
/*  Message Action Keywords:                                         */
/*  ACTION(IGNORE|DISCARD|RETRY|FWD)                                 */
/*                                                                   */
/*         IGNORE - leave the message unchanged on the DLQ.          */
/*         DISCARD - Remove the message from the DLQ and discard it. */
/*         RETRY - Attempt to send the message to its original       */
/*                 destination.                                      */
/*         FWD   - Attempt to send the message to the destination    */
/*                 identified by FWDQ and FWDQM.                     */
/*                                                                   */
/*                                                                   */
/*  FWDQ    The name of the queue to use when the action is FWD.     */
/*          If specified as '=' the queue name is taken from the     */
/*          ReplyToQ field in the message descriptor.                */
/*          This keyword must be specified if ACTION(FWD) is         */
/*          specified and must not be specified otherwise.           */
/*                                                                   */
/*  FWDQM   The name of the queue manager to use when the action is  */
/*          FWD.                                                     */
/*          If specified as '=' the queue mgr name is taken from the */
/*          ReplyToQMgr field in the message descriptor.             */
/*          This keyword may be specified if ACTION(FWD) is          */
/*          specified and must not be specified otherwise.           */
/*                                                                   */
/*  HEADER  This indicates whether the MQDLH header should be        */
/*          removed (HEADER(NO)) or left as is (HEADER(YES)) when    */
/*          the message is forwarded.                                */
/*          This keyword is only valid if ACTION(FWD) is specified.  */
/*                                                                   */
/*  PUTAUT  This indicates the authority with which a message should */
/*          be PUT by the DLQ handler.                               */
/*          PUTAUT(DEF) causes the message to be put with the        */
/*                      authority of the DLQ handler itself.         */
/*          PUTAUT(CTX) causes the message to be put with the        */
/*                      authority of the userid in the message       */
/*                      context.                                     */
/*          Default: PUTAUT(DEF)                                     */
/*                                                                   */
/*  RETRY   The number of times that the specified action should be  */
/*          attempted (at intervals as dictated in RETRYINT). Once   */
/*          the specified number of retries have been attempted then */
/*          the table will be searched for a further matching entry. */
/*                                                                   */
/*********************************************************************/
/* Change History                                                    */
/*  pn Reason    Rls  Date     Orig.   Comments                      */
/*  -- --------  ---  ------   ----    ------------------            */
/* �A1=PHY50066  520  010522   CTTNG:  Initialise Retry Count struct */
/*                                                                   */
/*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmqc.h>
#define DEFINE_GLOBALS
#include <amqodqha.h>
#undef DEFINE_GLOBALS



/*-------------------------------------------------------------------*/
/*                                                                   */
/*    S T A T I C    F U N C T I O N    P R O T O T Y P E S          */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
/* odqInquireQMAttrs: Obtain the default DLQ name for the currently */
/*                    connected queue manager, and the default      */
/*                    ccsid for the queue manager.                  */
/*                                                                  */
/*------------------------------------------------------------------*/
static odqResp odqInquireQMAttrs( /*IN*/
                                  /*OUT*/
                                  char *dlqname,
                                  MQLONG *ccsid
                                );

/*------------------------------------------------------------------*/
/* odqdisConnectQmgr: disconnect from the currently connected queue */
/*                    manager.                                      */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
static odqResp odqDisconnectQmgr( /*IN*/
                                  void
                                  /*OUT*/
                                );

/*------------------------------------------------------------------*/
/*                                                                  */
/*     E X E C U T A B L E     C O D E                              */
/*                                                                  */
/*------------------------------------------------------------------*/
#define FUNCTION_ID odqtmain

MQCNO   cno = {MQCNO_DEFAULT};

/*********************************************************************/
/*                                                                   */
/* Function: main                                                    */
/*                                                                   */
/* Description: run the dead letter queue handler                    */
/*                                                                   */
/* Intended Function: This is the entry point for execution of the   */
/*                    default IBM MQ dead letter queue handler.      */
/*                                                                   */
/* Input Parameters: <arg[1]=queue name>                             */
/*                   <arg[2]=queue manager name>                     */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
int main( int argc, char *argv[] )
{
  odqResp rc ;                  /* DLQ internal response    */
  char             dlqname[MQ_Q_NAME_LENGTH+1];

  MQCSP   csp = {MQCSP_DEFAULT};
  char   *UserId = NULL;
  char    Password[MQ_CSP_PASSWORD_LENGTH + 1] = {0}; /* For auth  */

  /*------------------------------------------------------------------*/
  /*      Initialize basic services (trace, dump, etc)                */
  /*      Parse the input file                                        */
  /*      Call the browser to process the queue                       */
  /*      Disconnect from the queue manager                           */
  /*      return.                                                     */
  /*                                                                  */
  /* note: The lack of symmetry in connecting/disconnecting from      */
  /*       the queue manager is a result of finding the queue manager */
  /*       name in the parser and wishing to connect to the queue     */
  /*       manager as early as possible so that errors are logged     */
  /*       to the correct destination.                                */
  /*------------------------------------------------------------------*/
  rc = odqPreInitialize(argc, argv);

  /*------------------------------------------------------------------*/
  /* Since we now look for extra arguments for runmqdlq, call the     */
  /* correct function to deal with the checking for -u and other args */
  /*------------------------------------------------------------------*/
  if( odq_Error > rc )
  {
   rc = odqParseArgs(&argc, argv, &UserId);
  }

  if (UserId && odq_Error > rc)
  {
   /****************************************************************/
   /* Set the connection options to use the security structure and */
   /* set version information to ensure the structure is processed.*/
   /****************************************************************/
   cno.SecurityParmsPtr = &csp;
   cno.Version = MQCNO_VERSION_5;

   csp.AuthenticationType = MQCSP_AUTH_USER_ID_AND_PWD;
   csp.CSPUserIdPtr = UserId;
   csp.CSPUserIdLength = (MQLONG)strlen(UserId);

   fgets(Password,sizeof(Password)-1,stdin);

   if (strlen(Password) > 0 && Password[strlen(Password) - 1] == '\n')
     Password[strlen(Password) -1] = 0;

   csp.CSPPasswordPtr = Password;
   csp.CSPPasswordLength = (MQLONG)strlen(csp.CSPPasswordPtr);
  }

  /*------------------------------------------------------------------*/
  /*      Check to see if the first char of the second argument is    */
  /*      either a ? or -  indicating that the usage message is to    */
  /*      be displayed.  don't need to check if -? as the - is an     */
  /*      invalid char any way and so a usage message should be       */
  /*      displayed.                                                  */
  /*------------------------------------------------------------------*/
  if( ( odq_Error > rc ) &&
      ( argc > 1 ) &&
      ( (argv[1][0] == '?')
#ifndef AMQ_AS400
        || (argv[1][0] == '-')
#endif
      ) )
  {
    rc = odq_UsageMsg;
    odqFFDC ( /*IN*/
              FUNCTION_ID,
              odq_UsageMsg,
              0,
              0,
              NULL
              /*OUT*/
            );
  }
  else
  {
    if(odq_Error > rc && argc > 2 )
    {
#ifdef AMQ_AS400
      /* -m indicates the queue manager should be taken from stdin  */
      if(strcmp(argv[2],"-m") != 0)
#endif
        rc = odqConnectQmgr( argv[2] , &cno) ;
    }
    if( odq_Error > rc )
    {
      /*---------------------------------------------------------*/
      /* Parse the input data now, checking for syntax errors    */
      /* and building an in core image of the patterns.          */
      /* The odqProcessStdin routine will call back to this      */
      /* program to connect to the queue manager as soon as the  */
      /* queue manager name is known.                            */
      /*---------------------------------------------------------*/
      rc = odqProcessStdin();
      if(odq_Error > rc)
      {
        rc = odqInquireQMAttrs( dlqname, &odqGlobal.odqCCSID ) ;
      }
      if(odq_Error > rc)
      {
        /*-------------------------------------------------------*/
        /* If an explicit queue name was provided then process   */
        /* that queue as if it were the dead letter queue.       */
        /* An explicit name may be provided either as the first  */
        /* positional parameter to runmqdlq, or may be provided  */
        /* using the INPUTQ keyword in the input file.           */
        /* If both methods of providing an explicit name are used*/
        /* then the positional parameter takes precedence.       */
        /* If the queue name is explicitly specified as ' ' or   */
        /* no queue name is specified then the DEADQ defined     */
        /* for the queue manager is used.                        */
        /*-------------------------------------------------------*/
        if((argc > 1)
#ifdef AMQ_AS400
           /* -q indicates the queue name should be taken from stdin */
           && (strcmp(argv[1],"-q") != 0)
#endif
          )
        {
          int i;
          /*----------------------------------------------------*/
          /* look for the first non blank in the name           */
          /*----------------------------------------------------*/
          for( i = 0 ;
             argv[1][i] == ' ';
             i ++ );
          /*----------------------------------------------------*/
          /* if the first non blank is not an end of string     */
          /* then the string is not comprised of only blanks    */
          /*----------------------------------------------------*/
          if(argv[1][i] != 0 )
          {
            strncpy( dlqname,
                     (char *)argv[1],
                     MQ_Q_NAME_LENGTH ) ;
          }
        }
        else
          if( odqGlobal.odqPatternHead->ParmSpecified[InputQ] == Specified &&
              odqGlobal.odqPatternHead->Parm[InputQ].s != NULL &&
              odqGlobal.odqPatternHead->Parm[InputQ].s[0] != ' ' )
        {
          strncpy( dlqname,
                   odqGlobal.odqPatternHead->Parm[InputQ].s,
                   MQ_Q_NAME_LENGTH ) ;
        }


        /*-------------------------------------------------------*/
        /* Call the queue processor (amqodqla.c) to process      */
        /* the queue.                                            */
        /*-------------------------------------------------------*/
        if( odq_Error > rc )
        {
          int size;

          for( size = 0 ;
             dlqname[size] != ' ' &&
             dlqname[size] != 0 &&
             size < MQ_Q_NAME_LENGTH ;
             size ++ ) ;

          dlqname[size] = 0 ;

          odqFFDC ( /*IN*/
                    FUNCTION_ID,
                    odq_Starting,
                    0,
                    0,
                    dlqname,
                    size,
                    "DeadLetterQueue",
                    NULL
                    /*OUT*/
                  );
          rc = odqProcessQueue( /*IN*/
                                dlqname
                                /*OUT*/
                              );
        }
      }
    }

    /*--------------------------------------------------------*/
    /* If we get a connection broken at this point and try to */
    /* perform an FDC then we will actually fall into trouble */
    /* further down the line because common services is on    */
    /* it's way out. To avoid this we skip the FDC call in    */
    /* this one case.                                         */
    /*--------------------------------------------------------*/

    if(odq_ConnectionBroken != rc)
    {
      odqFFDC ( /*IN*/
                FUNCTION_ID,
                odq_Ending,
                0,
                0,
                NULL
                /*OUT*/
              );
    }

    /*-------------------------------------------------------*/
    /* If we connected to the queue manager then we must     */
    /* disconnect from the queue manager.                    */
    /* If we've suffered an error then we need to explicitly */
    /* rollback to prevent an implicit commit.               */
    /*-------------------------------------------------------*/
    if (1 == odqGlobal.odqConnected)
    {
      if (rc >= odq_Error)
      {
        MQLONG tmpCC, tmpRC;
        odqMQBACK(odqGlobal.odqHConn, &tmpCC, &tmpRC);
      }
      odqDisconnectQmgr();
    }
  } /* if (odq_Error > rc && ( (argv[1][0] == '?') || (argv[1][0] == '-'))) */

  /* always cancel the odqPreInitialize */
  odqTerminate();

  return( rc );
}


#undef FUNCTION_ID
#define FUNCTION_ID odqtodqConnectQmgr
/*------------------------------------------------------------------*/
/* odqConnectQmgr:                                                  */
/*    Connect to the required queue manager. If the connect should  */
/*    fail then produce appropriate diagnostics.                    */
/*------------------------------------------------------------------*/

/*********************************************************************/
/*                                                                   */
/* Function: odqConnectQmgr                                          */
/*                                                                   */
/* Description: Connect to the nominated queue manager               */
/*                                                                   */
/* Intended Function: Connect to the nominated queue manager.        */
/*                    If the connect should fail then produce the    */
/*                    necessary diagnostics.                         */
/*                                                                   */
/* Input Parameters: queue manager name                              */
/*                   connection options                              */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
odqResp odqConnectQmgr( /*IN*/
                        const char *qm_name,
                        PMQCNO pCno
                        /*OUT*/
                      )
{
  MQLONG  CompCode ;
  MQLONG  Reason ;
  odqResp rc;

  odq_fnc_entry( FUNCTION_ID ) ;
  if( 1 != odqGlobal.odqConnectAttempted)
  {
    odqGlobal.odqConnectAttempted = 1 ;

    /* Now we know the queue manager name we can initialize properly */
    (void)odqInitialize(qm_name);


    odqMQCONNX( (char *)qm_name,
               pCno,
               &CompCode,
               &Reason );


    switch( CompCode )
    {
    case MQCC_OK:
      odqGlobal.odqConnected = 1 ;
      rc = odq_Ok ;
      break ;

    case MQCC_WARNING:
      switch( Reason )
      {
      case MQRC_ALREADY_CONNECTED:
        /*
        We don't really expect to get a response of already connected
        but if this program has been frontended then it's possible
        and as INT32 as we're connected now then we don't mind.
        */
        odqGlobal.odqConnected = 0 ;
        rc =  odq_Warning ;
        break;

      case MQRC_CLUSTER_EXIT_LOAD_ERROR:
        /*
        If there is a problem with the cluster workload exit then
        we do not want to continue (otherwise messages may be sent
        to the wrong destinations), so promote this warning to an error
        */
        odqFFDC ( /*IN*/
                  FUNCTION_ID,
                  odq_ConnectErr,
                  CompCode,
                  Reason,
                  qm_name,
                  MQ_Q_NAME_LENGTH,
                  "QueueMgrName",
                  NULL
                  /*OUT*/
                );
        odqGlobal.odqConnected = 1 ;
        rc =  odq_ConnectErr ;
        break;

      default:
        /*
        There are no other expected warnings on connect.
        */
        odqFFDC ( /*IN*/
                  FUNCTION_ID,
                  odq_UnexpectedConnectErr,
                  CompCode,
                  Reason,
                  qm_name,
                  MQ_Q_NAME_LENGTH,
                  "QueueMgrName",
                  NULL
                  /*OUT*/
                );
        rc =  odq_UnexpectedConnectErr ;
        break;
      }
      break ;
    default:
      switch( Reason )
      {
      /*
         Expected 'user errors'
      */
      case MQRC_ANOTHER_Q_MGR_CONNECTED:
      case MQRC_NOT_AUTHORIZED:
      case MQRC_Q_MGR_NAME_ERROR:
      case MQRC_Q_MGR_NOT_AVAILABLE:
      case MQRC_Q_MGR_QUIESCING:
      case MQRC_Q_MGR_STOPPING:
      case MQRC_RESOURCE_PROBLEM:
      case MQRC_SECURITY_ERROR:
      case MQRC_TOKEN_TIMESTAMP_NOT_VALID:
      case MQRC_STORAGE_NOT_AVAILABLE:
      case MQRC_STANDBY_Q_MGR:
      case MQRC_INSTALLATION_MISMATCH:
      case MQRC_HOST_NOT_AVAILABLE:
      case MQRC_CHANNEL_CONFIG_ERROR:
      case MQRC_UNKNOWN_CHANNEL_NAME:
      case MQRC_CHANNEL_NOT_AVAILABLE:
      case MQRC_CONNECTION_NOT_AVAILABLE:

        odqFFDC ( /*IN*/
                  FUNCTION_ID,
                  odq_ConnectErr,
                  CompCode,
                  Reason,
                  qm_name,
                  MQ_Q_NAME_LENGTH,
                  "QueueMgrName",
                  NULL
                  /*OUT*/
                );
        rc =  odq_ConnectErr ;
        break ;
      default:
        odqFFDC ( /*IN*/
                  FUNCTION_ID,
                  odq_UnexpectedConnectErr,
                  CompCode,
                  Reason,
                  qm_name,
                  MQ_Q_NAME_LENGTH,
                  "QueueMgrName",
                  NULL
                  /*OUT*/
                );
        rc =  odq_UnexpectedConnectErr ;
      }
    }
  }
  else
  {
    rc = odq_DuplicateConnect ;
  }
  odq_fnc_retcode( FUNCTION_ID, rc ) ;
  return rc ;
}

#undef FUNCTION_ID
#define FUNCTION_ID odqtodqDisconnectQmgr

/*********************************************************************/
/*                                                                   */
/* Function: odqDisconnectQmgr                                       */
/*                                                                   */
/* Description: Disconnect from the currently connected queue mgr.   */
/*                                                                   */
/* Intended Function: Disconnect from the currently connected queue  */
/*                    manager. If the disconnect should fail then    */
/*                    produce the necessary diagnostic materials.    */
/*                                                                   */
/* Input Parameters: none                                            */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
static odqResp odqDisconnectQmgr( /*IN*/
                                  void
                                  /*OUT*/
                                )
{
  odqResp rc ;
  MQLONG  CompCode ;
  MQLONG  Reason ;
  odq_fnc_entry( FUNCTION_ID ) ;

  odqMQDISC (&odqGlobal.odqHConn,
             &CompCode,
             &Reason );

  switch( CompCode )
  {
  case MQCC_OK:
    rc = odq_Ok ;
    break ;
  default:
    switch(Reason)
    {
    /*
      The following errors are treated as odq_Ok as the
      connection to the queue manager has ended and we have
      detected no error.
    */
    case MQRC_CONNECTION_BROKEN:
    case MQRC_Q_MGR_STOPPING:
      rc = odq_Ok ;
      break ;
      /*
        The following errors are treated as warnings as they do not
        indicate that anything is seriously wrong and we will rely
        on the queue manager cleaning up when our process terminates
      */
    case MQRC_RESOURCE_PROBLEM:
    case MQRC_STORAGE_NOT_AVAILABLE:
      rc = odq_Warning ;
      break ;

    default:
      odqFFDC ( /*IN*/
                FUNCTION_ID,
                odq_UnexpectedDisconnectErr,
                CompCode,
                Reason ,
                NULL
                /*OUT*/
              );
      rc =  odq_UnexpectedDisconnectErr ;
    }
  }

  odq_fnc_retcode( FUNCTION_ID, rc ) ;
  return rc ;
}

#undef  FUNCTION_ID
#define FUNCTION_ID odqtodqInquireQMAttrs
/*********************************************************************/
/*                                                                   */
/* Function: odqInquireQMAttrs                                       */
/*                                                                   */
/* Description: obtain the name of the default DLQ.                  */
/*                                                                   */
/* Intended Function: This function inquires upon the name of the    */
/*                    DEADQ defined for the currently connected      */
/*                    queue manager.                                 */
/*                                                                   */
/* Input Parameters: none                                            */
/*                                                                   */
/* Output Parameters: dead letter queue name                         */
/*                    coded character set id                         */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
static odqResp odqInquireQMAttrs( /*IN*/
                                  /*OUT*/
                                  char *dlqname,
                                  MQLONG *ccsid
                                )
{
  MQHOBJ Hobj;
  MQOD Obj = {MQOD_DEFAULT};
  const MQLONG Options=MQOO_INQUIRE ;
  MQLONG CompCode;
  MQLONG Reason;
  MQLONG Select[2] = {MQCA_DEAD_LETTER_Q_NAME, MQIA_CODED_CHAR_SET_ID};
  odqResp rc ;

  odq_fnc_entry( FUNCTION_ID );

  /*-------------------------------------------------------------------*/
  /* Open the queue manager object                                     */
  /*-------------------------------------------------------------------*/
  Obj.ObjectType = MQOT_Q_MGR ;
  odqMQOPEN( odqGlobal.odqHConn,
             &Obj,
             Options,
             &Hobj,
             &CompCode,
             &Reason ) ;

  switch( CompCode )
  {
  case MQCC_OK:
    {
      rc = odq_Ok ;
      break ;
    }
  default:
    {
      switch( Reason )
      {
      case MQRC_CONNECTION_BROKEN:
        rc =  odq_ConnectionBroken ;
        break ;
        /*---------------------------------------------------------------*/
        /* The following errors can occur without implying any problem   */
        /* in the DLQ handler. Upon receipt of one of these errors then  */
        /* we will issue a message and terminate the DLQ handler.        */
        /*---------------------------------------------------------------*/
      case MQRC_HANDLE_NOT_AVAILABLE:
      case MQRC_NOT_AUTHORIZED:
      case MQRC_Q_MGR_QUIESCING:
      case MQRC_Q_MGR_STOPPING:
      case MQRC_RESOURCE_PROBLEM:
      case MQRC_STORAGE_NOT_AVAILABLE:
        odqFFDC( /*IN*/
                 FUNCTION_ID,
                 odq_OpenQmgrErr,
                 CompCode,
                 Reason,
                 NULL
                 /*OUT*/
               );
        rc =  odq_OpenQmgrErr ;
        break ;
      default:
        /*---------------------------------------------------------------*/
        /* Any other error is unexpected and implies an error in this    */
        /* code, or an error in the queue manager.                       */
        /*---------------------------------------------------------------*/

        odqFFDC( /*IN*/
                 FUNCTION_ID,
                 odq_UnexpectedQmgrOpenErr,
                 CompCode,
                 Reason,
                 NULL
                 /*OUT*/
               );
        rc =  odq_UnexpectedQmgrOpenErr ;
        break ;
      }
    }
  }
  /*-------------------------------------------------------------------*/
  /* If the open succeeded then inquire upon the dead letter queue name*/
  /*-------------------------------------------------------------------*/
  if( odq_Ok == rc )
  {
    odqMQINQ( odqGlobal.odqHConn,
              Hobj,
              2,
              Select,
              1,
              ccsid,
              MQ_Q_NAME_LENGTH,
              dlqname,
              &CompCode,
              &Reason ) ;
    switch( CompCode )
    {
    case MQCC_OK:
      {
        break ;
      }
    case MQCC_FAILED:
    default:
      {
        switch( Reason )
        {
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
                   odq_InquireQmgrErr,
                   CompCode,
                   Reason,
                   NULL
                   /*OUT*/
                 );
          rc =  odq_InquireQmgrErr ;
          break ;
          /*---------------------------------------------------------------*/
          /* Any other error is unexpected and implies an error in this    */
          /* code, or an error in the queue manager.                       */
          /*---------------------------------------------------------------*/
        default:
          odqFFDC( /*IN*/
                   FUNCTION_ID,
                   odq_UnexpectedQmgrInquireErr,
                   CompCode,
                   Reason,
                   NULL
                   /*OUT*/
                 );
          rc =  odq_UnexpectedQmgrInquireErr ;
          break ;
        }
      }
    }
  }
  if( odq_Ok == rc )
  {
    odqMQCLOSE( odqGlobal.odqHConn,
                &Hobj,
                0,
                &CompCode,
                &Reason ) ;
    switch( CompCode )
    {
    case MQCC_OK:
      break;
    default:
      switch( Reason )
      {
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
                 odq_CloseQmgrErr,
                 CompCode,
                 Reason,
                 NULL
                 /*OUT*/
               );
        rc =  odq_CloseQmgrErr ;
        break ;
      default: ;
        /*---------------------------------------------------------------*/
        /* Any other error is unexpected and implies an error in this    */
        /* code, or an error in the queue manager.                       */
        /*---------------------------------------------------------------*/
        odqFFDC( /*IN*/
                 FUNCTION_ID,
                 odq_UnexpectedQmgrCloseErr,
                 CompCode,
                 Reason,
                 NULL
                 /*OUT*/
               );
        rc =  odq_UnexpectedQmgrCloseErr ;
        break ;
      }
    }
  }
  odq_fnc_retcode( FUNCTION_ID, rc ) ;
  return rc;

}

