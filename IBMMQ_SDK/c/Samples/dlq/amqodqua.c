const static char sccsid[] = "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=cmd/servers/amqodqua.c";
/*********************************************************************/
/*                                                                   */
/* Module Name: AMQODQUA.C                                           */
/*                                                                   */
/* Description: Common source for the IBM MQ Dead letter             */
/* handler.                                                          */
/*   <copyright                                                      */
/*   notice="lm-source-program"                                      */
/*   pids="5724-H72"                                                 */
/*   years="1994,2022"                                               */
/*   crc="2122534788" >                                              */
/*   Licensed Materials - Property of IBM                            */
/*                                                                   */
/*   5724-H72                                                        */
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
/* This file is the common source file which contains all of the     */
/* functions which are private to the sample version of the dead     */
/* letter handler.                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <cmqc.h>
#include <amqodqha.h>


static int indent;            /* Trace indentation level            */
static FILE *MsgFile ;        /* Console messages                   */
static int odq_trace ;        /* Trace on/off indicator             */
static int odq_msg ;          /* Console messages enabled indicator */
static fpos_t *MsgIndex;      /* Console message file index         */
#define ODQ_FUNCTION_NAMES
struct {
     char name[40];
     unsigned int reserved;
  } odq_function[] = {
#include <amqodtha.h>
};

#undef FUNCTION_ID
#define FUNCTION_ID odqtodqPreInitialize
/*********************************************************************/
/*                                                                   */
/* Function: odqPreInitialize                                        */
/*                                                                   */
/* Description: Enable basic MQ services                             */
/*                                                                   */
/* Intended Function: This function initializes basic IBM MQ         */
/*                    services such as trace/dump etc.               */
/*                                                                   */
/* Input Parameters: none                                            */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
odqResp odqPreInitialize(int argc, char *argv[])
{
 char * trace ;
 char * FileName;
 char   Buffer[256] ;
 int    MsgId,matches ;
 fpos_t MsgPos ;
 odqResp rc = odq_Ok ;

/*--------------------------------------------------------------------*/
/* Use an environment variable to detect if tracing is to be active.  */
/*                                                                    */
/*--------------------------------------------------------------------*/
 trace = getenv( "ODQ_TRACE" );
 if( (NULL != trace) &&
     ( (strncmp(trace,"YES", strlen(trace)) == 0) |
       (strncmp(trace,"yes", strlen(trace)) == 0)  ) )
 {
   odq_trace = 1 ;
 }
 else
 {
   odq_trace = 0 ;
 }

/*--------------------------------------------------------------------*/
/* Now that tracing is initialized we can trace entry to this         */
/* function.                                                          */
/*--------------------------------------------------------------------*/
 odq_fnc_entry( FUNCTION_ID );

/*--------------------------------------------------------------------*/
/* Open the message catalog.                                          */
/*--------------------------------------------------------------------*/
 FileName = getenv("ODQ_MSG");
 if (FileName == NULL)
 {
   fprintf( stderr,
            "odq-001  Environment variable \"ODQ_MSG\" not set\n") ;
   rc = odq_UnexpectedInitErr ;
 }
 else
 {
   MsgFile = fopen( FileName, odqNO_INHERIT_MODE("r")) ;
   if ( NULL == MsgFile )
   {
     fprintf( stderr,
              "odq-000  Unable to open message catalog\n"
              "check environment variable \"ODQ_MSG\" is set to the\n"
              "path name of the message file\n") ;
     rc = odq_UnexpectedInitErr ;
   }
   else
   {
   /*-----------------------------------------------------------------*/
   /* Obtain storage for the message index.                           */
   /*-----------------------------------------------------------------*/
     rc = odqGetMem ( sizeof( fpos_t )*odq_high_error,
                      "MessageIndex",
                      (void **)&MsgIndex ) ;
     if( odq_Error > rc )
     {
       memset( MsgIndex, 0, sizeof( fpos_t )*odq_high_error);
       /*-------------------------------------------------------------*/
       /* Browse the message file building an index as we go.         */
       /*-------------------------------------------------------------*/
       while( 0 == fgetpos( MsgFile, &MsgPos ) &&
              NULL != fgets( Buffer, sizeof(Buffer), MsgFile ))
       {
          matches = sscanf( Buffer, "odq-%d %*s", &MsgId  );
          /*----------------------------------------------------------*/
          /* Ignore any line that does not start with a message id.   */
          /*----------------------------------------------------------*/
          if( 0 < matches )
          {
            *(MsgIndex + MsgId) = MsgPos ;
          }
       }
       /*-------------------------------------------------------------*/
       /* Indicate that messages can be sent.                         */
       /*-------------------------------------------------------------*/
       odq_msg = 1 ;
     }
   }
 }
 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return rc;
}


