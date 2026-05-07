/*@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/cpp/imqwrld.cpp*/
// Description:   Sample C++ program that puts and gets a message
//                to a queue.
//
//                IMQWRLD has 3 parameters:
//                - the name of a queue (optional)
//                  e.g. SYSTEM.DEFAULT.LOCAL.QUEUE or
//                       SYSTEM.DEFAULT.MODEL.QUEUE
//                - the name of a queue manager (optional)
//                - a channel definition (optional)
//                  e.g. SYSTEM.DEF.SVRCONN/TCP/hostname(1414)
//
//                If no queue name is given, the name defaults to
//                SYSTEM.DEFAULT.LOCAL.QUEUE.
//
//                If no queue manager name is given, the queue manager
//                defaults to the default local queue manager.
//    <copyright
//    notice="lm-source-program"
//    pids="5724-H72,"
//    years="1994,2021"
//    crc="2074802474" >
//    Licensed Materials - Property of IBM
//
//    5724-H72,
//
//    (C) Copyright IBM Corp. 1994, 2021 All Rights Reserved.
//
//    US Government Users Restricted Rights - Use, duplication or
//    disclosure restricted by GSA ADP Schedule Contract with
//    IBM Corp.
//    </copyright>

extern "C" {
#include <stdio.h>
}

#include <imqi.hpp> // MQI

#define EXISTING_QUEUE "SYSTEM.DEFAULT.LOCAL.QUEUE"

#define BUFFER_SIZE 12

static char gpszHello[ BUFFER_SIZE ] = "Hello world" ;

int main ( int argc, char * * argv ) {
  ImqQueueManager manager ;
  ImqChannel * pchannel = 0 ;
  int iReturnCode = 0 ;

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

    manager.setChannelReference( pchannel );
  }

  // Connect to the queue manager.
  if ( argc > 2 ) {
    manager.setName( argv[ 2 ] );
  }
  if ( manager.connect( ) ) {
    ImqQueue * pqueue = new ImqQueue ;
    ImqMessage * pmsg = new ImqMessage ;

    // Identify the queue which will hold the message.
    pqueue -> setConnectionReference( manager );
    if ( argc > 1 ) {
      pqueue -> setName( argv[ 1 ] );

      // The named queue may be a model queue, which will result in the
      // creation of a temporary dynamic queue, which will be destroyed as
      // soon as it is closed. Therefore we must ensure that such a queue
      // is not automatically closed and re-opened. We do this by setting
      // open options which will avoid the need for closure and re-opening.
      pqueue -> setOpenOptions( MQOO_OUTPUT | MQOO_INPUT_SHARED |
                                MQOO_INQUIRE );

    } else {
      pqueue -> setName( EXISTING_QUEUE );

      // The existing queue is not a model queue, and will not be destroyed
      // by automatic closure and re-opening. Therefore we will let the
      // open options be selected on an as-needed basis. The queue will be
      // opened with an output option for the "put", and then closed and
      // re-opened with the addition of an input option for the "get".
    }

    // Prepare a message containing the text "Hello world".
    pmsg -> setFormat( MQFMT_STRING );
    pmsg -> useFullBuffer( gpszHello , BUFFER_SIZE );

    // Place the message on the queue, using default put message options.
    // The queue will be automatically opened with an output option.
    if ( pqueue -> put( * pmsg ) ) {
      ImqString strQueue( pqueue -> name( ) );

      // Discover the queue manager's name.
      ImqString strQueueManagerName( manager.name( ) );
      printf( "The queue manager name is %s.\n", (char *)strQueueManagerName );

      // Show the name of the queue.
      printf( "Message sent to %s.\n", (char *)strQueue );

      // Retrieve the data message just sent ("Hello world" expected) from the
      // queue, using default get message options. The queue will automatically
      // be closed and re-opened with an input option if it is not already open
      // with an input option. We will get the message just sent, rather than
      // any other message on the queue, because the "put" will have set the
      // id of the message in the message object, so, as we are using the
      // same message object, the message id acts as a filter which says that
      // we are only interested in a message if it has this particular id.
      if ( pqueue -> get( * pmsg ) ) {
        int iDataLength = (int)pmsg -> dataLength( );

        // Show the text of the received message.
        printf( "Message of length %d received, ", iDataLength );

        if ( pmsg -> formatIs( MQFMT_STRING ) ) {
          char * pszText = pmsg -> bufferPointer( );

          // If the last character of data is a null, then we can assume that
          // the data can be interpreted as a text string.
          if ( ! pszText[ iDataLength - 1 ] ) {
            printf( "text is \"%s\".\n", pszText );
          } else {
            printf( "no text.\n" );
          }

        } else {
          printf( "non-text message.\n" );
        }

      } else {
        printf( "ImqQueue::get failed with reason code %d\n",
                pqueue -> reasonCode( ) );
        iReturnCode = (int)pqueue -> reasonCode( );
      }

    } else {
      printf( "ImqQueue::open/put failed with reason code %d\n",
              pqueue -> reasonCode( ) );
      iReturnCode = (int)pqueue -> reasonCode( );
    }

    // Deletion of the queue will ensure that it is closed.
    // If the queue is dynamic then it will also be destroyed.
    delete pqueue ;
    delete pmsg ;

  } else {
    printf( "ImqQueueManager::connect failed with reason code %d\n",
            manager.reasonCode( ) );
    iReturnCode = (int)manager.reasonCode( );
  }

  // Tidy up the channel object if allocated.
  if ( pchannel ) {
    manager.setChannelReference( );
    delete pchannel ;
  }

  // Destruction of the queue manager will ensure that it is disconnected.
  // If the queue object were still available and open (which it is not),
  // then the queue would be closed prior to disconnection.

  return iReturnCode ;
}
