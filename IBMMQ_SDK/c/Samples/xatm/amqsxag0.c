static const char sccsid[]= "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/xatm/amqsxag0.pre";
/******************************************************************************/
/*                                                                            */
/* Program name: AMQSXAG                                                      */
/*                                                                            */
/* Description:  Sample C program for MQ coordinating XA-compliant database   */
/*               managers.                                                    */
/*                                                                            */
/* Note:         Useful information and diagrams about this sample are in the */
/*               Application Programming Guide.                               */
/*                                                                            */
/* <copyright                                                                 */
/* notice="lm-source-program"                                                 */
/* pids=""                                                                    */
/* years="1998,2009"                                                          */
/* crc="3749995384" >                                                         */
/* Licensed Materials - Property of IBM                                       */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* (C) Copyright IBM Corp. 1998, 2009 All Rights Reserved.                    */
/*                                                                            */
/* US Government Users Restricted Rights - Use, duplication or                */
/* disclosure restricted by GSA ADP Schedule Contract with                    */
/* IBM Corp.                                                                  */
/* </copyright>                                                               */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Function:                                                                  */
/*                                                                            */
/*    AMQSXAG is a sample C program for MQ to coordinate getting a message    */
/*    off a queue and updating two databases within an MQ unit of work.       */
/*    This sample calls functions that are in AMQSXAB0.SQC and AMQSXAF0.SQC,  */
/*    these contain the SQL to update the two databases and hence must be     */
/*    prepared with the appropriate database.                                 */
/*                                                                            */
/*    Note: Whilst the first MQBEGIN call will create a connection to the     */
/*          two databases listed in the QM.INI file, the active connection    */
/*          needs to be toggled between the two databases, hence the calls to */
/*          connect to one of the databases.                                  */
/*                                                                            */
/*    -- A message is read from a queue (under sync point), it must be in     */
/*       the form:                                                            */
/*                                                                            */
/*       UPDATE Balance change=nnn WHERE Account=nnn                          */
/*                                                                            */
/*       The sample AMQSPUT can be used to put the messages on the queue.     */
/*                                                                            */
/*    -- Information from the database is obtained and updated with the       */
/*       information in the message.                                          */
/*                                                                            */
/*    -- The new status of the database is printed.                           */
/*                                                                            */
/* Program logic:                                                             */
/*                                                                            */
/*    MQCONN connect to default queue manager (or optionally supplied name)   */
/*    MQOPEN open queue for input (using supplied parameter)                  */
/*    while no failures                                                       */
/*    .  MQBEGIN start a unit of work                                         */
/*    .  MQGET get next message from queue under sync point                   */
/*    .  get information from databases                                       */
/*    .  update information from databases                                    */
/*    .  MQCMIT commit changes                                                */
/*    .  print updated information                                            */
/*    .  (no message available counts as failure, and loop ends)              */
/*    MQCLOSE close queue                                                     */
/*    MQDISC disconnect from queue manager                                    */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* AMQSXAG has 2 parameters - the name of the message queue (required)        */
/*                          - the queue manager name (optional)               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <cmqc.h>                                /* MQI                       */

/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/
#ifndef OK
   #define OK        0                           /* define OK as zero         */
#endif
#ifndef NOT_OK
   #define NOT_OK    1                           /* define NOT_OK as one      */
#endif

#ifndef BOOL
   #define BOOL MQULONG
#endif
#ifndef TRUE
   #define TRUE  1
#endif
#ifndef FALSE
   #define FALSE 0
#endif

/******************************************************************************/
/* Function prototypes for SQL routines in AMQSXAB0.SQC and AMQSXAF0.SQC      */
/******************************************************************************/
MQLONG ConnectToMQBankDB(void);
MQLONG ConnectToMQFeeDB(void);
MQLONG DeclareMQBankDBCursor(void);
MQLONG DeclareMQFeeDBCursor(void);
MQLONG GetMQBankTBDetails(char *name, MQLONG account, MQLONG *balance,
                          MQLONG *transactions);
MQLONG GetMQFeeTBDetails(MQLONG account, MQLONG *feeDue, MQLONG *tranFee,
                         MQLONG *transactions);
MQLONG UpdateMQBankTBBalance(MQLONG balance, MQLONG transactions);
MQLONG UpdateMQFeeTBFeeDue(MQLONG feeDue, MQLONG transactions);