#undef FUNCTION_ID
#define FUNCTION_ID odqtodqParseArgs
/*********************************************************************/
/*                                                                   */
/* Function: odqParseArgs                                            */
/*                                                                   */
/* Description: Parsing of arguments for dead letter queue handler   */
/*                                                                   */
/* Intended Function: This function Parsing the arguments given to   */
/*                    the DLQ handler, looking for special args      */
/*                    (like -u).                                     */
/*                                                                   */
/* Input Parameters: pArgc                                           */
/*                   argv                                            */
/*                   pUserId                                         */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
odqResp odqParseArgs(int *pArgc, char *argv[], char **pUserId)
{
  odqResp rc = odq_Ok;
  int i = 0, skipped = 0;

  char *DLQName = NULL, *QMName = NULL;

  odq_fnc_entry( FUNCTION_ID );

  /*------------------------------------------------------------------*/
  /* Look to see if a userid has been defined for connection          */
  /* authentication. This must be done before trying to parse the     */
  /* rules table as the first line of stdin is taken to be the        */
  /* password.                                                        */
  /*------------------------------------------------------------------*/
  for(i = 1; i < *pArgc; i++)
  {
    /* -u */
    if ((argv[i][0] == '-') && (argv[i][1] == 'u'))
    {
      /* -u UserId */
      if (argv[i][2] == '\0')
      {
        if ((i + 1) < *pArgc)
        {
          *pUserId = argv[i + 1];

          skipped = skipped + 2;
          i++;
        }
        else
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
          break;
        }
      }
      /* -uUserid */
      else
      {
        *pUserId = argv[i] + 2;
        skipped++;
      }
    }
    else
    {
      if(!DLQName)
      {
        DLQName = argv[i];
      }
      else if(!QMName)
      {
        QMName = argv[i];
      }
    }
  }

  if (skipped)
  {
    /*----------------------------------------------------------------*/
    /* Set the DLQ name and the QM name back in the correct order     */
    /*----------------------------------------------------------------*/
    if(DLQName)
    {
      argv[1] = DLQName;
    }
    if(QMName)
    {
      argv[2] = QMName;
    }

    *pArgc = *pArgc - skipped;
    argv[*pArgc] = NULL;
  }

  odq_fnc_retcode( FUNCTION_ID, rc ) ;
  return rc;
}

#undef FUNCTION_ID
#define FUNCTION_ID odqtodqInitialize
/*********************************************************************/
/*                                                                   */
/* Function: odqInitialize                                           */
/*                                                                   */
/* Description: Perform second stage initialization                  */
/*                                                                   */
/* Intended Function: This function performs queue manager related   */
/*                    initialization which could not be performed    */
/*                    during PreInitialization as the queue  manager */
/*                    name was not known at that time.               */
/*                                                                   */
/* Input Parameters: Queue Manager Name                              */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
odqResp odqInitialize(const char * QMgrName)
{
 odqResp rc = odq_Ok ;
 odq_fnc_entry( FUNCTION_ID );
 /* currently no initialization required */
 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return rc;
}

/*********************************************************************/
/*                                                                   */
/* Function: odqTerminate                                            */
/*                                                                   */
/* Description: Perform termination                                  */
/*                                                                   */
/* Intended Function: This function performs queue manager related   */
/*                    temination.                                    */
/*                                                                   */
/* Input Parameters: none                                            */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
void odqTerminate(void)
{
  /* currently no termination required */
  return;
}

