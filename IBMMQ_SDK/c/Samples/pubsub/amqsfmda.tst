**********************************************************************
*                                                                    *
* Program name: AMQSFDMA                                             *
*                                                                    *
* Description:  Sample MQSC source, defining MQSeries Publish/       *
*               Subscribe SYSTEM.BROKER.MODEL.STREAM.                *
* <copyright                                                         *
* notice="lm-source-program"                                         *
* pids=""                                                            *
* years="1994,2000"                                                  *
* crc="2912891938" >                                                 *
* Licensed Materials - Property of IBM                               *
*                                                                    *
*                                                                    *
*                                                                    *
* (C) Copyright IBM Corp. 1994, 2000 All Rights Reserved.            *
*                                                                    *
* US Government Users Restricted Rights - Use, duplication or        *
* disclosure restricted by GSA ADP Schedule Contract with            *
* IBM Corp.                                                          *
* </copyright>                                                       *
*                                                                    *
**********************************************************************
* Function:                                                          *
*                                                                    *
*   AMQSFDMA is a sample MQSC file to create or reset the MQSeries   *
*   Publish/Subscribe SYSTEM.BROKER.MODEL.STREAM.                    *
*                                                                    *
*   This file, or a similar one, can be processed when the MQM       *
*   is started - it creates the objects if missing, or resets        *
*   their attributes to the prescribed values.                       *
*                                                                    *
**********************************************************************
**********************************************************************
*                                                                    *
*   AMQSFDMA is sample data for the MQSC utility                     *
*                                                                    *
**********************************************************************

def qmodel(SYSTEM.BROKER.MODEL.STREAM) replace +
    deftype(permdyn) +
    noshare +
    defsopt(excl)
