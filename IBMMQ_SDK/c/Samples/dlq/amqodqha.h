/*  @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=include/amqodqha.h */
/*********************************************************************/
/*                                                                   */
/* Module Name: AMQODQHA.H                                           */
/*                                                                   */
/* Description: RUNMQDLQ header file                                 */
/*   <copyright                                                      */
/*   notice="lm-source-program"                                      */
/*   pids="5724-H72,"                                                */
/*   years="1994,2022"                                               */
/*   crc="0" >                                                       */
/*   Licensed Materials - Property of IBM                            */
/*                                                                   */
/*   5724-H72,                                                       */
/*                                                                   */
/*   (C) Copyright IBM Corp. 1994, 2022 All Rights Reserved.         */
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
/* This file contains:                                               */
/*                                                                   */
/*  - DLQ handler response codes                                     */
/*  - DLQ handler function prototypes                                */
/*                                                                   */
/* This file includes:                                               */
/*  - DLQ handler trace identifiers                                  */
/*  - DLQ handler keyword identifiers                                */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
/* Change History                                                    */
/*  pn Reason    Rls  Date     Orig.   Comments                      */
/*  -- --------  ---  ------   ----    ------------------            */
/* £A1=HY50066  520  010522   CTTNG:  Initialise Retry Count struct  */
/*                                                                   */
/*********************************************************************/
#if !defined(AMQODQHA_H)
#define AMQODQHA_H
#define odqANY_SUBSTRING '*'
#define odqANY_CHARACTER '?'

#if defined(DEFINE_GLOBALS) || defined(xcgvGLOBAL)
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

#include <amqodtha.h>

typedef enum
{
odqIGNORE,
odqDISCARD,
odqRETRY,
odqFORWARD
} odqAction_t;

typedef enum
{
 NotSpecified=0,
 Specified,
 ExplicitlyDefaulted
} odqParmSpec_t;



/*--------------------------------------------------------------------*/
/*  amqodkha.h generates a different set of declarations depending    */
/*  on the setting of ODQENUM. Include both sets of declarations      */
/*  here.                                                             */
/*--------------------------------------------------------------------*/
#include <amqodkha.h>
#define ODQENUM
#include <amqodkha.h>


/*--------------------------------------------------------------------*/
/*  Parameters are either pointers to character strings, or else      */
/*  integers.                                                         */
/*--------------------------------------------------------------------*/
typedef union
{
 char        *s;      /* string  */
 int         i;       /* integer */
 odqAction_t a;       /* action  */
}odqParm;

/*--------------------------------------------------------------------*/
/*  The type odqPattern_t is used to represent the internal layout    */
/*  of a template line.                                               */
/*  Each template tine is represented by an array indicating if the   */
/*  parameter is specified, and an array of values.                   */
/*  The corresponding line number in the input file is stored to      */
/*  help with any problem diagnosis.                                  */
/*--------------------------------------------------------------------*/
typedef struct _odqPattern_t
{
 struct _odqPattern_t *next;
 MQLONG                LineNo;
 odqParmSpec_t        *ParmSpecified ;
 odqParm               Parm[LastKeyword+1] ;
} odqPattern_t;


