/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/amqsbcg0.c */
/**********************************************************************/
/*                                                                    */
/* Program name: AMQSBCG0                                             */
/*                                                                    */
/* Description : Sample program to read and output the message        */
/*               descriptor fields, any other message properties, any */
/*               Dead Letter Header, and the message content of all   */
/*               the messages on a queue                              */
/*                                                                    */
/*   <copyright                                                       */
/*   notice="lm-source-program"                                       */
/*   pids="5724-H72"                                                  */
/*   years="1994,2024"                                                */
/*   crc="2777991688" >                                               */
/*   Licensed Materials - Property of IBM                             */
/*                                                                    */
/*   5724-H72                                                         */
/*                                                                    */
/*   (C) Copyright IBM Corp. 1994, 2024 All Rights Reserved.          */
/*                                                                    */
/*   US Government Users Restricted Rights - Use, duplication or      */
/*   disclosure restricted by GSA ADP Schedule Contract with          */
/*   IBM Corp.                                                        */
/*   </copyright>                                                     */
/*                                                                    */
/* Function    : This program is passed the name of a queue manager   */
/*               and a queue. It then reads each message from the     */
/*               queue and outputs the following to the stdout        */
/*                    -  Formatted message descriptor fields          */
/*                    -  Optionally any other formatted message       */
/*                       properties                                   */
/*                    -  Dead Letter Header structure if included     */
/*                    -  Message data (dumped in hex and, where       */
/*                       possible, character format)                  */
/*                                                                    */
/* Parameters  : Queue Manager Name                                   */
/*               Queue Name                                           */
/*               Property Option                                      */
/*                                                                    */
/* Restriction : This program is currently restricted to printing     */
/*               the first 65536 characters of the message.           */
/*                                                                    */
/* Note:         To convert this program to read the messages         */
/*               destructively, rather than browsing, change          */
/*               GetMsgOpts and Open Options by commenting out        */
/*               two lines in the program. See lines marked @@@@.     */
/*                                                                    */
/*  Environment variable MQSAMP_USER_ID can be set to authenticate    */
/*  application. If it is set, a password must also be entered at     */
/*  the prompt.                                                       */
/*                                                                    */
/*  Environment variable MQSAMP_TOKEN can be set to authenticate      */
/*  application. If it is set, a token must be entered at the prompt. */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/*                     Program logic                                  */
/*                     -------------                                  */
/*                                                                    */
/*    main (Last function in the code)                                */
/*    ----                                                            */
/*        Initialize the variables                                    */
/*        If correct parameters not passed                            */
/*          Report the error to the user                              */
/*          Terminate the program with return code 4                  */
/*        End-if                                                      */
/*        Connect to the queue manager                                */
/*        If the connect fails                                        */
/*          Report the error to the user                              */
/*          Terminate the program                                     */
/*        End-if                                                      */
/*        Open the queue                                              */
/*        If the open fails                                           */
/*          Report the error to the user                              */
/*          Terminate the program                                     */
/*        End-if                                                      */
/*        If required, create a message handle                        */
/*          If creation fails                                         */
/*            Report the error to the user                            */
/*            Terminate the program                                   */
/*          End-if                                                    */
/*        End-if                                                      */
/*        While compcode is ok                                        */
/*          Reset call variables                                      */
/*          Get a message                                             */
/*          If compcode not ok                                        */
/*            If reason not no-msg-available                          */
/*              print error message                                   */
/*            Else                                                    */
/*              print no more messages                                */
/*            End-if                                                  */
/*          Else                                                      */
/*            Call printMD                                            */
/*            If required, call printProperties                       */
/*            If required, call printDLH                              */
/*            Print the message length                                */
/*            Print each group of 16 bytes of the message as follows: */
/*            -Offset into message (in hex)                           */
/*            -Message content in hex                                 */
/*            -Printable message content ('.' if not printable)       */
/*            Pad the last line of the message to maintain format     */
/*          End-if                                                    */
/*        End-while                                                   */
/*        If required, delete the message handle                      */
/*          If the deletion fails                                     */
/*            Report the error to the user                            */
/*          End-if                                                    */
/*        End-if                                                      */
/*        If required, close the queue                                */
/*          If the close fails                                        */
/*            Report the error to the user                            */
/*          End-if                                                    */
/*        End-if                                                      */
/*        If required, disconnect from the queue manager              */
/*          If the disconnect fails                                   */
/*            Report the error to the user                            */
/*          End-if                                                    */
/*        End-if                                                      */
/*        Return to calling program                                   */
/*                                                                    */
/*                                                                    */
/*    printMD                                                         */
/*    -------                                                         */
/*        For each field of the message descriptor                    */
/*         Print the field name and contents                          */
/*                                                                    */
/*    printProperties                                                 */
/*    ---------------                                                 */
/*        For each property of the message                            */
/*         Print the property name and value                          */
/*                                                                    */
/**********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <locale.h>
#include <cmqc.h>
#include <cmqstrc.h>

#ifdef SAMPLE_AUTH_ENABLED
 #include "./amqsauth.h"
#endif

#define    CHARS_PER_LINE  16  /* Used in formatting the message */
#define    BUFFERLENGTH  65536 /* Max length of message accepted */
#define    NAMELENGTH     256  /* Initial returned name buffer length */
#define    VALUELENGTH   32767 /* Initial property value length */

