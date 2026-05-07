**********************************************************************
*                                                                    *
* Program name: amqsgama.tst                                         *
*                                                                    *
* Description:  Sample MQSC source, defining MQSeries Publish/       *
*               Subscribe SAMPLE.BROKER.RESULTS.STREAM               *
* <copyright                                                         *
* notice="lm-source-program"                                         *
* pids=""                                                            *
* years="1994,2000"                                                  *
* crc="3193427172" >                                                 *
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
*   This file is to be read by runmqsc on the MQSeries queue         *
*   manager running the Publish/Subscribe broker that is to          *
*   be used by the sample amqsgam (Match Simulator) only. If         *
*   the same queue manager is to be used for both samples,           *
*   amqsres and amqsgam, only amqsresa.tst is required.              *
*                                                                    *
* Example:                                                           *
*                                                                    *
*   runmqsc QueueManager < amqsgama.tst                              *
*                                                                    *
**********************************************************************
**********************************************************************
*                                                                    *
*   AMQSGAMA is sample data for the MQSC utility                     *
*                                                                    *
**********************************************************************

**********************************************************************
* Results service sample stream                                      *
**********************************************************************
def qlocal('SAMPLE.BROKER.RESULTS.STREAM') replace +
    noshare
