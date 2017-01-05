

/*
 * File:   IncomingMsgTester.cpp
 * Author: jcavalie
 *
 * Created on Dec 11, 2016, 11:37:43 PM
 */
//
#include "IncomingMsgTester.hpp"
#include "../node_controller.hpp"
#include "../message_buffer.hpp"
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>


nodeController::MessageBuffer<1, MSG_BUFF_SIZE> to_incominghandler_buff;

nodeController::MessageBuffer<1, MSG_BUFF_SIZE> to_remotesender_buff;

nodeController::MessageBuffer<2 * NUMFLAVORS, MSG_BUFF_SIZE> to_rqstqueue_buff;

#define MYPORT 34533
#define LISTENSOCK "/tmp/unitest_localmonitor.sock"




CPPUNIT_TEST_SUITE_REGISTRATION( IncomingMsgTester );

void IncomingMsgTester::run_local_comm( )
{

    using namespace std;
    using namespace clientUtils;
    using nodeController::local_process_monitor;
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

    l_monitor_thread = thread( local_process_monitor, l_arg );

    constexpr int NUM_MSGS = 4;
    clientUtils::Message out_msgs[NUM_MSGS];
    printf( "localMonitorTest--initializing msgs to send to monitor thread\n" );
    fflush( stdout );


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

    //    for ( int itr = 0; itr < NUM_MSGS; itr++ ) {
    //
    //        cerr << "LOCAL original msg:\n" << out_msgs[itr] << endl;
    //        cerr << "LOCAL consumed msg:\n" << to_incominghandler_buff.consume_msg( 0 ) << endl;
    //
    //    }
    //
    //    if ( true /*check result*/ ) {
    //        CPPUNIT_ASSERT( true );
    //    }


}

void IncomingMsgTester::run_remote_comm( )
{
    using namespace std;
    using namespace nodeController::remoteMonitor;
    using nodeController::host2network_msg;
    using nodeController::remote_peer_monitor;

    //array of msgs that I will send over socket then after
    //remote_peer_monitor thread produces each msg into the buffer,
    //I will consume each and compare to original msgs stored in this array.
    constexpr int NUM_MSGS = 4;
    nodeController::Message out_msgs[NUM_MSGS];

    printf( "remoteMonitorTest--initializing msgs to send to monitor thread\n" );
    fflush( stdout );


    struct sockaddr_in cliaddr, myaddr = { };
    socklen_t cliaddr_len = sizeof (cliaddr );

    Socket listen_socket( AF_INET, SOCK_STREAM );

    int optval = 1;
    if ( setsockopt( listen_socket.get_channel( ),
                     SOL_SOCKET, SO_REUSEADDR, &optval,
                     sizeof (optval ) ) == -1 ) {

    }

    myaddr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons( MYPORT );

    printf( "remoteMonitorTest--binding my socket to  %d:%d\n",
            INADDR_LOOPBACK, MYPORT );
    fflush( stdout );

    if ( bind( listen_socket.get_channel( ),
               (struct sockaddr*) &myaddr, sizeof (myaddr ) ) < 0 ) {
        perror( "remoteMonitorTest--bind socket: " );
        exit( EXIT_FAILURE );
    }

    listen( listen_socket.get_channel( ), 6 );

    printf( "remoteMonitorTest--starting monitor thread\n" );
    fflush( stdout );
    r_monitor_thread = thread( remote_peer_monitor, r_arg );

    Socket accept_socket( accept( listen_socket.get_channel( ),
                                  (struct sockaddr*) &cliaddr,
                                  &cliaddr_len ) );

    printf( "remoteMonitorTest--just accepted connection from monitor thread\n" );
    fflush( stdout );
    for ( int msgcount = 0; msgcount < NUM_MSGS; msgcount++ ) {
        out_msgs[msgcount].act = ( msgcount % 2 ) ? Action::CONSUME : Action::PRODUCE;
        out_msgs[msgcount].sub_act = static_cast<subAction> ( msgcount % 4 );
        out_msgs[msgcount].node_ID = 1;
        out_msgs[msgcount].type = ( msgcount < 5 ) ?
                nodeController::MESSAGE_T::REPLY :
                nodeController::MESSAGE_T::REQUEST;

        nodeClock( out_msgs[msgcount].time_stamp );
        nodeController::update_rcvd_replies_array( 3, out_msgs[msgcount].rcvdReplies );

        char buffer[sizeof (out_msgs[0] )] = { 0 };

        host2network_msg( out_msgs[msgcount], buffer );


        if ( write( accept_socket.get_channel( ),
                    buffer, sizeof (buffer ) ) <
             sizeof (buffer ) ) {

            perror( "remoteMonitorTest--socket write failed: " );
            exit( EXIT_FAILURE );

        }

    }

    //    for ( int itr = 0; itr < NUM_MSGS; itr++ ) {
    //
    //        cerr << "REMOTE original msg:\n" << out_msgs[itr] << endl;
    //        cerr << "REMOTE consumed msg:\n" << to_incominghandler_buff.consume_msg( 0 ) << endl;
    //
    //    }

    //CPPUNIT_ASSERT(result);


    return;



}

using nodeController::incoming_msg_handler;

IncomingMsgTester::IncomingMsgTester( ) : listen_sock( AF_LOCAL, SOCK_DGRAM ),
    accInfo( new nodeController::localMonitor::accept_info( listen_sock ) ),
    l_arg( new nodeController::localMonitor::thread_arg{ accInfo, &to_incominghandler_buff } ){ }

IncomingMsgTester::~IncomingMsgTester( ){ }

void IncomingMsgTester::setUp( )
{
    arg = new nodeController::incomingMsgHandler::thread_arg{ &to_remotesender_buff,
                                                             &to_incominghandler_buff,
                                                             &to_rqstqueue_buff };

    unlink( LISTENSOCK );
    connInfo = new nodeController::remoteMonitor::connect_info{ "127.0.0.1", //IP address of server
                                                               MYPORT //port number of server
    };

    r_arg = new nodeController::remoteMonitor::thread_arg{
                                                          3, //peer ID--ID of host that is being monitored
                                                          nodeController::remoteMonitor::comm_actions::CONNECT,
                                                          connInfo,
                                                          NULL, //accept data not needed
                                                          &to_incominghandler_buff
    };


}

void IncomingMsgTester::tearDown( ){
    //    delete arg;
    //    delete accInfo, l_arg;
    //    delete connInfo, r_arg;
}

void IncomingMsgTester::testIncoming_msg_handler( )
{
    using namespace std;



    run_remote_comm( );

    run_local_comm( );

    handler_thread = thread( incoming_msg_handler, arg );

    sleep( 3 );
    printf( "\n\n\nincomingMsgTester--printing msgs from outgoing buffer\n" );

    for ( int itr = 0; itr < 4; itr++ ) {
        cerr << "OUTGOING MSG:\n" << to_remotesender_buff.consume_msg( 0 ) << endl;
    }

    printf( "incomingMsgTester--printing msgs from rqst queue buffer\n" );
    for ( int itr = 0; itr < 2 * NUMFLAVORS; itr++ ) {
        int bufsize = to_rqstqueue_buff.get_size( itr );
        cerr << "RQST QUEUE BUFFERSIZE: " << bufsize << endl;
        for ( int itr2 = 0; itr2 < bufsize; itr2++ ) {
            cerr << "RQST QUEUE MSG:\n" << to_rqstqueue_buff.consume_msg( itr ) << endl;
        }
    }






    if ( true /*check result*/ ) {
        CPPUNIT_ASSERT( true );
    }
}
