/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/oltp/amqscih0.h */
 /********************************************************************/
 /*                                                                  */
 /* Module name: AMQSCIHO                                            */
 /*                                                                  */
 /* Environment : CICS/2 Version 2.0; IBM C Set++                    */
 /*                                                                  */
 /* Description : Header file for CICS sample AMQSCIC0               */
 /*   <copyright                                                     */
 /*   notice="lm-source-program"                                     */
 /*   pids="5724-H72,"                                               */
 /*   years="1994,2021"                                              */
 /*   crc="3297023590" >                                             */
 /*   Licensed Materials - Property of IBM                           */
 /*                                                                  */
 /*   5724-H72,                                                      */
 /*                                                                  */
 /*   (C) Copyright IBM Corp. 1994, 2021 All Rights Reserved.        */
 /*                                                                  */
 /*   US Government Users Restricted Rights - Use, duplication or    */
 /*   disclosure restricted by GSA ADP Schedule Contract with        */
 /*   IBM Corp.                                                      */
 /*   </copyright>                                                   */
 /*                                                                  */
 /********************************************************************/

#ifndef AMQSCIH0_DEFINED               /* File not yet included?      */
  #define AMQSCIH0_DEFINED             /* Show file now included      */

 #define TRUE 1
 #define FALSE 0

 #define WAIT_INTERVAL          30000

 #define APPL_NAME              "AMQSCIC0"

 /*********************************************************************/
 /* Error messages                                                    */
 /*********************************************************************/

 #define ERROR_MSG_1            "An error has occurred in "            \
                                "transaction %4.4s Task no. %07ld "    \
                                "on %8.8s %8.8s"

 #define ERROR_MSG_2            "Error - Operation %-12.12s  "         \
                                "CompCode %1.1d  Reason %4.4d  "       \
                                "Object %-48.48s"

 /*********************************************************************/
 /* Queue Names                                                       */
 /*********************************************************************/

 #define DEAD_QNAME             "SYSTEM.SAMPLE.CICS.DLQ"

 #define WORK_QNAME             "SYSTEM.SAMPLE.CICS.WORKQUEUE"

 /*********************************************************************/
 /* Declaration of input messages                                     */
 /*********************************************************************/
 typedef struct tagAMQSCIC0_INPUT {
    MQXQH XmitHeader;
    char  MessageData[2000];
 } AMQSCIC0_INPUT;

 /*********************************************************************/
 /* Declaration of dead letter queue messages                         */
 /*********************************************************************/
 typedef struct tagAMQSCIC0_DLQ {
    MQDLH DeadLetterHeader;
    char  MessageData[2000];
 } AMQSCIC0_DLQ;

 /*********************************************************************/
 /* Define min() macro                                                */
 /*********************************************************************/
#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#endif                                         /* End of header file */