typedef enum                   /* Property options */
{
  PROPS_AS_Q_DEF = 0,
  PROPS_IN_MSG_HANDLE,
  PROPS_NONE,
  PROPS_IN_MQRFH2,
  PROPS_COMPATIBILITY,
  PROPS_LAST
} PropOptions;

#if MQAT_DEFAULT == MQAT_WINDOWS_NT /* printf 64-bit integer type */
  #define  Int64 "I64"
#elif defined(MQ_64_BIT)
  #define  Int64 "l"
#else
  #define  Int64 "ll"
#endif

/**********************************************************************/
/* Function name:   printMD                                           */
/*                                                                    */
/* Description:      Prints the name of each field in the message     */
/*                   descriptor together with its contents in the     */
/*                   appropriate format viz:                          */
/*                   integers as a number (%d)                        */
/*                   binary fields as a series of hex digits (%02X)   */
/*                   character fields as characters (%s)              */
/*                                                                    */
/* Called by:        main                                             */
/*                                                                    */
/* Receives:         pointer to message descriptor structure          */
/*                                                                    */
/* Calls:            nothing                                          */
/*                                                                    */
/**********************************************************************/
void printMD(MQMD *MDin)
{
   int i;

   printf("\n****Message descriptor****\n");
   printf("\n  StrucId  : '%.4s'", MDin->StrucId);
   printf("  Version : %d", MDin->Version);
   printf("\n  Report   : %d", MDin->Report);
   printf("  MsgType : %d", MDin->MsgType);
   printf("\n  Expiry   : %d", MDin->Expiry);
   printf("  Feedback : %d", MDin->Feedback);
   printf("\n  Encoding : %d", MDin->Encoding);
   printf("  CodedCharSetId : %d", MDin->CodedCharSetId);
   printf("\n  Format : '%.*s'", MQ_FORMAT_LENGTH, MDin->Format);
   printf("\n  Priority : %d", MDin->Priority);
   printf("  Persistence : %d", MDin->Persistence);
   printf("\n  MsgId : X'");

   for (i = 0 ; i < MQ_MSG_ID_LENGTH ; i++)
     printf("%02X",MDin->MsgId[i] );

   printf("'");
   printf("\n  CorrelId : X'");

   for (i = 0 ; i < MQ_CORREL_ID_LENGTH ; i++)
     printf("%02X",MDin->CorrelId[i] );

   printf("'");
   printf("\n  BackoutCount : %d", MDin->BackoutCount);
   printf("\n  ReplyToQ       : '%.*s'", MQ_Q_NAME_LENGTH,
          MDin->ReplyToQ);
   printf("\n  ReplyToQMgr    : '%.*s'", MQ_Q_MGR_NAME_LENGTH,
          MDin->ReplyToQMgr);
   printf("\n  ** Identity Context");
   printf("\n  UserIdentifier : '%.*s'", MQ_USER_ID_LENGTH,
          MDin->UserIdentifier);
   printf("\n  AccountingToken : \n   X'");

   for (i = 0 ; i < MQ_ACCOUNTING_TOKEN_LENGTH ; i++)
     printf("%02X",MDin->AccountingToken[i] );

   printf("'");
   printf("\n  ApplIdentityData : '%.*s'", MQ_APPL_IDENTITY_DATA_LENGTH,
          MDin->ApplIdentityData);
   printf("\n  ** Origin Context");
   printf("\n  PutApplType    : '%d'", MDin->PutApplType);
   printf("\n  PutApplName    : '%.*s'", MQ_PUT_APPL_NAME_LENGTH,
          MDin->PutApplName);
   printf("\n  PutDate  : '%.*s'", MQ_PUT_DATE_LENGTH, MDin->PutDate);
   printf("    PutTime  : '%.*s'", MQ_PUT_TIME_LENGTH, MDin->PutTime);
   printf("\n  ApplOriginData : '%.*s'\n", MQ_APPL_ORIGIN_DATA_LENGTH,
          MDin->ApplOriginData);
   printf("\n  GroupId : X'");

   for (i = 0 ; i < MQ_GROUP_ID_LENGTH ; i++)
     printf("%02X",MDin->GroupId[i] );

   printf("'");
   printf("\n  MsgSeqNumber   : '%d'", MDin->MsgSeqNumber);
   printf("\n  Offset         : '%d'", MDin->Offset);
   printf("\n  MsgFlags       : '%d'", MDin->MsgFlags);
   printf("\n  OriginalLength : '%d'", MDin->OriginalLength);
}  /* end printMD */


