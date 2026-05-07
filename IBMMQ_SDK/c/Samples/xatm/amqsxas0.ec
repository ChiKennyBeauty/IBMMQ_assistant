static const char sccsid[]= "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/xatm/amqsxas0.pre";
/******************************************************************************/
/*                                                                            */
/* Program name: AMQSXAS                                                      */
/*                                                                            */
/* Description:  Sample program for MQ coordinating an XA-compliant database  */
/*               manager using SQL and MQ calls.                              */
/*                                                                            */
/* Note:         Useful information and diagrams about this sample are in the */
/*               Application Programming Guide.                               */
/*                                                                            */
/* <copyright                                                                 */
/* notice="lm-source-program"                                                 */
/* pids=""                                                                    */
/* years="1996,2016"                                                          */
/* crc="3032758260" >                                                         */
/* Licensed Materials - Property of IBM                                       */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* (C) Copyright IBM Corp. 1996, 2016 All Rights Reserved.                    */
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
/*    AMQSXAS is a sample program for MQ to coordinate getting a message      */
/*    off a queue and updating a database within an MQ unit of work.          */
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
/*    .  get information from database                                        */
/*    .  update information from database                                     */
/*    .  MQCMIT commit changes                                                */
/*    .  print updated information                                            */
/*    .  (no message available counts as failure, and loop ends)              */
/*    MQCLOSE close queue                                                     */
/*    MQDISC disconnect from queue manager                                    */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* AMQSXAS has 2 parameters - the name of the message queue (required)        */
/*                           - the queue manager name (optional)              */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Notes:                                                                     */
/*                                                                            */
/*    This program was prepared for use with the IBM Informix database        */
/*    product.                                                                */
/*                                                                            */
/*    Before this sample can be compiled, the database and table must be      */
/*    created, for example:                                                   */
/*                                                                            */
/*    echo create database MQBankDB with log > MQBankDB.fil                   */
/*    dbaccess < MQBankDB.fil                                                 */
/*                                                                            */
/*    MQBankT table was created using the following SQL statement:            */
/*                                                                            */
/*       EXEC SQL CREATE TABLE MQBankT(Name         VARCHAR(40) NOT NULL,     */
/*                                     Account      INTEGER     NOT NULL,     */
/*                                     Balance      INTEGER     NOT NULL,     */
/*                                     PRIMARY KEY (Account));                */
/*                                                                            */
/*       ALTER TABLE MQBankT LOCK MODE (ROW);                                 */
/*                                                                            */
/*    MQBankT table was filled in using the following SQL statements:         */
/*                                                                            */
/*       EXEC SQL INSERT INTO MQBankT VALUES ('Mr Fred Bloggs',1,0);          */
/*       EXEC SQL INSERT INTO MQBankT VALUES ('Mrs S Smith',2,0);             */
/*       EXEC SQL INSERT INTO MQBankT VALUES ('Ms Mary Brown',3,0);           */
/*       etc.                                                                 */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*----------------------------------------------------------------------------*/
/* MQ includes                                                                */
/*----------------------------------------------------------------------------*/
#include <cmqc.h>                                /* MQI                       */

/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/
#ifndef OK
   #define OK                0                   /* define OK as zero         */
#endif
#ifndef NOT_OK
   #define NOT_OK            1                   /* define NOT_OK as one      */
#endif

#ifndef BOOL
   #define BOOL MQULONG
#endif
#ifndef TRUE
   #define TRUE              1
#endif
#ifndef FALSE
   #define FALSE             0
#endif

/******************************************************************************/
/* Define and declare an SQLCA (SQL Communication Area) structure             */
/******************************************************************************/
EXEC SQL INCLUDE SQLCA;

