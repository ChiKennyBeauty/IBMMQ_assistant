static const char sccsid[]= "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/xatm/amqsxaf0.pre";
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
/* crc="3858133071" >                                                         */
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
/* Description: Functions to access MQFeeTB table in MQFeeDB database         */
/*                                                                            */
/* Function:    These functions provide access to MQFeeTB table in MQFeeDB    */
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
/*    echo create database MQFeeDB with log > MQFeeDB.fil                     */
/*    dbaccess < MQFeeDB.fil                                                  */
/*                                                                            */
/*    MQFeeTB table was created using the following SQL statement:            */
/*                                                                            */
/*       EXEC SQL CREATE TABLE MQFeeTB(Account      INTEGER     NOT NULL,     */
/*                                     FeeDue       INTEGER     NOT NULL,     */
/*                                     TranFee      INTEGER     NOT NULL,     */
/*                                     Transactions INTEGER     NOT NULL,     */
/*                                     PRIMARY KEY (Account));                */
/*                                                                            */
/*       ALTER TABLE MQFeeTB LOCK MODE (ROW);                                 */
/*                                                                            */
/*    MQFeeTB table was filled in using the following SQL statements:         */
/*                                                                            */
/*       EXEC SQL INSERT INTO MQFeeTB VALUES (1,0,50,0);                      */
/*       EXEC SQL INSERT INTO MQFeeTB VALUES (2,0,50,0);                      */
/*       EXEC SQL INSERT INTO MQFeeTB VALUES (3,0,50,0);                      */
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
static int hAccount;                             /* account number            */
static int hFeeDue;                              /* fee due                   */
static int hTranFee;                             /* Transaction fee           */
static int hTransactions;                        /* transactions              */
EXEC SQL END DECLARE SECTION;

/******************************************************************************/
/*                                                                            */
/* Function: InitialiseMQFeeDB                                                */
/*                                                                            */
/* Description: Initialise MQFeeDB database parameters                        */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Function:                                                                  */
/* ---------                                                                  */
/* Set the isolation level and lock mode for updates to MQFeeDB database.     */
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
MQLONG InitialiseMQFeeDB(void)
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
/* Function: DeclareMQFeeDBCursor                                             */
/*                                                                            */
/* Description: Declare MQFeeDB database MQFeeTB table cursor                 */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Function:                                                                  */
/* ---------                                                                  */
/* Declare a cursor for updating MQFeeTB table in MQFeeDB database.           */
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
MQLONG DeclareMQFeeDBCursor(void)
{
   MQLONG rc=OK;                                 /* return code               */

   /***************************************************************************/
   /* Declare the cursor for locking of reads from database                   */
   /***************************************************************************/
   EXEC SQL DECLARE curFee CURSOR FOR
            SELECT FeeDue, TranFee, Transactions
            FROM MQFeeDB:MQFeeTB
            WHERE Account = :hAccount
            FOR UPDATE OF FeeDue, Transactions;
   CHECKERR ("DECLARE CURSOR");

   return(rc);
}

/******************************************************************************/
/*                                                                            */
/* Function: GetMQFeeTBDetails                                                */
/*                                                                            */
/* Description: Get MQFeeTB table details for supplied account number         */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Function:                                                                  */
/* ---------                                                                  */
/* Get MQFeeTB table details from MQFeeDB database for the supplied account   */
/* number.                                                                    */
/*                                                                            */
/* Input Parameters:                                                          */
/* -----------------                                                          */
/* MQLONG   account               - Account in MQFeeTB table                  */
/*                                                                            */
/* In/Out Parameters:                                                         */
/* ------------------                                                         */
/* None                                                                       */
/*                                                                            */
/* Output Parameters:                                                         */
/* ------------------                                                         */
/* MQLONG  *feeDue                - FeeDue in MQFeeTB table                   */
/* MQLONG  *tranFee               - TranFee in MQFeeTB table                  */
/* MQLONG  *transactions          - Transactions in MQFeeTB table             */
/*                                                                            */
/* Returns:                                                                   */
/* --------                                                                   */
/* OK                             - normal execution                          */
/* Reason                         - return code from SQL command              */
/*                                                                            */
/******************************************************************************/
MQLONG GetMQFeeTBDetails(MQLONG account, MQLONG *feeDue, MQLONG *tranFee,
                         MQLONG *transactions)
{
   MQLONG rc=OK;                                 /* return code               */

   /***************************************************************************/
   /* Copy calling function variable to SQL host variable                     */
   /***************************************************************************/
   hAccount = account;

   EXEC SQL OPEN curFee;
   CHECKERR ("OPEN CURSOR");

   if (rc == OK)
   {
      EXEC SQL FETCH curFee INTO :hFeeDue, :hTranFee, :hTransactions;
      CHECKERR ("FETCH");
   } /* endif */

   /***************************************************************************/
   /* Copy SQL host variables to calling function variables                   */
   /***************************************************************************/
   if (rc == OK)
   {
      *feeDue = hFeeDue;
      *tranFee = hTranFee;
      *transactions = hTransactions;
   } /* endif */

   return(rc);
}

/******************************************************************************/
/*                                                                            */
/* Function: UpdateMQFeeTBFeeDue                                              */
/*                                                                            */
/* Description: Update MQFeeTB table FeeDue and Transactions for the current  */
/*              cursor                                                        */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Function:                                                                  */
/* ---------                                                                  */
/* Update MQFeeTB table FeeDue and Transactions in MQFeeDB database for the   */
/* current cursor.                                                            */
/*                                                                            */
/* Input Parameters:                                                          */
/* -----------------                                                          */
/* MQLONG   feeDue                - FeeDue in MQFeeTB table                   */
/* MQLONG   transactions          - Transactions in MQFeeTB table             */
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
MQLONG UpdateMQFeeTBFeeDue(MQLONG feeDue, MQLONG transactions)
{
   MQLONG rc=OK;                                 /* return code               */

   /***************************************************************************/
   /* Copy calling function variables to SQL host variables                   */
   /***************************************************************************/
   hFeeDue = feeDue;
   hTransactions = transactions;

   EXEC SQL UPDATE MQFeeDB:MQFeeTB SET FeeDue = :hFeeDue,
                                       Transactions = :hTransactions
                                       WHERE CURRENT OF curFee;
   CHECKERR ("UPDATE MQFeeTB");

   return(rc);
}
