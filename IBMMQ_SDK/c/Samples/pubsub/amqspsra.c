/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/pubsub/amqspsra.c */
/******************************************************************************/
/*                                                                            */
/* Module name: AMQSPSRA.C                                                    */
/*                                                                            */
/* Description: Sample C program for MQSeries Publish/Subscribe routing       */
/*              exit.                                                         */
/*                                                                            */
/*   <copyright                                                                */
/*   notice="lm-source-program"                                                */
/*   pids="5724-H72,"                                                          */
/*   years="1994,2012"                                                         */
/*   crc="449609625" >                                                         */
/*   Licensed Materials - Property of IBM                                      */
/*                                                                             */
/*   5724-H72,                                                                 */
/*                                                                             */
/*   (C) Copyright IBM Corp. 1994, 2012 All Rights Reserved.                   */
/*                                                                             */
/*   US Government Users Restricted Rights - Use, duplication or               */
/*   disclosure restricted by GSA ADP Schedule Contract with                   */
/*   IBM Corp.                                                                 */
/*   </copyright>                                                              */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Function:                                                                  */
/*                                                                            */
/*   AMQSPSRA is a sample C program for MQSeries Publish/Subscribe routing    */
/*   exit. It changes either the destination queue or queue manager           */
/*   depending upon the parameters supplied.                                  */
/*                                                                            */
/* Program logic:                                                             */
/*                                                                            */
/*    if message destination = application                                    */
/*       if stream name = default stream                                      */
/*          if destination Q name = Q1                                        */
/*             change destination Q name to Q2                                */
/*          else if destination Q name = Q2                                   */
/*             change destination Q name to Q3                                */
/*          else if destination Q name = Q3                                   */
/*             change destination Q name to Q4                                */
/*    else if message destination = broker                                    */
/*       if stream name = my routing stream                                   */
/*          if destination QMgr name = QMgr1                                  */
/*             change destination QMgr name to QMgr2                          */
/*          else if destination QMgr name = QMgr2                             */
/*             change destination QMgr name to QMgr3                          */
/*          else if destination QMgr name = QMgr3                             */
/*             change destination QMgr name to QMgr4                          */
/*                                                                            */
/*    WARNING: The usage of this is now deprecated.                           */
/*    Please use amqspub/amqssub instead.                                     */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* AMQSPSRA should be built into a dynamically loadable module                */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* Includes                                                                   */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <string.h>

#include <cmqc.h>             /* MQI                                          */
#include <cmqxc.h>            /* MQI exit                                     */


/******************************************************************************/
/*                                                                            */
/* Defines                                                                    */
/*                                                                            */
/******************************************************************************/

#define TRUE  1
#define FALSE 0


/******************************************************************************/
/*                                                                            */
/* Function: MQStart                                                          */
/*                                                                            */
/* Description: Dummy entry function for MQSeries                             */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Function:                                                                  */
/* ---------                                                                  */
/* Dummy entry function for MQSeries                                          */
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
/* None                                                                       */
/*                                                                            */
/******************************************************************************/
void MQENTRY MQStart(void)
{

}