#undef FUNCTION_ID
#define FUNCTION_ID odqtodqFFDC
/*********************************************************************/
/*                                                                   */
/* Function: odqFFDC                                                 */
/*                                                                   */
/* Description: Perform first failure data capture                   */
/*                                                                   */
/* Intended Function: This function provides first failure data      */
/*                    capture facilities for the dead letter queue   */
/*                    handler.                                       */
/*                                                                   */
/* Input Parameters: function in which the error occured             */
/*                   error identifier                                */
/*                   numeric code 1                                  */
/*                   numeric code 2                                  */
/*                   variable number of triplets of the form         */
/*                       address                                     */
/*                       length                                      */
/*                       remark                                      */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
void odqFFDC( /*IN*/
                const int function,
                const odqResp  MessageId,
                const MQLONG SubCode1,
                const MQLONG SubCode2,
                ...
              /*OUT*/
            )
{


int        argCount=0;
va_list    listp;
void *     argp;
int        argl;
char       *argcomment;
char       *inserts[3]={0};
time_t     now ;
struct tm  *ptime;
char       date[24] ;

#ifdef AMQ_UNIX
struct tm  timebuffer;
#endif

odq_fnc_entry( FUNCTION_ID );

/*--------------------------------------------------------------------*/
/* Build the date/time stamp for the message.                         */
/*--------------------------------------------------------------------*/
time (&now) ;
#ifdef AMQ_UNIX
ptime = gmtime_r( &now, &timebuffer );
#else
ptime = gmtime( &now ) ;
#endif
strftime( date, sizeof(date), "%d/%m/%y %H:%M:%S", ptime ) ;


/*--------------------------------------------------------------------*/
/* Convert the variable length plist into an array of inserts.        */
/*--------------------------------------------------------------------*/
va_start(listp, SubCode2);
for( argp = va_arg(listp, void *),
     argl = va_arg(listp, int ) ,
     argcomment = va_arg(listp, char * ) ;
     argp != NULL;
     argCount ++
   )
{
  inserts[argCount] = argp ;
  argp = va_arg(listp, void *);
  if ( NULL != argp )
  {
    argl = va_arg(listp, int ) ;
    argcomment = va_arg(listp, char * ) ;
  }
}
va_end(listp);

/*--------------------------------------------------------------------*/
/* If messages are initialized then write the appropriate message.    */
/*--------------------------------------------------------------------*/
if ( 0 != odq_msg )
{
  /*------------------------------------------------------------------*/
  /* Locate the message via the index.                                */
  /*------------------------------------------------------------------*/
  if( 0 != fsetpos( MsgFile, MsgIndex+MessageId ))
  {
   fprintf( stderr, "%s odq-000: Message odq-%d not found\n",
            date, MessageId );
  }
  else
  {
   /*-----------------------------------------------------------------*/
   /* Look for the strings &1, &2,&3,&4,&5 which need to be replaced  */
   /* by the relevant inserts.                                        */
   /* And write the message to stderr.                                */
   /*-----------------------------------------------------------------*/
   char c;
   fprintf( stderr,
            "%s ",
            date);

   for( c = fgetc(MsgFile)  ;
       c != '\n' ;
       c = fgetc( MsgFile) )
   {
     if ( '&' == c )
     {
       c = fgetc(MsgFile) ;
       switch( c )
       {
         case '1': fprintf(stderr,"%d", SubCode1 );
           break;
         case '2': fprintf(stderr,"%d", SubCode2 );
           break;
         case '3': fprintf(stderr,"%s", inserts[0]?inserts[0]:"null" );
           break;
         case '4': fprintf(stderr,"%s", inserts[1]?inserts[1]:"null" );
           break;
         case '5': fprintf(stderr,"%s", inserts[2]?inserts[2]:"null" );
           break;
         default:  fprintf(stderr,"MESSAGE_TEMPLATE_ERROR");
           break;
       }
     }
     else if( EOF == c )
     {
       fprintf(stderr,"MESSAGE_TEMPLATE_ERROR");
       break;
     }
     else
     {
       fputc( c, stderr ) ;
     }
   }
   fputc( '\n', stderr ) ;
  }
}

/*--------------------------------------------------------------------*/
/* If this is a severe error then dump the inserts to stderr in       */
/* both hex and text format.                                          */
/*--------------------------------------------------------------------*/
if( odq_Disaster <= MessageId)
{
  char *p;
  int   i ;
  va_start(listp, SubCode2);
  for( argp = va_arg(listp, void *),
       argl = va_arg(listp, int ) ,
       argcomment = va_arg(listp, char * ) ;
       argp != NULL;
     )
  {
    fputs(argcomment, stderr) ;
    fputc('\n',stderr);
    for( p = argp;
         p < (char *)argp + argl -16 ;
         p += 16 )
    {
        for( i  = 0 ;
             i < 16 ;
             i ++  )
        {
          fprintf( stderr, "%02x", *(p+i) ) ;
          if( 3 == i%4)
          {
           fputc( ' ',stderr ) ;
          }
        }
        fputc('<', stderr) ;
        for( i  = 0 ;
             i < 16 ;
             i ++  )
        {
          if( isalnum(p[i])| ispunct(p[i]) )
          {
            fputc( *(p+i) ,stderr) ;
          }
          else
          {
            fputc( '.',stderr);
          }
        }
        fprintf(stderr,">\n") ;
    }
    if( argl%16 )
    {
     for( i  = 0 ;
          i < 16 ;
          i ++  )
     {
       if( argl%16 >= i )
       {
         fprintf( stderr, "%02x", *(p+i) ) ;
       }
       else
       {
        fputc( ' ',stderr ) ;
       }
       if( 3 == i%4)
       {
        fputc( ' ',stderr ) ;
       }
     }
     fputc('<', stderr) ;
     for( i  = 0 ;
          i < 16 ;
          i ++  )
     {
       if (argl%16 >= i )
       {
         if( isalnum(p[i])| ispunct(p[i]) )
         {
           fputc( *(p+i),stderr );
         }
         else
         {
           fputc( '.',stderr);
         }
       }
     }
     fprintf(stderr, ">\n" ) ;
    }
    fprintf(stderr, "%s\n", argcomment ) ;
    argp = va_arg(listp, void *);
    if ( NULL != argp )
    {
      argl = va_arg(listp, int ) ;
      argcomment = va_arg(listp, char * ) ;
    }
  }
  va_end(listp);
}


 odq_fnc_retcode( FUNCTION_ID, odq_Ok ) ;
}