/******************************************************************************/
/*                                                                            */
/* Function: Main                                                             */
/* ==============                                                             */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Input Parameters:                                                          */
/* -----------------                                                          */
/* int argc                       - number of arguments supplied              */
/* char *argv[]                   - program arguments                         */
/*                                                                            */
/* Returns:                                                                   */
/* --------                                                                   */
/* OK                             - all is well                               */
/*                                                                            */
/******************************************************************************/
int main(int argc, char *argv[])                 /* ----- START OF MAIN ----- */
{
   /***************************************************************************/
   /* MQI structures                                                          */
   /***************************************************************************/
   MQOD od = {MQOD_DEFAULT};                     /* object descriptor         */
   MQMD md = {MQMD_DEFAULT};                     /* message descriptor        */
   MQGMO gmo = {MQGMO_DEFAULT};                  /* get message options       */
   MQBO bo = {MQBO_DEFAULT};                     /* begin options             */
   /***************************************************************************/
   /* MQI variables                                                           */
   /***************************************************************************/
   MQLONG rc=OK;                                 /* return code               */
   MQHCONN hCon;                                 /* handle to connection      */
   MQHOBJ hObj;                                  /* handle to object          */
   MQCHAR QMgrName[MQ_Q_MGR_NAME_LENGTH+1]="";   /* queue manager name        */
   MQCHAR QName[MQ_Q_NAME_LENGTH+1]="";          /* queue name                */
   MQLONG options;                               /* options                   */
   MQLONG reason;                                /* reason code               */
   MQLONG connReason;                            /* MQCONN reason code        */
   MQLONG compCode;                              /* completion code           */
   MQLONG openCompCode;                          /* MQOPEN completion code    */
   char msgBuf[100];                             /* message buffer            */
   MQLONG msgBufLen;                             /* message buffer length     */
   MQLONG msgLen;                                /* message length received   */
   /***************************************************************************/
   /* Other variables                                                         */
   /***************************************************************************/
   char *pStr;                                   /* ptr to string             */
   MQLONG balanceChange;                         /* balance change            */
   char name[40];                                /* name                      */
   MQLONG account;                               /* account number            */
   MQLONG balance;                               /* balance                   */
   MQLONG transactions;                          /* transactions              */
   MQLONG temp;                                  /* temporary variable        */
   MQLONG feeDue;                                /* fee due                   */
   MQLONG tranFee;                               /* transaction fee           */
   BOOL gotMsg;                                  /* got message from queue    */
   BOOL committedUpdate=FALSE;                   /* committed update          */

   /***************************************************************************/
   /* First check we have been given correct arguments                        */
   /***************************************************************************/
   if (argc != 2 && argc != 3)
   {
      printf("Input is: %s 'queue name' 'queue manager name'.\n"
             "Note the queue manager name is optional\n", argv[0]);
      exit(99);
   } /* endif */

   if (strlen(argv[1]) <= MQ_Q_NAME_LENGTH)
   {
      strcpy(QName, argv[1]);                    /* get the queue name        */
   }
   else
   {
      printf("Queue name too long, max length %d\n", MQ_Q_NAME_LENGTH);
      exit(99);
   } /* endif */

   if (argc == 3)                                /* use the queue manager     */
   {                                             /* name if supplied          */
      if (strlen(argv[2]) <= MQ_Q_MGR_NAME_LENGTH)
      {
         strcpy(QMgrName, argv[2]);
      }
      else
      {
         printf("Queue manager name too long, max length %d\n", MQ_Q_MGR_NAME_LENGTH);
         exit(99);
      } /* endif */
   } /* endif */

   /***************************************************************************/
   /* Connect to queue manager                                                */
   /***************************************************************************/
   MQCONN(QMgrName, &hCon, &compCode, &connReason);

   if (compCode == MQCC_FAILED)
   {
     printf("MQCONN ended with reason code %d\n", connReason);
     exit(connReason);                           /* no point in going on      */
   } /* endif */

   /***************************************************************************/
   /* Use input parameter as the name of the target queue                     */
   /***************************************************************************/
   strncpy(od.ObjectName, QName, MQ_Q_NAME_LENGTH);
   printf("Target queue is %s\n", od.ObjectName);

   /***************************************************************************/
   /* Open the target message queue for input                                 */
   /***************************************************************************/
   options = MQOO_INPUT_AS_Q_DEF + MQOO_FAIL_IF_QUIESCING;

   MQOPEN(hCon, &od, options, &hObj, &openCompCode, &reason);

   if (reason != MQRC_NONE)
      printf("MQOPEN ended with reason code %d\n", reason);

   if (openCompCode == MQCC_FAILED)
   {
      printf("Unable to open queue for input\n");
      rc = NOT_OK;                               /* stop further action       */
   } /* endif */

   /***************************************************************************/
   /* Declare the cursors for locking of reads from databases                 */
   /***************************************************************************/
   if (rc == OK)
      rc = DeclareMQBankDBCursor();
   if (rc == OK)
      rc = DeclareMQFeeDBCursor();

   /***************************************************************************/
   /* Set up some things for the MQGET                                        */
   /***************************************************************************/
   msgBufLen = sizeof(msgBuf) - 1;
   gmo.Version = MQGMO_VERSION_2;    /* Avoid need to reset message ID and    */
   gmo.MatchOptions = MQMO_NONE;     /* correlation ID after every MQGET      */
   gmo.Options = MQGMO_WAIT + MQGMO_CONVERT + MQGMO_SYNCPOINT;
   gmo.WaitInterval = 15000;                     /* 15 sec limit for waiting  */

   /***************************************************************************/
   /* Get messages from the message queue, loop until there is a failure      */
   /***************************************************************************/
   while (rc == OK)
   {
      /************************************************************************/
      /* Set flags so that we can back out if something goes wrong and not    */
      /* lose the message.                                                    */
      /************************************************************************/
      gotMsg = FALSE;
      committedUpdate = FALSE;

      /************************************************************************/
      /* Start a unit of work                                                 */
      /************************************************************************/
      MQBEGIN (hCon, &bo, &compCode, &reason);

      if (reason == MQRC_NONE)
      {
         printf("Unit of work started\n");
      }
      else
      {
         if (reason == MQRC_NO_EXTERNAL_PARTICIPANTS)
            printf("No participating resource managers registered\n");
         else if (reason == MQRC_PARTICIPANT_NOT_AVAILABLE)
            printf("Participating resource manager not avaliable\n");
         else
            printf("MQBEGIN ended with reason code %d\n"
                   "Unable to start a unit of work\n", reason);

         /*********************************************************************/
         /* If we get a reason code and only a warning on the compCode, there */
         /* is something wrong with one or more of the resource managers so   */
         /* stop looping and sort it out, whatever the compCode.              */
         /*********************************************************************/
         rc = NOT_OK;                            /* stop looping              */
      } /* endif */

      /************************************************************************/
      /* Get message off queue                                                */
      /************************************************************************/
      if (rc == OK)
      {
         MQGET(hCon, hObj, &md, &gmo, msgBufLen, msgBuf, &msgLen,
               &compCode, &reason);

         if (reason != MQRC_NONE)
         {
            if (reason == MQRC_NO_MSG_AVAILABLE)
               printf("No more messages\n");
            else
               printf("MQGET ended with reason code %d\n", reason);

            rc = NOT_OK;                         /* stop looping              */
         }
         else
         {
            gotMsg = TRUE;
         } /* endif */
      } /* endif */

      /************************************************************************/
      /* Process the message received                                         */
      /************************************************************************/
      if (rc == OK)
      {
         msgBuf[msgLen] = '\0';                  /* add string terminator     */

         pStr = strstr(msgBuf, "UPDATE Balance change=");
         if (pStr != NULL)
         {
            pStr += sizeof("UPDATE Balance change=") -1;
            sscanf(pStr, "%d", &balanceChange);
         }
         else
         {
            printf("Invalid string received: %s\n", msgBuf);
            rc = NOT_OK;                         /* stop looping              */
         } /* endif */

         if (rc == OK)
         {
            pStr = strstr(msgBuf, "Account=");
            if (pStr != NULL)
            {
               pStr += sizeof("Account=") -1;
               sscanf(pStr, "%d", &account);
            }
            else
            {
               printf("Invalid string received: %s\n", msgBuf);
               rc = NOT_OK;                      /* stop looping              */
            } /* endif */
         } /* endif */

         /*********************************************************************/
         /* Note only actively connected to one database at a time            */
         /*********************************************************************/
         if (rc == OK)
            rc = ConnectToMQBankDB();

         /*********************************************************************/
         /* Get details from MQBankTB table in MQBankDB database              */
         /*********************************************************************/
         if (rc == OK)
            rc = GetMQBankTBDetails(name, account, &balance, &transactions);

         if (rc == OK)
            rc = ConnectToMQFeeDB();

         /*********************************************************************/
         /* Get details from MQFeeTB table in MQFeeDB database                */
         /*********************************************************************/
         if (rc == OK)
            rc = GetMQFeeTBDetails(account, &feeDue, &tranFee, &temp);

         /*********************************************************************/
         /* The number of transactions to the two databases should be         */
         /* identical, stop if not.                                           */
         /*********************************************************************/
         if (rc == OK)
         {
            if (temp != transactions)
            {
               printf("Databases are out of step !\n");
               rc = NOT_OK;                      /* stop looping              */
            } /* endif */
         } /* endif */

         /*********************************************************************/
         /* Update the bank balance                                           */
         /*********************************************************************/
         if (rc == OK)
         {
            transactions++;                      /* bump no of transactions   */
            balance += balanceChange;            /* alter balance             */
            feeDue += tranFee;                   /* alter fee due             */

            rc = UpdateMQFeeTBFeeDue(feeDue, transactions);

            if (rc == OK)                        /* must now connect back to  */
               rc = ConnectToMQBankDB();         /* other database            */
            if (rc == OK)
               rc = UpdateMQBankTBBalance(balance, transactions);

            if (rc == OK)
            {
               printf("Account No %d Balance updated from %d to %d %s\n",
                      account, balance - balanceChange, balance, name);
               printf("Fee Due updated from %d to %d\n",
                      feeDue - tranFee, feeDue);

               /***************************************************************/
               /* Set the flag showing that the message has been commited.    */
               /* It is possible that the MQCMIT will fail, there are a many  */
               /* reasons why something may go wrong, so in these cases, some */
               /* of which will reqire manual intervention, the best thing we */
               /* can do is stop looping, and terminate the program making    */
               /* sure MQBACK is not issued.                                  */
               /***************************************************************/
               committedUpdate = TRUE;

               /***************************************************************/
               /* Note: the cursor will be implicitly closed by the MQCMIT.   */
               /***************************************************************/
               MQCMIT(hCon, &compCode, &reason);

               if (reason == MQRC_NONE)
               {
                  printf("Unit of work successfully completed\n");
               }
               else if (reason == MQRC_BACKED_OUT)
               {
                  /************************************************************/
                  /* If we have been backed out for some reason, it may be a  */
                  /* transatory problem, so we will keep trying until it's    */
                  /* obvious that we are never going to commit this update.   */
                  /************************************************************/
                  if (md.BackoutCount == 0)
                  {
                     printf("The unit of work was backed out, will try again\n");
                  }
                  else if (md.BackoutCount > 100)
                  {
                     printf("MQGET backout count exceeded program limit\n");
                     rc = NOT_OK;                /* stop looping              */
                  } /* endif */
               }
               else
               {
                  printf("MQCMIT ended with reason code %d completion code "
                         "%d\n", reason, compCode);
                  rc = NOT_OK;                   /* stop looping              */
               } /* endif */
            } /* endif */
         } /* endif */
      } /* endif */

      /************************************************************************/
      /* If we got the message but something went wrong, back out so that we  */
      /* don't lose the message, unless we have issued MQCMIT.                */
      /************************************************************************/
      if (gotMsg && !committedUpdate)
      {
         MQBACK(hCon, &compCode, &reason);

         if (reason == MQRC_NONE)
            printf("MQBACK successfully issued\n");
         else
            printf("MQBACK ended with reason code %d\n", reason);
      } /* endif */
   } /* endwhile */

   /***************************************************************************/
   /* Normal loop termination should be no more messages left on the queue,   */
   /* if this is the case set an OK return code.                              */
   /***************************************************************************/
   if (reason == MQRC_NO_MSG_AVAILABLE)
      rc = OK;

   /***************************************************************************/
   /* Close queue if opened                                                   */
   /***************************************************************************/
   if (openCompCode != MQCC_FAILED)
   {
      options = 0;                               /* no close options          */

      MQCLOSE(hCon, &hObj, options, &compCode, &reason);

      if (reason != MQRC_NONE)
         printf("MQCLOSE ended with reason code %d\n", reason);
   } /* endif */

   /***************************************************************************/
   /* Disconnect from queue manager if not already connected, unless we have  */
   /* had an MQCMIT failure, in which case the flag will still be set. MQDISC */
   /* may issue an implicit MQBACK, which we do not want in the failure case. */
   /***************************************************************************/
   if (connReason != MQRC_ALREADY_CONNECTED && !committedUpdate)
   {
      MQDISC(&hCon, &compCode, &reason);

      if (reason != MQRC_NONE)
         printf("MQDISC ended with reason code %d\n", reason);
   } /* endif */

   return rc;
                                                 /*****************************/
}                                                /* ------ END OF MAIN ------ */
                                                 /*****************************/