/*--------------------------------------------------------------------*/
/*  R E S P O N S E    C O D E S                                      */
/*--------------------------------------------------------------------*/
typedef enum
{
 /*-------------------------------------------------------------------*/
 /* Ok response codes are used to return information to the caller.   */
 /*-------------------------------------------------------------------*/
 odq_Ok=0,
    odq_RetryNow,
    odq_RetryLater,
    odq_IgnoreMessage,
    odq_PatternMatches,
    odq_PatternMismatch,
    odq_TerminateTimeout,
    odq_DuplicateConnect,
    odq_Starting,
    odq_Ending,
  odq_UsageMsg=0x10,
 /*-------------------------------------------------------------------*/
 /* Warning response codes typically indicate that an exception       */
 /* ocondition ccured (and appropriate diagnostics collected) but that*/
 /* the process should be allowed to continue.                        */
 /* The output values from a function that returns a warning can be   */
 /* relied upon.                                                      */
 /*-------------------------------------------------------------------*/
 odq_Warning=0x20,
    odq_InvalidDLQHeader,
    odq_PutFailure,
 /*-------------------------------------------------------------------*/
 /* Error response codes indicate that an anticipated error has       */
 /* occured (and appropriate diagnostics collected) and that the      */
 /* process should terminate.                                         */
 /* The output values from a function that returns a error cannot be  */
 /* relied upon.                                                      */
 /*-------------------------------------------------------------------*/
 odq_Error=0x40,
    odq_ConnectErr,              /* Unable to connect to queue manager*/
    odq_OpenQmgrErr,             /* Unable to open qmgr object        */
    odq_InquireQmgrErr,          /* Unable to inquire upon qmgr object*/
    odq_CloseQmgrErr,            /* Unable to close qmgr object       */
    odq_StartBrowseErr,          /* Unable to open DLQ for browse     */
    odq_EndBrowseErr,            /* Unable to close DLQ               */
    odq_ValueOutOfRange,         /* Unable to allocate message buffer */
    odq_GetNextErr,              /* Unable to browse message          */
    odq_SyncpointErr,            /* Unable to commit/backout          */
    odq_NoValidInput=0x50,
    odq_NoStorage,               /* Unable to acquire required storage*/
    odq_InvalidFieldLen,         /* Parameter exceeds maximum length  */
    odq_DuplicateKeyword,        /* Same keyword twice on single line */
    odq_unused_1,                /* Not currently in use              */
    odq_ConnectionBroken,        /* Queue manager has shut down       */
    odq_StorageError,            /* Error detected at freemain        */
    odq_InvalidInteger,          /* Integer outside range             */
    odq_InvalidInput,            /* One or more errors in input       */
    odq_InvalidInputComb,        /* Invalid combination of parms      */
 /*-------------------------------------------------------------------*/
 /* Disaster response codes indicate that an unexpected error has     */
 /* occured (and appropriate diagnostics collected) and that the      */
 /* process should terminate.                                         */
 /* The output values from a function that returns a disaster cannot  */
 /* be relied upon.                                                   */
 /*-------------------------------------------------------------------*/
 odq_Disaster=0x5F,
    odq_UnexpectedInitErr,       /* Unable to init common services    */
    odq_UnexpectedConnectErr,    /* Unexpected CompCode/Reason        */
    odq_UnexpectedQmgrOpenErr,   /* Unexpected error opening qmgr     */
    odq_UnexpectedQmgrInquireErr,/* Unexpected error querying qmgr    */
    odq_UnexpectedQmgrCloseErr,  /* Unexpected error closing qmgr     */
    odq_UnexpectedStartBrowseErr,/* Unexpected error opening DLQ      */
    odq_UnexpectedEndBrowseErr,  /* Unexpected error closing DLQ      */
    odq_UnexpectedGetNextErr,    /* Unexpected error browsing DLQ     */
    odq_UnexpectedSyncpointErr,  /* Unable to commit/backout          */
    odq_UnexpectedDisconnectErr, /* Unexpected CompCode/Reason on disc*/
#ifdef AMQ_AS400
    odq_FileErr=0x70,            /* Unable to open Input/Output file  */
#endif
 /*-------------------------------------------------------------------*/
 /* Message error ID for runmqdlq when we fail to load the libraries  */
 /* we require (server or client).                                    */
 /* The odq_Error range is all used so we have to put this after      */
 /*-------------------------------------------------------------------*/
 	odq_LoadLibraryFailed=0x97,  /* Unable to load Libraries          */
 odq_high_error
} odqResp ;

/*--------------------------------------------------------------------*/
/* Although it's bad practice from an encapsulation viewpoint we make */
/* the environment parms globals because it makes it easy to pass     */
/* things around between the parser and the rest of the handler.      */
/* It also makes it easier to give multiple versions of the code      */
/* different capabilities without defining too many different         */
/* interfaces.                                                        */
/*--------------------------------------------------------------------*/

typedef struct
{
odqPattern_t *odqPatternHead ;
MQHCONN      odqHConn ;
MQLONG       odqCCSID ;
int          odqConnected;
int          odqConnectAttempted;
int          odqInputErrorCount;
#ifdef AMQ_AS400
 FILE         *odqOutputFile;
 char         odqSpecialASCII[4];
#endif

} odqGLOBAL_t ;

GLOBAL odqGLOBAL_t odqGlobal ;

#define ASCII_NEWLINE '\n'
#define ASCII_PLUS '+'
#define ASCII_STAR '*'
#define ASCII_DASH '-'


#ifdef AMQ_AS400
/*--------------------------------------------------------------------*/
/* Define message inserts structure for AS400 messages                */
/*--------------------------------------------------------------------*/

typedef struct {
   MQLONG       SubCode1;
   MQLONG       SubCode2;
   char         *Insert1;
   char         *Insert2;
   char         *Insert3;
} odqMESSAGE_INSERTS_t;