#undef  FUNCTION_ID
#define FUNCTION_ID odqtodqGetMem
/*********************************************************************/
/*                                                                   */
/* Function: odqGetMem                                               */
/*                                                                   */
/* Description: obtain private storage                               */
/*                                                                   */
/* Intended Function: This function is called to obtain private      */
/*                    storage for use by the DLQ handler.            */
/*                                                                   */
/* Input Parameters: length                                          */
/*                   remark                                          */
/*                                                                   */
/* Output Parameters: address                                        */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
odqResp odqGetMem( /*IN*/
                     int length,
                     char * comment,
                   /*OUT*/
                     void **address
                 )

{
 odqResp rc=odq_Ok ;
/*-------------------------------------------------------------------*/
/* Obtain private storage for use by the dead letter queue handler.  */
/*                                                                   */
/* note: The 'comment' input field is not used by this routine but   */
/*       is provided to allow the callers to identify the            */
/*       corresponding odqFreMem.                                    */
/*-------------------------------------------------------------------*/
 odq_fnc_entry( FUNCTION_ID ) ;
 *address = malloc(length) ;
 if( NULL == *address )
 {
   odqFFDC( /*IN*/
              FUNCTION_ID,
              odq_NoStorage,
              0,
              0,
              comment,
              strlen(comment),
              "Remark",
              NULL
            /*OUT*/
          ) ;
   rc = odq_NoStorage ;
 }
 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return rc;
}
#undef  FUNCTION_ID
#define FUNCTION_ID odqtodqFreeMem
/*********************************************************************/
/*                                                                   */
/* Function: odqFreeMem                                              */
/*                                                                   */
/* Description: release private storage                              */
/*                                                                   */
/* Intended Function: This function is called to obtain private      */
/*                    storage for use by the DLQ handler.            */
/*                                                                   */
/* Input Parameters: remark                                          */
/*                   address                                         */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
odqResp odqFreeMem( /*IN*/
                      char * comment,
                      void * address
                    /*OUT*/
                  )

{
 odqResp rc=odq_Ok ;
/*-------------------------------------------------------------------*/
/* Release private storage used by the dead letter queue handler.    */
/*                                                                   */
/* note: The 'comment' input field is not used by this routine but   */
/*       is provided to allow the callers to identify the            */
/*       corresponding odqGetMem.                                    */
/*-------------------------------------------------------------------*/
 odq_fnc_entry( FUNCTION_ID ) ;
 free( address ) ;
 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return rc;
}


