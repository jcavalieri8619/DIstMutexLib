/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   remoteMonitorTestClass.cpp
 * Author: jcavalie
 *
 * Created on Dec 10, 2016, 3:52:12 PM
 */
//
#include "remoteMonitorTestClass.hpp"
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>

CPPUNIT_TEST_SUITE_REGISTRATION( remoteMonitorTestClass );
#define MYPORT 34533

using nodeController::host2network_msg;
using nodeController::remote_peer_monitor;
using namespace nodeController::remoteMonitor;

nodeController::MessageBuffer<1, MSG_BUFF_SIZE> to_incominghandler_buff;

remoteMonitorTestClass::remoteMonitorTestClass( ){ }

remoteMonitorTestClass::~remoteMonitorTestClass( ){ }

void remoteMonitorTestClass::setUp( )
{

    connInfo = new connect_info{ "127.0.0.1", //IP address of server
                                MYPORT //port number of server
    };

    arg = new thread_arg{
                         3, //peer ID--ID of host that is being monitored
                         comm_actions::CONNECT,
                         connInfo,
                         NULL, //accept data not needed
                         &to_incominghandler_buff
    };



}

void remoteMonitorTestClass::tearDown( )
{
    delete connInfo, arg;
}

void remoteMonitorTestClass::testRemote_peer_monitor( )
{
    //this test function is mimicing NODE 3 remote sender
    //and sending NODE 1's remote monitor
    using namespace std;

    //array of msgs that I will send over socket then after
    //remote_peer_monitor thread produces each msg into the buffer,
    //I will consume each and compare to original msgs stored in this array.
    constexpr int NUM_MSGS = 10;
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
    monitor_thread = thread( remote_peer_monitor, arg );

    Socket accept_socket( accept( listen_socket.get_channel( ),
                                  (struct sockaddr*) &cliaddr,
                                  &cliaddr_len ) );

    printf( "remoteMonitorTest--just accepted connection from monitor thread\n" );
    fflush( stdout );
    for ( int msgcount = 0; msgcount < NUM_MSGS; msgcount++ ) {

        out_msgs[msgcount].act = ( msgcount % 2 ) ? Action::CONSUME : Action::PRODUCE;
        out_msgs[msgcount].sub_act = static_cast<subAction> ( msgcount % 4 );
        out_msgs[msgcount].node_ID = 1;
        out_msgs[msgcount].type = ( msgcount % 3 ) ?
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


    printf( "remoteMonitorTest--comparing original msgs to msgs "
            "processed by remote monitor thread\n" );
    fflush( stdout );


    for ( int msgcount = 0; msgcount < NUM_MSGS; msgcount++ ) {

        cerr << "original msg:\n" << out_msgs[msgcount] << endl;
        cerr << "consumed msg:\n" << to_incominghandler_buff.consume_msg( 0 ) << endl;


    }



    //CPPUNIT_ASSERT(false);


    return;
}