/*--------------------------------------------------------------------*/
/* AS400 will allow an environment variable to pass input file info   */
/*--------------------------------------------------------------------*/
#define AMQ_INFILE "AMQ_INFILE"
#define odqCOMMAND "STRMQMDLQ"
int yylex_initialise(void);

#endif

/*--------------------------------------------------------------------*/
/* Turn on the mode flag to indicate that the file being opened       */
/* shouldn't be inherited by child processes (if a flag exists).      */
/*--------------------------------------------------------------------*/
#ifndef odqNO_INHERIT_MODE
#if !defined(_WIN32)
  #if defined(__GLIBC__) && (  (__GLIBC__ > 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 7)))
    #define odqNO_INHERIT_MODE(mode) mode "e"
  #else
    #define odqNO_INHERIT_MODE(mode) mode
  #endif
#else
  #define odqNO_INHERIT_MODE(mode) mode "N"
#endif
#endif

/*--------------------------------------------------------------------*/
/* F U N C T I O N    P R O T O T Y P E S                             */
/*--------------------------------------------------------------------*/
odqResp odqConnectQmgr( /*IN*/
                          const char *qm_name,
                          PMQCNO pCno
                        /*OUT*/
                      );


odqResp odqProcessQueue(
                         /*IN*/
                           const char    *dlqname
                         /*OUT*/
                       );


odqResp odqProcessStdin(
                        /*IN*/
                        /*OUT*/
                        void
                       );


odqResp odqMatchMessage( /*IN*/
                           const MQDLH   *dlh,
                           const MQMD    *MsgDesc,
                           const MQLONG  RetryCount,
                         /*OUT*/
                           odqPattern_t **pattern
                       );

void    odqInitialiseRetryCount(void);                     /* @A1A */
odqResp odqInquireRetryCount(
                           /*IN*/
                             const MQBYTE24 MsgId,
                             const MQBYTE24 CorrelId,
                           /*OUT*/
                             MQLONG *RetryCount
                         );
odqResp odqAddMsg(
                  /*IN*/
                    const MQBYTE24 MsgId,
                    const MQBYTE24 CorrelId
                  /*OUT*/
                  );
odqResp odqDeleteMsg(
                     /*IN*/
                       const MQBYTE24 MsgId,
                       const MQBYTE24 CorrelId
                     /*OUT*/
                    );

/*--------------------------------------------------------------------*/
/* Common services initialization occurs in two phases, the first when*/
/* runmqdlq is first started, and the second immediately after a      */
/* connection to the queue manager has been established.              */
/*--------------------------------------------------------------------*/
odqResp odqPreInitialize(int argc, char *argv[]) ;
odqResp odqInitialize(const char * QueueMgrName) ;


/*--------------------------------------------------------------------*/
/* odqTerminate:  performs queue manager related termination.         */
/*--------------------------------------------------------------------*/
void odqTerminate(void);

/*--------------------------------------------------------------------*/
/* odqFFDC:                                                           */
/*            This function is called to perform first failure data   */
/*            capture. This routine calls FFST to issue a message and */
/*            dump the appropriate data areas (if any).               */
/*                                                                    */
/*--------------------------------------------------------------------*/
void odqFFDC( /*IN*/
                const int function,
                const odqResp  MessageId,
                const MQLONG SubCode1,
                const MQLONG SubCode2,
                ...
              /*OUT*/
            );

/*--------------------------------------------------------------------*/
/* odqGetMem: obtain some storage                                     */
/*                                                                    */
/*--------------------------------------------------------------------*/
odqResp odqGetMem( /*IN*/
                     int length,
                     char * comment,
                   /*OUT*/
                     void **address
                 );

/*--------------------------------------------------------------------*/
/* odqFreeMem: release previously acquired storage                    */
/*                                                                    */
/*--------------------------------------------------------------------*/
odqResp odqFreeMem( /*IN*/
                      char * comment,
                      void * address
                    /*OUT*/
                  );
/*--------------------------------------------------------------------*/
/* odq_fnc_entry: trace entry to a function.                          */
/*                                                                    */
/*--------------------------------------------------------------------*/
void odq_fnc_entry(/*IN*/
                      const odqTraceId function
                   /*OUT*/
                 );
/*--------------------------------------------------------------------*/
/* odq_fnc_retcode: trace exit from a function.                       */
/*                                                                    */
/*--------------------------------------------------------------------*/
void odq_fnc_retcode(/*IN*/
                      const odqTraceId function,
                      const odqResp retcode
                     /*OUT*/
                    );