#undef  FUNCTION_ID
/*********************************************************************/
/*                                                                   */
/* Function: odq_fnc_entry                                           */
/*                                                                   */
/* Description: Trace entry to a function                            */
/*                                                                   */
/* Intended Function: This function makes a trace entry for entry    */
/*                    to a function.                                 */
/*                                                                   */
/* Input Parameters: function identifier                             */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
void odq_fnc_entry(/*IN*/
                      const odqTraceId function
                   /*OUT*/
                 )
{
  if( odq_trace )
  {
    if( indent )
    {
      fprintf( stderr, "%*s",  indent, "->");
    }
    fprintf(stderr,"%s\n", odq_function[function].name );
    indent += 2 ;
  }
}


/*********************************************************************/
/*                                                                   */
/* Function: odq_fnc_retcode                                         */
/*                                                                   */
/* Description: Trace exit from a function                           */
/*                                                                   */
/* Intended Function: This function makes a trace entry on exit      */
/*                    from a function.                               */
/*                                                                   */
/* Input Parameters: function identifier                             */
/*                   return code                                     */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
void odq_fnc_retcode(/*IN*/
                      const odqTraceId function,
                      const odqResp retcode
                     /*OUT*/
                    )
{
  if( odq_trace )
  {
    indent -= 2 ;
    if( indent )
    {
      fprintf( stderr, "%*s", indent,"<-");
    }
    fprintf(stderr,"%s rc=%d\n", odq_function[function].name,retcode );
  }
}

#define FUNCTION_ID odqtodqMQCONNX
/*********************************************************************/
/*                                                                   */
/* Function: odqMQCONNX                                              */
/*                                                                   */
/* Description: Connect to a queue manager.                          */
/*                                                                   */
/* Intended Function: This function connects to queue manager.       */
/*                                                                   */
/* Input Parameters: Queue manager name                              */
/*                   pCno                                            */
/*                                                                   */
/* Output Parameters: pCompCode                                      */
/*                    pReason                                        */
/*                                                                   */
/* Returns:                                                          */
/*                                                                   */
/*********************************************************************/
void odqMQCONNX(char * qm,
               PMQCNO pCno,
               PMQLONG pCompCode,
               PMQLONG pReason)
{
  odqResp rc = odq_Ok ;
  odq_fnc_entry( FUNCTION_ID );
  MQCONNX( qm,
           pCno,
           &odqGlobal.odqHConn,
           pCompCode,
           pReason );
  odq_fnc_retcode( FUNCTION_ID, rc ) ;
}
#undef FUNCTION_ID

#define FUNCTION_ID odqtodqMQBACK
/*********************************************************************/
/*                                                                   */
/* Function: odqMQBACK                                               */
/*                                                                   */
/* Description: Backs out all message puts/gets that occurred since  */
/* last sync                                                         */
/*                                                                   */
/* Intended Function: Indicates to the queue manager that all the    */
/* message gets and puts that have occurred since the last sync point*/
/* are to be backed out.                                             */
/*                                                                   */
/* Input Parameters: Hconn                                           */
/*                                                                   */
/* Output Parameters: pCompCode                                      */
/*                    pReason                                        */
/*                                                                   */
/* Returns:                                                          */
/*                                                                   */
/*********************************************************************/
void odqMQBACK (
   MQHCONN  Hconn,     /* I: Connection handle */
   PMQLONG  pCompCode, /* OC: Completion code */
   PMQLONG  pReason)   /* OR: Reason code qualifying CompCode */
{
  odqResp rc = odq_Ok ;
  odq_fnc_entry( FUNCTION_ID );
  MQBACK(Hconn,
         pCompCode,
         pReason);
  odq_fnc_retcode( FUNCTION_ID, rc ) ;
}
#undef FUNCTION_ID

#define FUNCTION_ID odqtodqMQCLOSE
/*********************************************************************/
/*                                                                   */
/* Function: odqMQCLOSE                                              */
/*                                                                   */
/* Description: Relinquishes access to an object.                    */
/*                                                                   */
/* Intended Function: This function will relinquishes access to the  */
/*                    object passed                                  */
/*                                                                   */
/* Input Parameters: Hconn                                           */
/*                   pHobj                                           */
/*                   Options                                         */
/* Output Parameters: pHobj                                          */
/*                    pCompCode                                      */
/*                    pReason                                        */
/*                                                                   */
/* Returns:                                                          */
/*                                                                   */
/*********************************************************************/
void odqMQCLOSE (
   MQHCONN  Hconn,      /* Connection handle */
   PMQHOBJ  pHobj,      /* Object handle */
   MQLONG   Options,    /* Options that control the action of MQCLOSE */
   PMQLONG  pCompCode,  /* Completion code */
   PMQLONG  pReason)    /* Reason code qualifying CompCode */
{
  odqResp rc = odq_Ok ;
  odq_fnc_entry( FUNCTION_ID );
  MQCLOSE(Hconn,
          pHobj,
          Options,
          pCompCode,
          pReason);
  odq_fnc_retcode( FUNCTION_ID, rc ) ;
}
#undef FUNCTION_ID

