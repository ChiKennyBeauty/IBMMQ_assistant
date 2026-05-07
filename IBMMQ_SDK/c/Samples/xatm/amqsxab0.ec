static const char sccsid[]= "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/xatm/amqsxab0.pre";
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
/* years="1998,2016"                                                          */
/* crc="1788976538" >                                                         */
/* Licensed Materials - Property of IBM                                       */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* (C) Copyright IBM Corp. 1998, 2016 All Rights Reserved.                    */
/*                                                                            */
/* US Government Users Restricted Rights - Use, duplication or                */
/* disclosure restricted by GSA ADP Schedule Contract with                    */
/* IBM Corp.                                                                  */
/* </copyright>                                                               */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Description: Functions to access MQBankTB table in MQBankDB database       */
/*                                                                            */
/* Function:    These functions provide access to MQBankTB table in MQBankDB  */
/*              database, they are called from AMQSXAG0.C                     */
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
/*    MQBankTB table was created using the following SQL statement:           */
/*                                                                            */
/*       EXEC SQL CREATE TABLE MQBankTB(Name         VARCHAR(40) NOT NULL,    */
/*                                      Account      INTEGER     NOT NULL,    */
/*                                      Balance      INTEGER     NOT NULL,    */
/*                                      Transactions INTEGER     NOT NULL,    */
/*                                      PRIMARY KEY (Account));               */
/*                                                                            */
/*       ALTER TABLE MQBankTB LOCK MODE (ROW);                                */
/*                                                                            */
/*    MQBankTB table was filled in using the following SQL statements:        */
/*                                                                            */
/*       EXEC SQL INSERT INTO MQBankTB VALUES ('Mr Fred Bloggs',1,0,0);       */
/*       EXEC SQL INSERT INTO MQBankTB VALUES ('Mrs S Smith',2,0,0);          */
/*       EXEC SQL INSERT INTO MQBankTB VALUES ('Ms Mary Brown',3,0,0);        */
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
   #define OK        0                           /* define OK as zero         */
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
/* SQL host declarations                                                      */
/******************************************************************************/
EXEC SQL BEGIN DECLARE SECTION;
static char hName[40];                           /* name                      */
static int hAccount;                             /* account number            */
static int hBalance;                             /* balance                   */
static int hTransactions;                        /* transactions              */
EXEC SQL END DECLARE SECTION;

/******************************************************************************/
/*                                                                            */
/* Function: InitialiseMQBankDB                                               */
/*                                                                            */
/* Description: Initialise MQBankDB database parameters                       */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Function:                                                                  */
/* ---------                                                                  */
/* Set the isolation level and lock mode for updates to MQBankDB database.    */
/*                                                                            */
/* Input Parameters:                                                          */
/* -----------------                                                          */
/* None                                                                       */
/*                                                                            */
/* In/Out Parameters:                                                         */
/* ------------------                                                         */
/* None                                                                       */
/*                                                                            */
/* Output Parameters:                                                         */
/* ------------------                                                         */
/* None                                                                       */
/*                                                                            */
/* Returns:                                                                   */
/* --------                                                                   */
/* OK                             - normal execution                          */
/* Reason                         - return code from SQL command              */
/*                                                                            */
/******************************************************************************/
MQLONG InitialiseMQBankDB(void)
{
   MQLONG rc=OK;                                 /* return code               */

   /***************************************************************************/
   /* We can now set the isolation level as we are connected to the database  */
   /***************************************************************************/
   EXEC SQL SET ISOLATION TO COMMITTED READ;
   CHECKERR ("SET ISOLATION");

   /***************************************************************************/
   /* To allow multiple copies of the program to run concurrently, we will    */
   /* set the lock mode to wait up to 15 seconds before detecting deadlock.   */
   /***************************************************************************/
   if (rc == OK)
   {
      EXEC SQL SET LOCK MODE TO WAIT 15;
      CHECKERR ("SET LOCK MODE");
   } /* endif */

   return(rc);
}