/******************************************************************************/
/* Define a macro for checking if an SQL call resulted in an error            */
/******************************************************************************/
#define CHECKERR(SQL_STR)                                                      \
   if (sqlca.sqlcode != 0 && sqlca.sqlcode != 100)                             \
   {                                                                           \
      mint msgLen;                                                             \
      char buffer[512];                                                        \
      rc = sqlca.sqlcode;                                                      \
      printf("Error: %s at line: %d in file: %s\n",                            \
             SQL_STR, __LINE__, __FILE__);                                     \
      rgetlmsg(sqlca.sqlcode, buffer, sizeof(buffer), &msgLen);                \
      printf("SQL %d: ", sqlca.sqlcode);                                       \
      printf(buffer, sqlca.sqlerrm);                                           \
      if (sqlca.sqlerrd[1] != 0)                                               \
      {                                                                        \
         rgetlmsg(sqlca.sqlerrd[1], buffer, sizeof(buffer), &msgLen);          \
         printf("ISAM %d: ", sqlca.sqlerrd[1]);                                \
         printf(buffer, sqlca.sqlerrm);                                        \
      } /* endif */                                                            \
   } /* endif */


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
   int rc=OK;                                    /* return code               */
   /***************************************************************************/
   /* MQI structures                                                          */
   /***************************************************************************/
   MQOD od={MQOD_DEFAULT};                       /* object descriptor         */
   MQMD md={MQMD_DEFAULT};                       /* message descriptor        */
   MQGMO gmo={MQGMO_DEFAULT};                    /* get message options       */
   MQBO bo={MQBO_DEFAULT};                       /* begin options             */
   /***************************************************************************/
   /* MQI variables                                                           */
   /***************************************************************************/
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
   BOOL gotMsg;                                  /* got message from queue    */
   BOOL committedUpdate=FALSE;                   /* committed update          */
   MQLONG balanceChange;                         /* balance change            */
   BOOL DBInitialised=FALSE;                     /* database initialised      */
   /***************************************************************************/
   /* SQL host declarations                                                   */
   /***************************************************************************/
   EXEC SQL BEGIN DECLARE SECTION;
      char name[40];                             /* name                      */
      int account;                               /* account number            */
      int balance;                               /* balance                   */
   EXEC SQL END DECLARE SECTION;

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
         /* If we get a reason code, regardless of the compCode, there is     */
         /* something wrong with one or more of the resource managers so stop */
         /* looping and sort it out.                                          */
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
         /* Now we are connected to the database we can initialise some       */
         /* database parameters.                                              */
         /*********************************************************************/
         if (rc == OK && !DBInitialised)
         {
            /******************************************************************/
            /* Set the isolation level                                        */
            /******************************************************************/
            EXEC SQL SET ISOLATION TO COMMITTED READ;
            CHECKERR ("SET ISOLATION");

            /******************************************************************/
            /* To allow multiple copies of the program to run concurrently,   */
            /* we will set the lock mode to wait up to 15 seconds before      */
            /* detecting deadlock.                                            */
            /******************************************************************/
            if (rc == OK)
            {
               EXEC SQL SET LOCK MODE TO WAIT 15;
               CHECKERR ("SET LOCK MODE");
            } /* endif */

            DBInitialised = TRUE;
         } /* endif */

         /*********************************************************************/
         /* Declare the cursor for locking of reads from database             */
         /*********************************************************************/
         if (rc == OK)
         {
            EXEC SQL DECLARE cur CURSOR FOR
                     SELECT Name, Balance
                     FROM MQBankT
                     WHERE Account = :account
                     FOR UPDATE OF Balance;
            CHECKERR ("DECLARE CURSOR");
         } /* endif */

         /*********************************************************************/
         /* Get details from database                                         */
         /*********************************************************************/
         if (rc == OK)
         {
            EXEC SQL OPEN cur;
            CHECKERR ("OPEN CURSOR");
         } /* endif */

         if (rc == OK)
         {
            EXEC SQL FETCH cur INTO :name, :balance;
            CHECKERR ("FETCH");
         } /* endif */

         /*********************************************************************/
         /* Update the bank balance                                           */
         /*********************************************************************/
         if (rc == OK)
         {
            balance += balanceChange;            /* alter balance             */

            EXEC SQL UPDATE MQBankT SET Balance = :balance
                                    WHERE CURRENT OF cur;
            CHECKERR ("UPDATE MQBankT");

            if (rc == OK)
            {
               printf("Account No %d Balance updated from %d to %d %s\n",
                      account, balance - balanceChange, balance, name);

               /***************************************************************/
               /* Set the flag showing that the message has been commited.    */
               /* It is possible that the MQCMIT will fail, there are many    */
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