#define FUNCTION_ID odqtodqMQCMIT
/*********************************************************************/
/*                                                                   */
/* Function: odqMQCMIT                                               */
/*                                                                   */
/* Description: Commit changes.                                      */
/*                                                                   */
/* Intended Function: This function indicates to the queue manager   */
/*                    that the application has reached a sync point, */
/*                    and that all the message gets and puts that    */
/*                    have occurred since the last sync point are to */
/*                    be made permanent.                             */
/*                                                                   */
/* Input Parameters: Hconn                                           */
/*                                                                   */
/* Output Parameters: pCompCode                                      */
/*                    pReason                                        */
/*                                                                   */
/* Returns:                                                          */
/*                                                                   */
/*********************************************************************/
void odqMQCMIT (
   MQHCONN  Hconn,      /* I: Connection handle */
   PMQLONG  pCompCode,  /* OC: Completion code */
   PMQLONG  pReason)    /* OR: Reason code qualifying CompCode */
{
  odqResp rc = odq_Ok ;
  odq_fnc_entry( FUNCTION_ID );
  MQCMIT(Hconn,
         pCompCode,
         pReason);
  odq_fnc_retcode( FUNCTION_ID, rc ) ;
}
#undef FUNCTION_ID

#define FUNCTION_ID odqtodqMQCRTMH
/*********************************************************************/
/*                                                                   */
/* Function: odqMQCRTMH                                              */
/*                                                                   */
/* Description: Create message handle.                               */
/*                                                                   */
/* Intended Function: This function returns a message handle.        */
/*                                                                   */
/* Input Parameters: Hconn                                           */
/*                   pCrtMsgHOpts                                    */
/*                                                                   */
/*                                                                   */
/* Output Parameters: pHmsg                                          */
/*                    pCompCode                                      */
/*                    pReason                                        */
/*                                                                   */
/* Returns:                                                          */
/*                                                                   */
/*********************************************************************/
void odqMQCRTMH (
   MQHCONN   Hconn,         /* I: Connection handle */
   PMQVOID   pCrtMsgHOpts,  /* IO: Options that control the action of
                               MQCRTMH */
   PMQHMSG   pHmsg,         /* IO: Message handle */
   PMQLONG   pCompCode,     /* OC: Completion code */
   PMQLONG   pReason)       /* OR: Reason code qualifying CompCode */
{
  odqResp rc = odq_Ok ;
  odq_fnc_entry( FUNCTION_ID );
  MQCRTMH(Hconn,
          pCrtMsgHOpts,
          pHmsg,
          pCompCode,
          pReason);
  odq_fnc_retcode( FUNCTION_ID, rc ) ;
}
#undef FUNCTION_ID

#define FUNCTION_ID odqtodqMQDISC
/*********************************************************************/
/*                                                                   */
/* Function: odqMQDISC                                               */
/*                                                                   */
/* Description: Disconnect queue manager.                            */
/*                                                                   */
/* Intended Function: This function disconnects from the queue       */
/*                    manager.                                       */
/*                                                                   */
/* Input Parameters: pHconn                                          */
/*                                                                   */
/* Output Parameters: pHconn                                         */
/*                    pCompCode                                      */
/*                    pReason                                        */
/*                                                                   */
/* Returns:                                                          */
/*                                                                   */
/*********************************************************************/
void odqMQDISC (
   PMQHCONN  pHconn,     /* Connection handle */
   PMQLONG   pCompCode,  /* Completion code */
   PMQLONG   pReason)    /* Reason code qualifying CompCode */
{
  odqResp rc = odq_Ok ;
  odq_fnc_entry( FUNCTION_ID );
  MQDISC(pHconn,
         pCompCode,
         pReason);
  odq_fnc_retcode( FUNCTION_ID, rc ) ;
}
#undef FUNCTION_ID