/******************************************************************************/
/*                                                                            */
/* Function: DeclareMQBankDBCursor                                            */
/*                                                                            */
/* Description: Declare MQBankDB database MQBankTB table cursor               */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Function:                                                                  */
/* ---------                                                                  */
/* Declare a cursor for updating MQBankTB table in MQBankDB database.         */
/*                                                                            */
/* Input Parameters:                                                          */
/* -----------------                                                          */
/* None                                                                       */
/*                                                                            */
/* In/Out Parameters:                                                         */
/* ------------------                                                         */
/* None                                                                       */
/*                                                                            */
/* Output Parameters:                                                         */
/* ------------------                                                         */
/* None                                                                       */
/*                                                                            */
/* Returns:                                                                   */
/* --------                                                                   */
/* OK                             - normal execution                          */
/* Reason                         - return code from SQL command              */
/*                                                                            */
/******************************************************************************/
MQLONG DeclareMQBankDBCursor(void)
{
   MQLONG rc=OK;                                 /* return code               */

   /***************************************************************************/
   /* Declare the cursor for locking of reads from database                   */
   /***************************************************************************/
   EXEC SQL DECLARE curBank CURSOR FOR
            SELECT Name, Balance, Transactions
            FROM MQBankTB
            WHERE Account = :hAccount
            FOR UPDATE OF Balance, Transactions;
   CHECKERR ("DECLARE CURSOR");

   return(rc);
}

/******************************************************************************/
/*                                                                            */
/* Function: GetMQBankTBDetails                                               */
/*                                                                            */
/* Description: Get MQBankTB table details for supplied account number        */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Function:                                                                  */
/* ---------                                                                  */
/* Get MQBankTB table details from MQBankDB database for the supplied account */
/* number.                                                                    */
/*                                                                            */
/* Input Parameters:                                                          */
/* -----------------                                                          */
/* MQLONG   account               - Account in MQBankTB table                 */
/*                                                                            */
/* In/Out Parameters:                                                         */
/* ------------------                                                         */
/* None                                                                       */
/*                                                                            */
/* Output Parameters:                                                         */
/* ------------------                                                         */
/* char    *name                  - Name in MQBankTB table                    */
/* MQLONG  *balance               - Balance in MQBankTB table                 */
/* MQLONG  *transactions          - Transactions in MQBankTB table            */
/*                                                                            */
/* Returns:                                                                   */
/* --------                                                                   */
/* OK                             - normal execution                          */
/* Reason                         - return code from SQL command              */
/*                                                                            */
/******************************************************************************/
MQLONG GetMQBankTBDetails(char *name, MQLONG account, MQLONG *balance,
                          MQLONG *transactions)
{
   MQLONG rc=OK;                                 /* return code               */

   /***************************************************************************/
   /* Copy calling function variable to SQL host variable                     */
   /***************************************************************************/
   hAccount = account;

   EXEC SQL OPEN curBank;
   CHECKERR ("OPEN CURSOR");

   if (rc == OK)
   {
      EXEC SQL FETCH curBank INTO :hName, :hBalance, :hTransactions;
      CHECKERR ("FETCH");
   } /* endif */

   /***************************************************************************/
   /* Copy SQL host variables to calling function variables                   */
   /***************************************************************************/
   if (rc == OK)
   {
      strcpy(name, hName);
      *balance = hBalance;
      *transactions = hTransactions;
   } /* endif */

   return(rc);
}

/******************************************************************************/
/*                                                                            */
/* Function: UpdateMQBankTBBalance                                            */
/*                                                                            */
/* Description: Update MQBankTB table Balance and Transactions for the        */
/*              current cursor                                                */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Function:                                                                  */
/* ---------                                                                  */
/* Update MQBankTB table Balance and Transactions in MQBankDB database for    */
/* the current cursor.                                                        */
/*                                                                            */
/* Input Parameters:                                                          */
/* -----------------                                                          */
/* MQLONG   balance               - Balance in MQBankTB table                 */
/* MQLONG   transactions          - Transactions in MQBankTB table            */
/*                                                                            */
/* In/Out Parameters:                                                         */
/* ------------------                                                         */
/* None                                                                       */
/*                                                                            */
/* Output Parameters:                                                         */
/* ------------------                                                         */
/* None                                                                       */
/*                                                                            */
/* Returns:                                                                   */
/* --------                                                                   */
/* OK                             - normal execution                          */
/* Reason                         - return code from SQL command              */
/*                                                                            */
/******************************************************************************/
MQLONG UpdateMQBankTBBalance(MQLONG balance, MQLONG transactions)
{
   MQLONG rc=OK;                                 /* return code               */

   /***************************************************************************/
   /* Copy calling function variables to SQL host variables                   */
   /***************************************************************************/
   hBalance = balance;
   hTransactions = transactions;

   EXEC SQL UPDATE MQBankTB SET Balance = :hBalance,
                                Transactions = :hTransactions
                            WHERE CURRENT OF curBank;
   CHECKERR ("UPDATE MQBankTB");

   return(rc);
}
