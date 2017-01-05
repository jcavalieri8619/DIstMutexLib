
/*
 * File:   localMonitorTester.cpp
 * Author: jcavalie
 *
 * Created on Dec 11, 2016, 12:40:06 AM
 */
//
#include "localMonitorTester.hpp"
#include "../node_controller.hpp"
#include "../client_utils.hpp"
#include "../globals.hpp"
#include "../message_buffer.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <iostream>

CPPUNIT_TEST_SUITE_REGISTRATION( localMonitorTester );
#define LISTENSOCK "/tmp/unitest_localmonitor.sock"


nodeController::MessageBuffer<1, MSG_BUFF_SIZE> to_incominghandler_buff;

using namespace std;
using namespace clientUtils;
using nodeController::local_process_monitor;

localMonitorTester::localMonitorTester( ) : listen_sock( AF_LOCAL, SOCK_DGRAM ),
    accInfo( new nodeController::localMonitor::accept_info( listen_sock ) ),
    arg( new nodeController::localMonitor::thread_arg{ accInfo, &to_incominghandler_buff } ){ }

localMonitorTester::~localMonitorTester( )
{
    delete accInfo, arg;

}

void localMonitorTester::setUp( )
{
    unlink( LISTENSOCK );


}

void localMonitorTester::tearDown( ){ }

void localMonitorTester::testLocal_process_monitor( )
{

    struct sockaddr_un cliaddr = { };
    struct sockaddr_un servaddr = { };

    //socket created as "listening socket" to pass into local monitor
    servaddr.sun_family = AF_LOCAL;
    strncpy( servaddr.sun_path, LISTENSOCK, sizeof (servaddr.sun_path ) - 1 );

    if ( bind( listen_sock.get_channel( ),
               (struct sockaddr*) &servaddr, sizeof (servaddr ) ) < 0 ) {
        perror( "localMonitorTest--bind socket: " );
        exit( EXIT_FAILURE );
    }



    //socket created for me so that server can local monitor can return ACKS
    Socket mysock( AF_LOCAL, SOCK_DGRAM );
    cliaddr.sun_family = AF_LOCAL;
    strncpy( cliaddr.sun_path, tmpnam( NULL ), sizeof (cliaddr.sun_path ) - 1 );
    if ( bind( mysock.get_channel( ),
               (struct sockaddr*) &cliaddr, sizeof (cliaddr ) ) < 0 ) {
        perror( "localMonitorTest--bind socket: " );
        exit( EXIT_FAILURE );
    }

    monitor_thread = thread( local_process_monitor, arg );

    constexpr int NUM_MSGS = 10;
    clientUtils::Message out_msgs[NUM_MSGS];

    for ( int itr = 0; itr < NUM_MSGS; itr++ ) {
        out_msgs[itr].act = ( itr % 2 ) ? Action::CONSUME : Action::PRODUCE;
        out_msgs[itr].localPID = getpid( );
        out_msgs[itr].subact = static_cast<subAction> ( itr % 4 );
        out_msgs[itr].type = clientUtils::MESSAGE_T::LOCAL_REQUEST;


        char buffer[sizeof (out_msgs[0] )] = { 0 };

        host2network_msg( out_msgs[itr], buffer );

        if ( sendto( mysock.get_channel( ), buffer, sizeof (buffer ), 0,
                     (struct sockaddr*) &servaddr, sizeof (servaddr ) ) < 0 ) {
            perror( "localMonitorTest--sento: " );
            exit( EXIT_FAILURE );
        }

    }

    for ( int itr = 0; itr < NUM_MSGS; itr++ ) {

        cerr << "original msg:\n" << out_msgs[itr] << endl;
        cerr << "consumed msg:\n" << to_incominghandler_buff.consume_msg( 0 ) << endl;

    }

    if ( false /*check result*/ ) {
        CPPUNIT_ASSERT( false );
    }
}