#define FUNCTION_ID odqtodqMQDLTMH
/*********************************************************************/
/*                                                                   */
/* Function: odqMQDLTMH                                              */
/*                                                                   */
/* Description: Delete message handle.                               */
/*                                                                   */
/* Intended Function: This function Deletes the message handle.      */
/*                                                                   */
/* Input Parameters: Hconn                                           */
/*                   pHmsg                                           */
/*                   pDltMsgHOpts                                    */
/* Output Parameters: pHmsg                                          */
/*                    pCompCode                                      */
/*                    pReason                                        */
/*                                                                   */
/* Returns:                                                          */
/*                                                                   */
/*********************************************************************/
void odqMQDLTMH (
   MQHCONN   Hconn,         /* I: Connection handle */
   PMQHMSG   pHmsg,         /* IO: Message handle */
   PMQVOID   pDltMsgHOpts,  /* I: Options that control the action of
                               MQDLTMH */
   PMQLONG   pCompCode,     /* OC: Completion code */
   PMQLONG   pReason)       /* OR: Reason code qualifying CompCode */
{
  odqResp rc = odq_Ok ;
  odq_fnc_entry( FUNCTION_ID );
  MQDLTMH(Hconn,
          pHmsg,
          pDltMsgHOpts,
          pCompCode,
          pReason);
  odq_fnc_retcode( FUNCTION_ID, rc ) ;
}
#undef FUNCTION_ID

#define FUNCTION_ID odqtodqMQGET
/*********************************************************************/
/*                                                                   */
/* Function: odqMQGET                                                */
/*                                                                   */
/* Description: Get message.                                         */
/*                                                                   */
/* Intended Function: This function gets a message from a queue on   */
/*                    queue manager.                                 */
/*                                                                   */
/* Input Parameters: Hconn                                           */
/*                   Hobj                                            */
/*                   pMsgDesc                                        */
/*                   pGetMsgOpts                                     */
/*                   BufferLength                                    */
/*                                                                   */
/* Output Parameters: pGetMsgOpts                                    */
/*                    pBuffer                                        */
/*                    DataLength                                     */
/*                    pCompCode                                      */
/*                    pReason                                        */
/*                                                                   */
/* Returns:                                                          */
/*                                                                   */
/*********************************************************************/
void odqMQGET (
   MQHCONN  Hconn,         /* Connection handle */
   MQHOBJ   Hobj,          /* Object handle */
   PMQVOID  pMsgDesc,      /* Message descriptor */
   PMQVOID  pGetMsgOpts,   /* Options that control the action of
                              MQGET */
   MQLONG   BufferLength,  /* Length in bytes of the Buffer area */
   PMQVOID  pBuffer,       /* Area to contain the message data */
   PMQLONG  pDataLength,   /* Length of the message */
   PMQLONG  pCompCode,     /* Completion code */
   PMQLONG  pReason)       /* Reason code qualifying CompCode */
{
  odqResp rc = odq_Ok ;
  odq_fnc_entry( FUNCTION_ID );
  MQGET(Hconn,
        Hobj,
        pMsgDesc,
        pGetMsgOpts,
        BufferLength,
        pBuffer,
        pDataLength,
        pCompCode,
        pReason);
  odq_fnc_retcode( FUNCTION_ID, rc ) ;
}
#undef FUNCTION_ID