/*--------------------------------------------------------------------*/
/* odqParseArgs: parse arguments                                      */
/*                                                                    */
/*--------------------------------------------------------------------*/
odqResp odqParseArgs(/*IN*/
                        int  *pArgc,
                        char *argv[],
                        char **pUserId
                     /*OUT*/
                    );

/*--------------------------------------------------------------------*/
/* odqMQCONNX: Connect to a queue manager                             */
/*                                                                    */
/*--------------------------------------------------------------------*/
void odqMQCONNX( char * qm,
                 PMQCNO  pCno,
                 PMQLONG pCompCode,
                 PMQLONG pReason);

/*--------------------------------------------------------------------*/
/* odqMQBACK: Backs out all message puts/gets that occurred since last*/
/*            sync                                                    */
/*--------------------------------------------------------------------*/
void odqMQBACK( MQHCONN Hconn,
                PMQLONG  pCompCode,
                PMQLONG  pReason);

/*--------------------------------------------------------------------*/
/* odqMQCLOSE: Relinquishes access to an object                       */
/*                                                                    */
/*--------------------------------------------------------------------*/
void odqMQCLOSE(MQHCONN  Hconn,
                PMQHOBJ  pHobj,
                MQLONG   Options,
                PMQLONG  pCompCode,
                PMQLONG  pReason);

/*--------------------------------------------------------------------*/
/* odqMQCMIT: Commit changes                                          */
/*                                                                    */
/*--------------------------------------------------------------------*/
void odqMQCMIT ( MQHCONN Hconn,
                 PMQLONG  pCompCode,
                 PMQLONG  pReason);

/*--------------------------------------------------------------------*/
/* odqMQCRTMH: Create message handle                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
void odqMQCRTMH( MQHCONN  Hconn,
                 PMQVOID  pCrtMsgHOpts,
                 PMQHMSG  pHmsg,
                 PMQLONG  pCompCode,
                 PMQLONG  pReason);

/*--------------------------------------------------------------------*/
/* odqMQDISC: Disconnect queue manager                                */
/*                                                                    */
/*--------------------------------------------------------------------*/
void odqMQDISC( PMQHCONN pHconn,
                PMQLONG  pCompCode,
                PMQLONG  pReason);

/*--------------------------------------------------------------------*/
/* odqMQDLTMH: Delete message handle                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
void odqMQDLTMH( MQHCONN Hconn,
                 PMQHMSG pHmsg,
                 PMQVOID pDltMsgHOpts,
                 PMQLONG pCompCode,
                 PMQLONG pReason);

/*--------------------------------------------------------------------*/
/* odqMQGET: Get message                                              */
/*                                                                    */
/*--------------------------------------------------------------------*/
void odqMQGET( MQHCONN Hconn,
               MQHOBJ  Hobj,
               PMQVOID pMsgDesc,
               PMQVOID pGetMsgOpts,
               MQLONG  BufferLength,
               PMQVOID pBuffer,
               PMQLONG pDataLength,
               PMQLONG pCompCode,
               PMQLONG pReason);

/*--------------------------------------------------------------------*/
/* odqMQINQ: Connect to a queue manager                             */
/*                                                                    */
/*--------------------------------------------------------------------*/
void odqMQINQ( MQHCONN Hconn,
               MQHOBJ  Hobj,
               MQLONG  SelectorCount,
               PMQLONG pSelectors,
               MQLONG  IntAttrCount,
               PMQLONG pIntAttrs,
               MQLONG  CharAttrLength,
               PMQCHAR pCharAttrs,
               PMQLONG pCompCode,
               PMQLONG pReason);

/*--------------------------------------------------------------------*/
/* odqMQOPEN: Establishes access to an object                         */
/*                                                                    */
/*--------------------------------------------------------------------*/
void odqMQOPEN( MQHCONN Hconn,
                PMQVOID pObjDesc,
                MQLONG  Options,
                PMQHOBJ pHobj,
                PMQLONG pCompCode,
                PMQLONG pReason);

/*--------------------------------------------------------------------*/
/* odqMQPUT1: Put one message                                         */
/*                                                                    */
/*--------------------------------------------------------------------*/
void odqMQPUT1( MQHCONN  Hconn,
                PMQVOID  pObjDesc,
                PMQVOID  pMsgDesc,
                PMQVOID  pPutMsgOpts,
                MQLONG   BufferLength,
                PMQVOID  pBuffer,
                PMQLONG  pCompCode,
                PMQLONG  pReason);

#endif