/**********************************************************************/
/* Function name:    printDLH                                         */
/*                                                                    */
/* Description:      Prints the name of each field in any dead letter */
/*                   structure together with its contents in the      */
/*                   appropriate format:                              */
/*                   integers as a number (%d)                        */
/*                   character fields as characters (%s)              */
/*                                                                    */
/* Called by:        main                                             */
/*                                                                    */
/* Receives:         pointer to message buffer                        */
/*                   length of received message data                  */
/*                                                                    */
/* Calls:            nothing                                          */
/* Returns:          length of DLH structure                          */
/*                                                                    */
/**********************************************************************/
int printDLH(PMQBYTE buf,MQLONG DataLength)
{
   PMQDLH dlh = (PMQDLH)buf;
   char *reasonStr;

   /* Check that we have got a plausible DLH */
   if (DataLength < MQDLH_LENGTH_1 || strncmp(dlh->StrucId,MQDLH_STRUC_ID,4) != 0)
   {
     return 0;
   }

   printf("\n");
   printf("\n****Dead Letter Header****\n");
   printf("\n  StrucId  : '%.4s'", dlh->StrucId);
   printf("  Version : %d", dlh->Version);

   reasonStr = MQRC_STR(dlh->Reason);
   if (reasonStr[0] == 0)
   {
     reasonStr = "UNKNOWN RC";
   }
   printf("\n  Reason   : %d [%s]", dlh->Reason,reasonStr);

   printf("\n  DestQName      : '%.*s'", MQ_Q_NAME_LENGTH,
          dlh->DestQName);
   printf("\n  DestQMgrName   : '%.*s'", MQ_Q_MGR_NAME_LENGTH,
          dlh->DestQMgrName);
   printf("\n  Encoding : %d", dlh->Encoding);
   printf("  CodedCharSetId : %d", dlh->CodedCharSetId);
   printf("\n  Format : '%.*s'", MQ_FORMAT_LENGTH, dlh->Format);
   printf("\n  PutApplType    : '%d'", dlh->PutApplType);
   printf("\n  PutApplName    : '%.*s'", MQ_PUT_APPL_NAME_LENGTH,
          dlh->PutApplName);
   printf("\n  PutDate  : '%.*s'", MQ_PUT_DATE_LENGTH, dlh->PutDate);
   printf("    PutTime  : '%.*s'", MQ_PUT_TIME_LENGTH, dlh->PutTime);

   return MQDLH_LENGTH_1; /* Only version to date */
}  /* end printDLH */

