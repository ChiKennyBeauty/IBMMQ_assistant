/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/amqsauth.h */
 /********************************************************************/
 /*                                                                  */
 /* Module name: AMQSAUTH                                            */
 /*                                                                  */
 /* Environment : CICS/2 Version 2.0; IBM C Set++                    */
 /*                                                                  */
 /* Description : Header file for adding authentication to samples   */
 /*               using funcs defined in amqsauth source file        */
 /*   <copyright                                                     */
 /*   notice="lm-source-program"                                     */
 /*   pids="5724-H72,"                                               */
 /*   years="1994,2024"                                              */
 /*   crc="3297023590" >                                             */
 /*   Licensed Materials - Property of IBM                           */
 /*                                                                  */
 /*   5724-H72,                                                      */
 /*                                                                  */
 /*   (C) Copyright IBM Corp. 1994, 2024 All Rights Reserved.        */
 /*                                                                  */
 /*   US Government Users Restricted Rights - Use, duplication or    */
 /*   disclosure restricted by GSA ADP Schedule Contract with        */
 /*   IBM Corp.                                                      */
 /*   </copyright>                                                   */
 /*                                                                  */
 /********************************************************************/

 void getAuthInfo(MQCNO *cno, MQCSP *csp);

 void get_password(char *buffer, size_t size);