#define FUNCTION_ID odqtodqMQINQ
/*********************************************************************/
/*                                                                   */
/* Function: odqMQINQ                                                */
/*                                                                   */
/* Description: Inquire object attributes.                           */
/*                                                                   */
/* Intended Function: This function Inquires about the provided      */
/*                    objects attributes                             */
/*                                                                   */
/* Input Parameters: Hconn                                           */
/*                   Hobj                                            */
/*                   SelectorCount                                   */
/*                   pSelectors                                      */
/*                   IntAttrCount                                    */
/*                   pIntAttrs                                       */
/*                   CharAttrLength                                  */
/*                                                                   */
/* Output Parameters: pCharAttrs                                     */
/*                    pCompCode                                      */
/*                    pReason                                        */
/*                                                                   */
/* Returns:                                                          */
/*                                                                   */
/*********************************************************************/
void odqMQINQ (
   MQHCONN  Hconn,           /* Connection handle */
   MQHOBJ   Hobj,            /* Object handle */
   MQLONG   SelectorCount,   /* Count of selectors */
   PMQLONG  pSelectors,      /* Array of attribute selectors */
   MQLONG   IntAttrCount,    /* Count of integer attributes */
   PMQLONG  pIntAttrs,       /* Array of integer attributes */
   MQLONG   CharAttrLength,  /* Length of character attributes buffer */
   PMQCHAR  pCharAttrs,      /* Character attributes */
   PMQLONG  pCompCode,       /* Completion code */
   PMQLONG  pReason)         /* Reason code qualifying CompCode */
{
  odqResp rc = odq_Ok ;
  odq_fnc_entry( FUNCTION_ID );
  MQINQ(Hconn,
        Hobj,
        SelectorCount,
        pSelectors,
        IntAttrCount,
        pIntAttrs,
        CharAttrLength,
        pCharAttrs,
        pCompCode,
        pReason);
  odq_fnc_retcode( FUNCTION_ID, rc ) ;
}
#undef FUNCTION_ID

#define FUNCTION_ID odqtodqMQOPEN
/*********************************************************************/
/*                                                                   */
/* Function: odqMQOPEN                                               */
/*                                                                   */
/* Description: Establishes access to an object.                     */
/*                                                                   */
/* Intended Function: This function Establishes access to the object */
/*                    provided.                                      */
/*                                                                   */
/* Input Parameters: Hconn                                           */
/*                   pObjDesc                                        */
/*                   Options                                         */
/*                                                                   */
/* Output Parameters: pHobj                                          */
/*                    pObjDesc                                       */
/*                    pCompCode                                      */
/*                    pReason                                        */
/*                                                                   */
/* Returns:                                                          */
/*                                                                   */
/*********************************************************************/
void odqMQOPEN (
   MQHCONN  Hconn,      /* Connection handle */
   PMQVOID  pObjDesc,   /* Object descriptor */
   MQLONG   Options,    /* Options that control the action of MQOPEN */
   PMQHOBJ  pHobj,      /* Object handle */
   PMQLONG  pCompCode,  /* Completion code */
   PMQLONG  pReason)    /* Reason code qualifying CompCode */
{
  odqResp rc = odq_Ok ;
  odq_fnc_entry( FUNCTION_ID );
  MQOPEN(Hconn,
         pObjDesc,
         Options,
         pHobj,
         pCompCode,
         pReason);
  odq_fnc_retcode( FUNCTION_ID, rc ) ;
}
#undef FUNCTION_ID

#define FUNCTION_ID odqtodqMQPUT1
/*********************************************************************/
/*                                                                   */
/* Function: odqMQPUT1                                               */
/*                                                                   */
/* Description: Put one message.                                     */
/*                                                                   */
/* Intended Function: This function puts one message on a queue.     */
/*                                                                   */
/* Input Parameters: Hconn                                           */
/*                   pObjDesc                                        */
/*                   pMsgDesc                                        */
/*                   pPutMsgOpts                                     */
/*                   BufferLength                                    */
/*                   pBuffer                                         */
/*                                                                   */
/* Output Parameters: pObjDesc                                       */
/*                    pMsgDesc                                       */
/*                    pPutMsgOpts                                    */
/*                    pCompCode                                      */
/*                    pReason                                        */
/*                                                                   */
/* Returns:                                                          */
/*                                                                   */
/*********************************************************************/
void odqMQPUT1 (
   MQHCONN  Hconn,         /* Connection handle */
   PMQVOID  pObjDesc,      /* Object descriptor */
   PMQVOID  pMsgDesc,      /* Message descriptor */
   PMQVOID  pPutMsgOpts,   /* Options that control the action of
                              MQPUT1 */
   MQLONG   BufferLength,  /* Length of the message in Buffer */
   PMQVOID  pBuffer,       /* Message data */
   PMQLONG  pCompCode,     /* Completion code */
   PMQLONG  pReason)       /* Reason code qualifying CompCode */
{
  odqResp rc = odq_Ok ;
  odq_fnc_entry( FUNCTION_ID );
  MQPUT1(Hconn,
         pObjDesc,
         pMsgDesc,
         pPutMsgOpts,
         BufferLength,
         pBuffer,
         pCompCode,
         pReason);
  odq_fnc_retcode( FUNCTION_ID, rc ) ;
}
#undef FUNCTION_ID