/******************************************************************************/
/*                                                                            */
/* Function: RoutingExit                                                      */
/*                                                                            */
/* Description: PubSub routing exit                                           */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Function:                                                                  */
/* ---------                                                                  */
/* PubSub routing exit                                                        */
/*                                                                            */
/* Input Parameters:                                                          */
/* -----------------                                                          */
/* PMQPXP pExitParm               - ptr to MQ PubSub routing exit parameters  */
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
/* None                                                                       */
/*                                                                            */
/******************************************************************************/
void MQENTRY RoutingExit(PMQPXP pExitParm)
{
   int QFound=FALSE;
   int QMgrFound=FALSE;
   char streamName[MQ_Q_NAME_LENGTH];
   char QNameIn[MQ_Q_NAME_LENGTH];
   char QNameOut[MQ_Q_NAME_LENGTH];
   char QMgrNameIn[MQ_Q_MGR_NAME_LENGTH];
   char QMgrNameOut[MQ_Q_MGR_NAME_LENGTH];

   /***************************************************************************/
   /* Switch on the reason the exit was called                                */
   /***************************************************************************/
   switch (pExitParm->ExitReason)
   {
      case MQXR_INIT:

         /*********************************************************************/
         /* Do any initialisation required, for example:                      */
         /*                                                                   */
         /*    Put something in exit user area (pExitParm->ExitUserArea) -    */
         /*    16 bytes are avaliable. If you have allocated some memory you  */
         /*    could use this area to store a pointer to it.                  */
         /*                                                                   */
         /*    Take advantage of the exit data (pExitParm->ExitData) - 32     */
         /*    chars from the QM.INI file are passed in for your use.         */
         /*                                                                   */
         /*    Allocate any memory required, see above.                       */
         /*                                                                   */
         /*    etc.                                                           */
         /*                                                                   */
         /*********************************************************************/

         break;

      case MQXR_TERM:

         /*********************************************************************/
         /* Do any termination required, for example:                         */
         /*                                                                   */
         /*    Take advantage of the exit data (pExitParm->ExitData) - 32     */
         /*    chars from the QM.INI file are passed in for your use.         */
         /*                                                                   */
         /*    Free any memory allocated.                                     */
         /*                                                                   */
         /*    etc.                                                           */
         /*                                                                   */
         /*********************************************************************/

         break;

      case MQXR_MSG:

         /*********************************************************************/
         /* We have been called due to a message being published.             */
         /*                                                                   */
         /* Program logic:                                                    */
         /*                                                                   */
         /*    if message destination = application                           */
         /*       if stream name = default stream                             */
         /*          if destination Q name = Q1                               */
         /*             change destination Q name to Q2                       */
         /*          else if destination Q name = Q2                          */
         /*             change destination Q name to Q3                       */
         /*          else if destination Q name = Q3                          */
         /*             change destination Q name to Q4                       */
         /*    else if message destination = broker                           */
         /*       if stream name = my routing stream                          */
         /*          if destination QMgr name = QMgr1                         */
         /*             change destination QMgr name to QMgr2                 */
         /*          else if destination QMgr name = QMgr2                    */
         /*             change destination QMgr name to QMgr3                 */
         /*          else if destination QMgr name = QMgr3                    */
         /*             change destination QMgr name to QMgr4                 */
         /*                                                                   */
         /*                                                                   */
         /*********************************************************************/

         /*********************************************************************/
         /* We are passed in, and MUST keep names blank padded, NOT string    */
         /* terminated.                                                       */
         /*********************************************************************/
         memset(streamName, ' ', MQ_Q_NAME_LENGTH);
         memset(QNameIn, ' ', MQ_Q_NAME_LENGTH);
         memset(QNameOut, ' ', MQ_Q_NAME_LENGTH);
         memset(QMgrNameIn, ' ', MQ_Q_MGR_NAME_LENGTH);
         memset(QMgrNameOut, ' ', MQ_Q_MGR_NAME_LENGTH);

         if (pExitParm->DestinationType == MQDT_APPL)
         {
            /******************************************************************/
            /* Default stream                                                 */
            /******************************************************************/
            memcpy(streamName, "SYSTEM.BROKER.DEFAULT.STREAM",
                        strlen("SYSTEM.BROKER.DEFAULT.STREAM"));

            if (memcmp(pExitParm->StreamName, streamName, MQ_Q_NAME_LENGTH) == 0)
            {
               memcpy(QNameIn, "Q1", 2);
               if (memcmp(pExitParm->DestinationQName, QNameIn, MQ_Q_NAME_LENGTH) == 0)
               {
                  memcpy(QNameOut, "Q2", 2);
                  memcpy(pExitParm->DestinationQName, QNameOut, MQ_Q_NAME_LENGTH);
                  QFound = TRUE;
               } /* endif */

               if (!QFound)
               {
                  memcpy(QNameIn, "Q2", 2);
                  if (memcmp(pExitParm->DestinationQName, QNameIn, MQ_Q_NAME_LENGTH) == 0)
                  {
                     memcpy(QNameOut, "Q3", 2);
                     memcpy(pExitParm->DestinationQName, QNameOut, MQ_Q_NAME_LENGTH);
                     QFound = TRUE;
                  } /* endif */
               } /* endif */

               if (!QFound)
               {
                  memcpy(QNameIn, "Q3", 2);
                  if (memcmp(pExitParm->DestinationQName, QNameIn, MQ_Q_NAME_LENGTH) == 0)
                  {
                     memcpy(QNameOut, "Q4", 2);
                     memcpy(pExitParm->DestinationQName, QNameOut, MQ_Q_NAME_LENGTH);
                     QFound = TRUE;
                  } /* endif */
               } /* endif */
            } /* endif */
         }
         else if (pExitParm->DestinationType == MQDT_BROKER)
         {
            /******************************************************************/
            /* My routing stream                                              */
            /******************************************************************/
            memcpy(streamName, "MY.ROUTING.STREAM",
                        strlen("MY.ROUTING.STREAM"));

            if (memcmp(pExitParm->StreamName, streamName, MQ_Q_MGR_NAME_LENGTH) == 0)
            {
               memcpy(QMgrNameIn, "QMgr1", 5);
               if (memcmp(pExitParm->DestinationQMgrName, QMgrNameIn, MQ_Q_MGR_NAME_LENGTH) == 0)
               {
                  memcpy(QMgrNameOut, "QMgr2", 5);
                  memcpy(pExitParm->DestinationQMgrName, QMgrNameOut, MQ_Q_MGR_NAME_LENGTH);
                  QMgrFound = TRUE;
               } /* endif */

               if (!QMgrFound)
               {
                  memcpy(QMgrNameIn, "QMgr2", 5);
                  if (memcmp(pExitParm->DestinationQMgrName, QMgrNameIn, MQ_Q_MGR_NAME_LENGTH) == 0)
                  {
                     memcpy(QMgrNameOut, "QMgr3", 5);
                     memcpy(pExitParm->DestinationQMgrName, QMgrNameOut, MQ_Q_MGR_NAME_LENGTH);
                     QMgrFound = TRUE;
                  } /* endif */
               } /* endif */

               if (!QMgrFound)
               {
                  memcpy(QMgrNameIn, "QMgr3", 5);
                  if (memcmp(pExitParm->DestinationQMgrName, QMgrNameIn, MQ_Q_MGR_NAME_LENGTH) == 0)
                  {
                     memcpy(QMgrNameOut, "QMgr4", 5);
                     memcpy(pExitParm->DestinationQMgrName, QMgrNameOut, MQ_Q_MGR_NAME_LENGTH);
                     QMgrFound = TRUE;
                  } /* endif */
               } /* endif */
            } /* endif */

         } /* endif */

         break;

   } /* endswitch */

}