/**********************************************************************/
/* Function name:    printProperties                                  */
/*                                                                    */
/* Description:      Prints the name of each non-message descriptor   */
/*                   property together with it's value in the         */
/*                   appropriate format viz:                          */
/*                   boolean values as TRUE or FALSE                  */
/*                   byte string values as a series of hex digits     */
/*                   floating-point values as a number (%g)           */
/*                   integer values as a number (%d)                  */
/*                   null values as NULL                              */
/*                   string values as characters (%s)                 */
/*                                                                    */
/* Called by:        main                                             */
/*                                                                    */
/* Receives:         Connection handle                                */
/*                   Message handle                                   */
/*                                                                    */
/* Calls:            None                                             */
/*                                                                    */
/**********************************************************************/
void printProperties(MQHCONN Hconn, MQHMSG Hmsg)
{
  /*                                                                  */
  /* variable declaration and initialisation                          */
  /*                                                                  */
  int     i;                              /* loop counter             */
  int     j;                              /* another loop counter     */
  MQIMPO  InqPropOpts = {MQIMPO_DEFAULT}; /* inquire prop options     */
  MQLONG  NameLength = NAMELENGTH;     /* returned name buffer length */
  PMQCHAR NameBuffer = NULL;              /* returned name buffer     */
  MQCHARV InqName = {MQPROP_INQUIRE_ALL}; /* browse all properties    */
  MQPD    PropDesc = {MQPD_DEFAULT};      /* property descriptor      */
  MQLONG  Type;                           /* property type            */
  MQLONG  ValueLength = VALUELENGTH;      /* value buffer length      */
  PMQBYTE Value = NULL;                   /* value buffer             */
  MQLONG  PropsLength;                    /* length of property value */
  MQLONG  CompCode = MQCC_OK;             /* MQINQMP completion code  */
  MQLONG  Reason = MQRC_NONE;             /* MQINQMP reason code      */

  /*                                       */
  /* Initialise storage                    */
  /*                                       */
  Value = (PMQBYTE)malloc(ValueLength);
  NameBuffer = (PMQCHAR)malloc(NameLength);

  /*                                       */
  /* Initialise the inquire prop options   */
  /*                                       */
  InqPropOpts.Options |= MQIMPO_CONVERT_VALUE;
  InqPropOpts.ReturnedName.VSPtr     = NameBuffer;
  InqPropOpts.ReturnedName.VSBufSize = NameLength;

  /*                                       */
  /* then dump the message properties      */
  /*                                       */
  printf("\n ");
  printf("\n****Message properties****\n");

  /*                                       */
  /* Loop until MQINQMP unsuccessful       */
  /*                                       */
  for (i = 0; CompCode == MQCC_OK; i++)
  {
    MQINQMP(Hconn,
            Hmsg,
            &InqPropOpts,
            &InqName,
            &PropDesc,
            &Type,
            ValueLength,
            Value,
            &PropsLength,
            &CompCode,
            &Reason);

    /*                                     */
    /* Check for success                   */
    /*                                     */
    if (CompCode != MQCC_OK)
    {
      switch(Reason)
      {
        case MQRC_PROPERTY_NOT_AVAILABLE:
          /*                               */
          /* This message contains no more */
          /* properties ....               */
          /*                               */
          if (i == 0)
          {
            /*                             */
            /* In fact there were no       */
            /* properties at all           */
            /*                             */
            printf("\n  None\n");
          }
          break;

        case MQRC_PROPERTY_VALUE_TOO_BIG:
          /*                               */
          /* The Value buffer is too       */
          /* small - inquire the same prop */
          /* again, but with a bigger      */
          /* value buffer                  */
          /*                               */
          if (PropsLength <= ValueLength)
          {
            /*                             */
            /* We expect to be allocating  */
            /* a bigger buffer than before */
            /*                             */
            Reason = MQRC_STORAGE_NOT_AVAILABLE;
            printf("\n Unable to allocate property value buffer");
          }
          else
          {
            ValueLength = PropsLength;
            free(Value);
            Value = (PMQBYTE)malloc(ValueLength);
            if (Value)
            {
              CompCode = MQCC_OK;
              InqPropOpts.Options = MQIMPO_CONVERT_VALUE | MQIMPO_INQ_PROP_UNDER_CURSOR;
            }
            else
            {
              Reason = MQRC_STORAGE_NOT_AVAILABLE;
              printf("\n Unable to allocate property value buffer");
            }
          }
          break;

        case MQRC_PROPERTY_NAME_TOO_BIG:
          /*                               */
          /* The returned name buffer is   */
          /* too small - inquire the same  */
          /* prop again, but with a bigger */
          /* returned name buffer          */
          /*                               */
          NameLength = InqPropOpts.ReturnedName.VSLength;
          free(NameBuffer);
          NameBuffer = (PMQCHAR)malloc(NameLength);
          if (NameBuffer)
          {
            CompCode = MQCC_OK;
            InqPropOpts.ReturnedName.VSPtr = NameBuffer;
            InqPropOpts.ReturnedName.VSBufSize = NameLength;
            InqPropOpts.Options = MQIMPO_CONVERT_VALUE | MQIMPO_INQ_PROP_UNDER_CURSOR;
          }
          else
          {
            Reason = MQRC_STORAGE_NOT_AVAILABLE;
            printf("\n Unable to allocate property name buffer");
          }
          break;

        default:
          /*                               */
          /* MQINQMP failed for some other */
          /* reason                        */
          /*                               */
          printf("\n MQINQMP failed with CompCode:%d Reason:%d",
                 CompCode,Reason);
          break;
      }
    }
    else
    {
      /*                                   */
      /* MQINQMP succeeded.                */
      /*                                   */

      /*                                   */
      /* Print the property name           */
      /*                                   */
      printf("\n  %.*s : ",
             InqPropOpts.ReturnedName.VSLength,
             (char*)InqPropOpts.ReturnedName.VSPtr);

      /*                                   */
      /* Print the property value          */
      /*                                   */
      switch (Type)
      {
        /* Boolean value */
        case MQTYPE_BOOLEAN:
          printf("%s", *(PMQBOOL)Value ? "TRUE" : "FALSE");
          break;

        /* Byte-string value */
        case MQTYPE_BYTE_STRING:
          printf("X'");
          for (j = 0 ; j < PropsLength ; j++)
            printf("%02X",Value[j] );
          printf("'");
          break;

        /* 32-bit floating-point number value */
        case MQTYPE_FLOAT32:
          printf("%.12g", *(PMQFLOAT32)Value);
          break;

        /* 64-bit floating-point number value */
        case MQTYPE_FLOAT64:
          printf("%.18g", *(PMQFLOAT64)Value);
          break;

        /* 8-bit integer value */
        case MQTYPE_INT8:
          printf("%u", Value[0]);
          break;

        /* 16-bit integer value */
        case MQTYPE_INT16:
          printf("%hd", *(PMQINT16)Value);
          break;

        /* 32-bit integer value */
        case MQTYPE_INT32:
          printf("%d", *(PMQLONG)Value);
          break;

        /* 64-bit integer value */
        case MQTYPE_INT64:
          printf("%"Int64"d", *(PMQINT64)Value);
          break;

        /* Null value */
        case MQTYPE_NULL:
          printf("NULL");
          break;

        /* String value */
        case MQTYPE_STRING:
          printf("'%.*s'", PropsLength, Value);
          break;

        /* A value with an unrecognized type */
        default:
          printf("<unrecognized data type>\n");
          break;
      }

      /*                                   */
      /* Inquire on the next property      */
      /*                                   */
      InqPropOpts.Options = MQIMPO_CONVERT_VALUE | MQIMPO_INQ_NEXT;
    }
  }

  free(Value);
  free(NameBuffer);

  return;
}


