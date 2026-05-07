/*@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/cpp/imqsget.cpp*/
// Description:   Sample C++ program that gets messages from a named
//                queue.
//
//                IMQSGET has 3 parameters:
//                - the name of a queue (required)
//                - the name of a queue manager (optional)
//                - the definition of a channel (optional)
//
//                If no queue manager name is given, the queue manager
//                defaults to the default local queue manager. If a
//                channel is defined, it should have the same format
//                as the MQSERVER environment variable.
//    <copyright
//    notice="lm-source-program"
//    pids="5724-H72,"
//    years="1994,2016"
//    crc="1784303197" >
//    Licensed Materials - Property of IBM
//
//    5724-H72,
//
//    (C) Copyright IBM Corp. 1994, 2016 All Rights Reserved.
//
//    US Government Users Restricted Rights - Use, duplication or
//    disclosure restricted by GSA ADP Schedule Contract with
//    IBM Corp.
//    </copyright>

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#include <imqi.hpp> // MQI

int main ( int argc, char * * argv ) {
  ImqQueueManager mgr;             // Queue manager
  ImqQueue queue;                  // Queue
  ImqMessage msg;                  // Data message
  ImqGetMessageOptions gmo;        // Get message options
  char buffer[ 101 ];              // Message buffer
  ImqChannel * pchannel = 0 ;      // Channel definition

  printf( "Sample IMQSGET start\n" );
  if ( argc < 2 ) {
    printf( "Required parameter missing - queue name\n" );
    exit( 99 );
  }

  // Create object descriptor for subject queue
  queue.setName( argv[ 1 ] );
  if ( argc > 2 ) {
    mgr.setName( argv[ 2 ] );
  }

  // Define a channel for client communication.
  if ( argc > 3 ) {
    ImqString strParse( argv[ 3 ] );
    ImqString strToken ;

    pchannel = new ImqChannel ;
    pchannel -> setHeartBeatInterval( 1 );

    // Break down the channel definition,
    // which is of the form "channel-name/transport-type/connection-name".
    if ( strParse.cutOut( strToken, '/' ) ) {
      pchannel -> setChannelName( strToken );

      if ( strParse.cutOut( strToken, '/' ) ) {

        // Interpret the transport type.
        if ( strToken.upperCase( ) == (ImqString)"LU62" ) {
          pchannel -> setTransportType( MQXPT_LU62 );
        }
        if ( strToken.upperCase( ) == (ImqString)"NETBIOS" ) {
          pchannel -> setTransportType( MQXPT_NETBIOS );
        }
        if ( strToken.upperCase( ) == (ImqString)"SPX" ) {
          pchannel -> setTransportType( MQXPT_SPX );
        }
        if ( strToken.upperCase( ) == (ImqString)"TCP" ) {
          pchannel -> setTransportType( MQXPT_TCP );
        }

        // Establish the connection name.
        if ( strParse.cutOut( strToken ) ) {
          pchannel -> setConnectionName( strToken );
        }
      }
    }

    mgr.setChannelReference( pchannel );
  }

  // Connect to queue manager
  if ( ! mgr.connect( ) ) {

    /* stop if it failed     */
    printf( "ImqQueueManager::connect failed with reason code %ld\n",
            (long)mgr.reasonCode( ) );
    exit( (int)mgr.reasonCode( ) );
  }

  // Associate queue with queue manager.
  queue.setConnectionReference( mgr );

  // Open the named message queue for input; exclusive or shared
  // use of the queue is controlled by the queue definition here
  queue.setOpenOptions(
                   MQOO_INPUT_AS_Q_DEF /* open queue for input      */
                 + MQOO_FAIL_IF_QUIESCING
                             );        /* but not if MQM stopping   */
  queue.open( );

  /* report reason, if any; stop if failed      */
  if ( queue.reasonCode( ) ) {
    printf( "ImqQueue::open ended with reason code %ld\n",
            (long)queue.reasonCode( ) );
  }

  if ( queue.completionCode( ) == MQCC_FAILED ) {
    printf( "unable to open queue for input\n" );
  }

  // Get messages from the message queue
  // Loop until there is a failure
  msg.useEmptyBuffer( buffer, sizeof( buffer ) - 1 );
                                 /* buffer size available for GET   */
  gmo.setOptions( MQGMO_WAIT |   /* wait for new messages           */
                  MQGMO_FAIL_IF_QUIESCING );
  gmo.setWaitInterval( 15000 );  /* 15 second limit for waiting     */

  while ( queue.completionCode( ) != MQCC_FAILED ) {

    // In order to read the messages in sequence, MsgId and
    // CorrelID must have the default value.  MQGET sets them
    // to the values in for message it returns, so re-initialise
    // them before every call
    msg.setMessageId( );
    msg.setCorrelationId( );

    if ( queue.get( msg, gmo ) ) {

      // Display each message received
      if ( msg.formatIs( MQFMT_STRING ) ) {

        buffer[ msg.dataLength( ) ] = 0 ;  /* add terminator          */
        printf( "message <%s>\n", msg.bufferPointer( ) );

      } else {
        printf( "Non-text message\n" );
      }

    } else {

      /* report reason, if any     */
      if ( queue.reasonCode( ) == MQRC_NO_MSG_AVAILABLE ) {
                                /* special report for normal end    */
        printf( "no more messages\n" );
      } else {
                                /* general report for other reasons */
        printf( "ImqQueue::get ended with reason code %ld\n",
                (long)queue.reasonCode( ) );

        /*   treat truncated message as a failure for this sample   */
        if ( queue.reasonCode( ) == MQRC_TRUNCATED_MSG_FAILED ) {
          break ;
        }
      }
    }

  }

  // Close the source queue (if it was opened)
  if ( ! queue.close( ) ) {

    /* report reason, if any     */
    printf( "ImqQueue::close failed with reason code %ld\n",
            (long)queue.reasonCode( ) );
  }

  // Disconnect from MQM if not already connected (the
  // ImqQueueManager object handles this situation automatically)
  if ( ! mgr.disconnect( ) ) {

    /* report reason, if any     */
    printf( "ImqQueueManager::disconnect failed with reason code %ld\n",
            (long)mgr.reasonCode( ) );
  }

  // Tidy up the channel object if allocated.
  if ( pchannel ) {
    mgr.setChannelReference( );
    delete pchannel ;
  }

  printf( "Sample IMQSGET end\n" );
  return( 0 );
}
