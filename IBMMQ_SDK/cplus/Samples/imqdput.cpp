/*@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/cpp/imqdput.cpp*/
// Description:   Sample C++ program that puts messages to a distribution
//                list containing two queues.
//
//                IMQDPUT has 4 parameters:
//                - the name of a queue (required)
//                - the name of another queue (required)
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
//    crc="926615340" >
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
}

#include <imqi.hpp> // MQI

int main ( int argc, char * * argv ) {
  FILE * fp ;                      // Console
  ImqQueueManager mgr;             // Queue manager
  ImqQueue queueA;                 // Queue
  ImqQueue queueB;                 // Another queue
  ImqMessage msg;                  // Data message
  int      buflen;                 // Buffer length
  char     buffer[100];            // Message buffer
  ImqDistributionList dlist;       // Distribution list
  ImqChannel * pchannel = 0 ;      // Channel definition

  printf( "Sample IMQDPUT start\n" );

  // Explain usage.
  if ( argc < 3 ) {
    printf( "Usage : %s queue1 queue2 [queue-manager [channel-definition]]\n",
            argv[0] );
    printf( "Missing parameter.\n" );
    return ( 99 );
  }

  // Define a channel for client communication.
  if ( argc > 4 ) {
    ImqString strParse( argv[ 4 ] );
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
  if ( argc > 2 ) {
    mgr.setName( argv[3] );
  }

  if ( ! mgr.connect( ) ) {

    /* stop if it failed */
    printf( "ImqQueueManager::connect ended with reason code %ld\n",
            (long)mgr.reasonCode( ) );
    exit ( (int)mgr.reasonCode( ) );
  }

  // Connection successful. Obtain queue manager's name.
  ImqString strManagerName( mgr.name( ) );
  printf("Successful connection to queue manager %s\n",
                                        (char *)strManagerName );

  // Associate distribution list with queue manager.
  dlist.setConnectionReference( mgr );

  // Associate queues with queue manager.
  queueA.setConnectionReference( mgr );
  queueB.setConnectionReference( mgr );

  // Use parameters to name target queues.
  queueA.setName( argv[ 1 ] );
  queueB.setName( argv[ 2 ] );
  printf( "First target queue is %s\n", (char *)queueA.name( ) );
  printf( "Second target queue is %s\n", (char *)queueB.name( ) );

  queueA.setQueueManagerName( (char *)strManagerName );
  queueB.setQueueManagerName( (char *)strManagerName );
  printf( "First target queue has queue manager %s\n",
          (char *)queueA.queueManagerName( ) );
  printf( "Second target queue has queue manager %s\n",
          (char *)queueB.queueManagerName( ) );

  // Associate the queues with the distribution list.
  queueA.setDistributionListReference( dlist );
  queueB.setDistributionListReference( dlist );

  // Open the distribution list for output.
  dlist.setOpenOptions( MQOO_OUTPUT /* open queue for output        */
    + MQOO_FAIL_IF_QUIESCING );     /* but not if MQM stopping      */
  dlist.open( );

  /* report reason, if any; stop if failed      */
  if ( dlist.reasonCode( ) ) {
    printf( "ImqDst::open ended with reason code %ld\n",
            (long)dlist.reasonCode( ) );

    printf( "First target queue has reason code %ld\n",
            (long)queueA.reasonCode( ) );
    printf( "Second target queue has reason code %ld\n",
            (long)queueB.reasonCode( ) );
  }

  if ( dlist.completionCode( ) == MQCC_FAILED ) {
    printf( "unable to open distributon list for output\n" );
  }

  // Read lines from the stdin and put them to the distribution
  // list. Repeat until null line entered or failure occurs.
  fp = stdin;
  msg.useEmptyBuffer( buffer, sizeof( buffer ) );
  msg.setFormat( MQFMT_STRING );      /* character string format    */

  while ( dlist.completionCode( ) != MQCC_FAILED ) {

    if ( fgets( buffer, (int)sizeof(buffer) - 1, fp ) ) {
      buflen = (int)strlen( buffer ) - 1 ; /* length w/o line-end   */
      buffer[ buflen ] = '\0' ;            /* add line-end          */
    } else {
      buflen = 0;           /* treat EOF same as null line          */
    }

    // Put each buffer to the distribution list.
    if ( buflen > 0 ) {
      msg.setMessageLength( buflen );
      if ( ! dlist.put( msg ) ) {

        /* report reason, if any */
        printf( "ImqDistributionList::put ended with reason code %ld\n",
                (long)dlist.reasonCode( ) );
      }

    } else {
      /* quit loop when empty line is read */
      break ;
    }
  }

  // Close the target distribution list (if it was opened).
  if ( ! dlist.close( ) ) {

    /* report reason, if any     */
    printf( "ImqDistributionList::close ended with reason code %ld\n",
            (long)dlist.reasonCode( ) );
  }

  // Disconnect from MQM if not already connected (the
  // ImqQueueManager object handles this situation automatically)
  if ( ! mgr.disconnect( ) ) {

    /* report reason, if any     */
    printf( "ImqQueueManager::disconnect ended with reason code %ld\n",
            (long)mgr.reasonCode( ) );
  }

  // Tidy up the channel object if allocated.
  if ( pchannel ) {
    mgr.setChannelReference( );
    delete pchannel ;
  }

  printf( "Sample IMQDPUT end\n" );
  return( 0 );
}
