**********************************************************************
*                                                                    *
* Program name: amqsresa.tst                                         *
*                                                                    *
* Description:  Sample MQSC source, defining MQSeries Publish/       *
*               Subscribe SAMPLE.BROKER.RESULTS.STREAM and           *
*               RESULTS.SERVICE.SAMPLE.QUEUE                         *
* <copyright                                                         *
* notice="lm-source-program"                                         *
* pids=""                                                            *
* years="1994,2000"                                                  *
* crc="3838198435" >                                                 *
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
*   be used by the sample amqsres (Results Service).                 *
*                                                                    *
* Example:                                                           *
*                                                                    *
*   runmqsc QueueManager < amqsresa.tst                              *
*                                                                    *
**********************************************************************
**********************************************************************
*                                                                    *
*   AMQSRESA is sample data for the MQSC utility                     *
*                                                                    *
**********************************************************************

**********************************************************************
* Results service sample stream                                      *
**********************************************************************
def qlocal('SAMPLE.BROKER.RESULTS.STREAM') replace +
    noshare

**********************************************************************
* Results service sample subscriber queue                            *
**********************************************************************
def qlocal('RESULTS.SERVICE.SAMPLE.QUEUE') replace