/**********************************************************************/
/* Function name:    main                                             */
/*                                                                    */
/* Description:      Connects to the queue manager, opens the queue,  */
/*                   then gets each message from the queue in a loop  */
/*                   until an error occurs. The message descriptor,   */
/*                   any other messagse properties and the message    */
/*                   content are output to stdout for each message.   */
/*                   Any errors are output to stdout and the program  */
/*                   terminates.                                      */
/*                                                                    */
/* Receives:         Three parameters - queue manager name            */
/*                                    - queue name                    */
/*                                    - property option               */
/*                                                                    */
/* Calls:            printMD                                          */
/*                   printDLH                                         */
/*                   printProperties                                  */
/*                                                                    */
/**********************************************************************/
int  main(int argc, char *argv[] )
{
  /*                                                                  */
  /* variable declaration and initialisation                          */
  /*                                                                  */
  int i = 0;       /* loop counter                                    */
  int j = 0;       /* another loop counter                            */

  /* variables for MQCONNX           ******/
  MQCHAR  QMgrName[MQ_Q_MGR_NAME_LENGTH];
  MQHCONN Hconn = MQHC_UNUSABLE_HCONN;
  MQLONG  CompCode = MQCC_OK, Reason = MQRC_NONE;
  MQLONG  ShutdownCompCode, ShutdownReason;
  MQCNO   cno = {MQCNO_DEFAULT};   /* connection options            */
  MQCSP   csp = {MQCSP_DEFAULT};   /* security parameters           */

  /* variables for MQOPEN            ******/
  MQCHAR  Queue[MQ_Q_NAME_LENGTH + 1];
  MQOD    ObjDesc = { MQOD_DEFAULT };
  MQLONG  OpenOptions;
  MQHOBJ  Hobj = MQHO_UNUSABLE_HOBJ;

  /* variables for message properties *****/
  int     PropOption = PROPS_AS_Q_DEF;
  MQCMHO  CrtMsgHOpts = { MQCMHO_DEFAULT };
  MQDMHO  DltMsgHOpts = { MQDMHO_DEFAULT };
  MQHMSG  Hmsg = MQHM_UNUSABLE_HMSG;

  /* variables for MQGET             ******/
  MQMD    MsgDesc = { MQMD_DEFAULT };
  PMQMD   pmdin ;
  MQGMO   GetMsgOpts = { MQGMO_DEFAULT };
  PMQGMO  pgmoin;
  PMQBYTE Buffer;
  MQLONG  BufferLength = BUFFERLENGTH;
  MQLONG  EffectiveBufferLength; /* After any headers have been processed */
  MQLONG  DataLength;
  MQLONG  LengthToPrint;

  /* variables for summary counts and average */
  MQULONG  MsgCount = 0;
  MQULONG  LargeMsgLen, SmallMsgLen;
  MQUINT64 TotalMsgLen = 0;

  /* variables for message formatting *****/
  int  ch;
  int  overrun;  /* used on MBCS characters */
  int  mbcsmax;  /* used for MBCS characters */
  int  char_len;  /* used for MBCS characters */
  char line_text[CHARS_PER_LINE + 4]; /* allows for up to 3 MBCS bytes overrun */
  int  chars_this_line = 0;
  int  lines_printed   = 0;
  int  page_number     = 1;
  int  hdrOffset       = 0; /* Set if there is a DLH on the message */

  /*                                       */
  /* Use a version 2 MQMD in case the      */
  /* message is Segmented/Grouped          */
  /*                                       */
  MsgDesc.Version = MQMD_VERSION_2 ;

  /*                                       */
  /* Initialise storage ....               */
  /*                                       */
  pmdin  = (PMQMD)malloc(sizeof(MQMD));
  pgmoin = (PMQGMO)malloc(sizeof(MQGMO));
  Buffer = (PMQBYTE)malloc(BUFFERLENGTH);

  /*                                       */
  /* determine locale for MBCS handling    */
  /*                                       */
  setlocale(LC_ALL,"");  /* for mbcs charactersets */
  mbcsmax = MB_CUR_MAX;  /* for mbcs charactersets */

#ifndef SAMPLE_AUTH_ENABLED
   if (getenv("MQSAMP_USER_ID") != NULL || getenv("MQSAMP_TOKEN") != NULL)
     printf("Authentication not enabled, if you wish to enable:\n"
            " - Build this sample with amqsauth.c\n"
            " - Define the 'SAMPLE_AUTH_ENABLED' compile flag in your compile command\n");
#endif

  /*                                       */
  /* Handle the arguments passed           */
  /*                                       */
  printf("\nAMQSBCG0 - starts here\n");
  printf(  "**********************\n ");

  if (argc < 2)
  {
    printf("Required parameter missing - queue name\n");
    printf("\n  Usage: %s QName [ QMgrName ] [ PropOption ]\n",argv[0]);
    CompCode = 4;
    goto MOD_EXIT;
  }

  if (argc > 3)
  {
    PropOption = (MQLONG)atoi(argv[3]);
    if ( (PropOption < PROPS_AS_Q_DEF) ||
         (PropOption >= PROPS_LAST)    )
    {
      printf("PropOption \"%d\" invalid - specify %d-%d\n",
             PropOption, PROPS_AS_Q_DEF, PROPS_LAST-1);
      printf("\n  Usage: %s QName [ QMgrName ] [ PropOption ]\n",argv[0]);
      CompCode = 4;
      goto MOD_EXIT;
    }
  }
#ifdef SAMPLE_AUTH_ENABLED
   /******************************************************************/
   /* Setup any authentication information supplied in the local     */
   /* environment. The connection options structure points to the    */
   /* security structure. If the userid is set, then the password    */
   /* is read from the terminal. Having the password entered this    */
   /* way avoids it being accessible from other programs that can    */
   /* inspect command line parameters or environment variables.      */
   /******************************************************************/
   getAuthInfo(&cno, &csp);
#endif

  /******************************************************************/
  /*                                                                */
  /*   Connect to queue manager                                     */
  /*                                                                */
  /******************************************************************/
  QMgrName[0] =  '\0';                  /* set to null   default QM */
  if (argc > 2)
    strncpy(QMgrName, argv[2], MQ_Q_MGR_NAME_LENGTH);

  strncpy(Queue,argv[1],MQ_Q_NAME_LENGTH);
  Queue[MQ_Q_NAME_LENGTH] = 0;

  /*                                       */
  /* Start function here....               */
  /*                                       */
  MQCONNX(QMgrName,
         &cno,
         &Hconn,
         &CompCode,
         &Reason);

  if (CompCode != MQCC_OK)
  {
    printf("\n MQCONNX failed with CompCode:%d, Reason:%d",
           CompCode,Reason);
    goto MOD_EXIT;
  }

  /*                                        */
  /* Set the options for the open call      */
  /*                                        */

  OpenOptions = MQOO_BROWSE;

  /*    @@@@ Use this for destructive read    */
  /*         instead of the above.            */
  /* OpenOptions = MQOO_INPUT_SHARED;         */
  /*                                          */

  strncpy(ObjDesc.ObjectName, Queue, MQ_Q_NAME_LENGTH);

  printf("\n MQOPEN - '%s'", Queue);
  MQOPEN(Hconn,
         &ObjDesc,
         OpenOptions,
         &Hobj,
         &CompCode,
         &Reason);

  if (CompCode != MQCC_OK)
  {
    printf("\n MQOPEN failed with CompCode:%d, Reason:%d",
           CompCode,Reason);
    goto MOD_EXIT;
  }

  if(PropOption == PROPS_IN_MSG_HANDLE)
  {
    printf("\n MQCRTMH");

    MQCRTMH(Hconn,
            &CrtMsgHOpts,
            &Hmsg,
            &CompCode,
            &Reason);

    if (CompCode != MQCC_OK)
    {
      printf("\n  failed with CompCode:%d, Reason:%d",
             CompCode,Reason);
      goto MOD_EXIT;
    }

    GetMsgOpts.MsgHandle = Hmsg;

    /* Set the version number for the Get Message Options with */
    /* properties */
    GetMsgOpts.Version = MQGMO_VERSION_4;
  }
  else
  {
    /* Set the version number for the Get Message Options */
    GetMsgOpts.Version = MQGMO_VERSION_2;
  }

  printf("\n ");

  /* Avoid need to reset Message ID and Correlation ID after */
  /* every MQGET                                             */
  GetMsgOpts.MatchOptions = MQMO_NONE;

  /* Set the options for the get calls         */
  GetMsgOpts.Options = MQGMO_NO_WAIT ;

  /* Accept but truncate the data received for messages      */
  /* larger than the 64K buffer                              */
  GetMsgOpts.Options += MQGMO_ACCEPT_TRUNCATED_MSG ;

  /* @@@@ Comment out the next line for          */
  /*      destructive read                       */

  GetMsgOpts.Options += MQGMO_BROWSE_NEXT ;

  if(PropOption == PROPS_IN_MSG_HANDLE)
  {
    /* If specified, request that the non-message descriptor */
    /* properties are returned in the message handle */
    GetMsgOpts.Options += MQGMO_PROPERTIES_IN_HANDLE;
  }
  else if(PropOption == PROPS_IN_MQRFH2)
  {
    /* If specified, request that the non-message descriptor */
    /* properties are returned in an MQRFH2 header */
    GetMsgOpts.Options += MQGMO_PROPERTIES_FORCE_MQRFH2;
  }
  else if(PropOption == PROPS_NONE)
  {
    /* If specified, request that only message descriptor */
    /* properties are returned */
    GetMsgOpts.Options += MQGMO_NO_PROPERTIES;
  }
  else if(PropOption == PROPS_COMPATIBILITY)
  {
    /* If specified, request that the non-message descriptor */
    /* properties are returned in an MQRFH2 header, but only */
    /* if the message contains MQ v6 compatible properties.  */
    GetMsgOpts.Options += MQGMO_PROPERTIES_COMPATIBILITY;
  }

  /* Set the message descriptor and get message */
  /* options to the defaults                     */
  memcpy(pmdin, &MsgDesc, sizeof(MQMD) );
  memcpy(pgmoin, &GetMsgOpts, sizeof(MQGMO) );

  /*                                           */
  /* Loop until MQGET unsuccessful             */
  /*                                           */
  for (j = 1; CompCode != MQCC_FAILED; j++)
  {
     /*                                               */
     /* Set up the output format of the report        */
     /*                                               */
     if (page_number == 1)
     {
       lines_printed = 29;
       page_number = -1;
     }
     else
     {
       printf("\n ");
       lines_printed = 22;
     }

     /* Initialize the buffer to blanks               */
     memset(Buffer,' ',BUFFERLENGTH);

     MQGET(Hconn,
           Hobj,
           pmdin,
           pgmoin,
           BufferLength,
           Buffer,
           &DataLength,
           &CompCode,
           &Reason);

     if  (CompCode == MQCC_FAILED)
     {
       if (Reason != MQRC_NO_MSG_AVAILABLE)
       {
         printf("\n MQGET %d, failed with CompCode:%d Reason:%d",
                j,CompCode,Reason);
       }
       else
       {
         printf("\n \n \n No more messages ");
         CompCode = MQCC_OK;
         Reason   = MQRC_NONE;
         break;
       }
     }
     else
     {
       /* Print the message             */
       /*                               */
       printf("\n ");
       printf("\n MQGET of message number %d, CompCode:%d Reason:%d",
              j, CompCode, Reason);

       /* Update count of messages and lengths                        */
       /* Note that message length represents the converted message   */
       /* payload and this includes the length of any chained headers */
       /* that are returned in the payload such as a MQDLH. Message   */
       /* properties are included in the length if they are requested */
       /* to be returned in an RFH2 header.                           */
       MsgCount++;
       if (MsgCount == 1)
       {
         SmallMsgLen = (MQULONG)DataLength;
         LargeMsgLen = (MQULONG)DataLength;
       }
       else
       {
         if ((MQULONG)DataLength < SmallMsgLen)
           SmallMsgLen = (MQULONG)DataLength;
         if ((MQULONG)DataLength > LargeMsgLen)
           LargeMsgLen = (MQULONG)DataLength;
       }
       TotalMsgLen += DataLength;

       /*                               */
       /* first the Message Descriptor  */
       printMD(pmdin);

       /*                               */
       /* next any other properties     */
       if (PropOption == PROPS_IN_MSG_HANDLE)
       {
         printProperties(Hconn, Hmsg);
       }

       /* If the message contains a DLH, then print it             */
       /* Adjust the size of the remaining data to match.          */
       hdrOffset = 0;
       if (!strncmp(pmdin->Format,MQFMT_DEAD_LETTER_HEADER,MQ_FORMAT_LENGTH))
       {
         hdrOffset = printDLH(Buffer,DataLength);
         DataLength -= hdrOffset;
       }

       /*                               */
       /* then dump the Message         */
       /*                               */
       printf("\n ");
       printf("\n****   Message      ****\n ");

       /* If the messages has been truncated ensure we only print */
       /* out as much data was actually returned.                 */
       EffectiveBufferLength = BufferLength - hdrOffset;
       LengthToPrint = (DataLength < EffectiveBufferLength?DataLength:EffectiveBufferLength);

       printf("\n length - %d of %d bytes\n ", LengthToPrint, DataLength);
       ch = 0;
       overrun = 0;
       do
       {
         chars_this_line = 0;
         printf("\n%08X: ",ch);
         for (;overrun>0; overrun--) /* for MBCS overruns */
         {
           printf("  ");            /* dummy space for characters  */
           line_text[chars_this_line] = ' ';
                                /* included in previous line */
           chars_this_line++;
           if (overrun % 2)
             printf(" ");
         }
         while ( (chars_this_line < CHARS_PER_LINE) &&
                 (ch < LengthToPrint) )
         {
           char_len = mblen((char *)&Buffer[ch+hdrOffset],mbcsmax);
           if (char_len < 1)   /* badly formed mbcs character */
             char_len = 1;     /* or NULL treated as sbcs     */
           if (char_len > 1 )
           { /* mbcs case, assumes mbcs are all printable */
             for (;char_len >0;char_len--)
             {
               if ((chars_this_line % 2 == 0) &&
                   (chars_this_line < CHARS_PER_LINE))
                 printf(" ");
               printf("%02X",Buffer[ch+hdrOffset] );
               line_text[chars_this_line] = Buffer[ch+hdrOffset];
               chars_this_line++;
               ch++;
             }
           }
           else
           {  /* sbcs case */
             if (chars_this_line % 2 == 0)
               printf(" ");
             printf("%02X",Buffer[ch+hdrOffset] );
             line_text[chars_this_line] =
                 isprint(Buffer[ch+hdrOffset]) ? Buffer[ch+hdrOffset] : '.';
             chars_this_line++;
             ch++;
           }
         }

         /* has an mbcs character overrun the usual end? */
         if (chars_this_line > CHARS_PER_LINE)
            overrun = chars_this_line - CHARS_PER_LINE;

         /* pad with blanks to format the last line correctly */
         if (chars_this_line < CHARS_PER_LINE)
         {
           for ( ;chars_this_line < CHARS_PER_LINE;
                chars_this_line++)
           {
             if (chars_this_line % 2 == 0) printf(" ");
             printf("  ");
             line_text[chars_this_line] = ' ';
           }
         }

         /* leave extra space between columns if MBCS characters possible */
         for (i=0;i < ((mbcsmax - overrun - 1) *2);i++)
         {
           printf(" "); /* prints space between hex representation and character */
         }

         line_text[chars_this_line] = '\0';
         printf(" '%s'",line_text);
         lines_printed += 1;
         if (lines_printed >= 60)
         {
           lines_printed = 0;
           printf("\n ");
         }
       }
       while (ch < LengthToPrint);

     } /* end of message received 'else' */

  } /* end of for loop */

MOD_EXIT:

  /* Display a summary of the messages browsed */
  if (MsgCount > 0)
  {
    printf("\n\n****   Summary      ****\n "
           "%lu messages browsed.\n Smallest message %lu bytes.\n "
           "Largest message %lu bytes.\n Average message %lu bytes.\n",
           MsgCount, SmallMsgLen, LargeMsgLen,
           (MQULONG)(TotalMsgLen / MsgCount));
  }

  if (Hmsg != MQHM_UNUSABLE_HMSG)
  {
    printf("\n MQDLTMH");
    MQDLTMH(Hconn,
            &Hmsg,
            &DltMsgHOpts,
            &ShutdownCompCode,
            &ShutdownReason);

    if (ShutdownCompCode != MQCC_OK)
    {
      printf("\n  failed with CompCode:%d, Reason:%d",
             ShutdownCompCode,ShutdownReason);
    }
  }

  if (Hobj != MQHO_UNUSABLE_HOBJ)
  {
    printf("\n MQCLOSE");
    MQCLOSE(Hconn,
            &Hobj,
            MQCO_NONE,
            &ShutdownCompCode,
            &ShutdownReason);

    if (ShutdownCompCode != MQCC_OK)
    {
      printf("\n  failed with CompCode:%d, Reason:%d",
             ShutdownCompCode,ShutdownReason);
    }
  }

  if (Hconn != MQHC_UNUSABLE_HCONN)
  {
    printf("\n MQDISC");
    MQDISC(&Hconn,
           &ShutdownCompCode,
           &ShutdownReason);

    if (ShutdownCompCode != MQCC_OK)
    {
      printf("\n  failed with CompCode:%d, Reason:%d",
             ShutdownCompCode,ShutdownReason);
    }
  }

  free(pmdin);
  free(pgmoin);
  free(Buffer);

  printf("\n");

  return(CompCode);
}