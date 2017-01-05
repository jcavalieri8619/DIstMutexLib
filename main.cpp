

/*
 * File:   main.cpp
 * Author: jcavalie
 *
 * Created on November 2, 2016, 9:57 AM
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <stdexcept>
#include <pthread.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include "message_buffer.hpp"
#include <cstdlib>
#include <cstdio>
#include "client_utils.hpp"
#include "globals.hpp"
#include "node_controller.hpp"
#include <vector>
#include <signal.h>





using namespace nodeController;

/*
 *
 */
void * nodeController::request_queue_notifier( void* argument );

void * nodeController::request_queue_manager( void* argument );

void* nodeController::remote_peer_sender( void* argument );

void * nodeController::local_process_sender( void* argument );

void * nodeController::incoming_msg_handler( void* argument );

void * nodeController::outgoing_peerMsg_handler( void* argument );

//receives data from remote peers
void * nodeController::remote_peer_monitor( void * argument );

//receives data from local producer & consumers
void * nodeController::local_process_monitor( void* argument );

Socket* peer_socket_array[NODECOUNT] = { nullptr };

LogicalClock node_clock;


Ricart_RequestQueue RQST_Q( MSG_BUFF_SIZE );

unsigned MY_NODE_ID;

void sig_handler( int sig )
{
    using std::cerr;
    using std::endl;

    cerr << "took signal " << sig << ": " << strsignal( sig ) << endl <<
            "killing node controller" << endl;
    exit( EXIT_SUCCESS );

}

void *asynch_rqstQ_investigator( void *arg )
{
    using std::cerr;
    using std::endl;


    sigset_t sigterm_signal;
    int signo;

    sigemptyset( &sigterm_signal );
    sigaddset( &sigterm_signal, SIGTERM );
    sigaddset( &sigterm_signal, SIGINT );

    if ( sigwait( &sigterm_signal, & signo ) != 0 ) {
        cerr << "sigwait failed, exiting \n" << endl;
        exit( 2 );
    }
    if ( DO_DEBUG ) {
        cerr << "Number of queued elements in each request queue:" << endl;
        for ( int itr = 1; itr < 3; itr++ ) {
            for ( int itr2 = 0; itr2 < 4; itr2++ ) {
                cerr <<  static_cast<Action> ( itr ) << " " <<
                        static_cast<subAction> ( itr2 ) << ":\t" <<
                        RQST_Q.get_size( static_cast<Action> ( itr ),
                                         static_cast<subAction> ( itr2 ),true )
                        << endl;
            }

        }
        cerr << endl << endl << "writing current state of request queue to "
                "RqstQueue_contents.txt" << endl;
        char fileName[50] = { 0 };
        snprintf( fileName, 50, "RqstQueue_contents_NODE%d.txt", MY_NODE_ID );
        RQST_Q.print_queue_to_file( fileName );
    }

    exit( EXIT_SUCCESS );

    return nullptr;
}

int main( int argc, char** argv )
{





    using std::cerr;
    using std::endl;
    using std::vector;

    if ( argc < 2 ) {
        fprintf( stderr, "Usage: %s <node_ID>", argv[0] );
        exit( EXIT_FAILURE );
    }



    int sigs[] = { SIGBUS, SIGSEGV, SIGFPE };
    int nsigs = sizeof (sigs ) / sizeof (sigs[0] );
    struct sigaction new_act;
    sigset_t all_signals;

    sigfillset( &all_signals );
    for ( int i = 0; i < nsigs; i++ )
        sigdelset( &all_signals, sigs [i] );
    sigprocmask( SIG_BLOCK, &all_signals, NULL );
    sigfillset( &all_signals );
    for ( int i = 0; i < nsigs; i++ ) {
        new_act.sa_handler = sig_handler;
        new_act.sa_mask = all_signals;
        new_act.sa_flags = 0;
        if ( sigaction( sigs[i], &new_act, NULL ) == -1 ) {
            perror( "MAIN sigaction failed " );
            exit( 1 );
        }
    }

    pthread_t asyncRqstQueue_TID;
    pthread_attr_t asyncRqstQueue_attr;

    pthread_attr_init( &asyncRqstQueue_attr );
    pthread_create( &asyncRqstQueue_TID, &asyncRqstQueue_attr,
                    asynch_rqstQ_investigator, nullptr );

    MY_NODE_ID = atoi( argv[1] );

    const int NUMPEERS = NODECOUNT - 1;

    int remotePeer_IDs[NODECOUNT - 1];
    for ( int i = 0, j = 0; i < ( NODECOUNT ) && j < NUMPEERS; i++ ) {
        if ( i != MY_NODE_ID ) {
            remotePeer_IDs[j++] = i;
        }
        //now pass each remotePeer_ID to remoteComm_monitor threads
        //so they will know who they are talking to
    }

    vector<pthread_t*> ALL_WORKER_THREADS;
    pthread_t* worker_thread;




    ///////////////////////REMOTE MONITOR THREADS//////////////////////////
    pthread_barrier_t comm_barrier;
    if ( pthread_barrier_init( &comm_barrier, nullptr, NUMPEERS ) != 0 ) {
        perror( "MAIN remote monitor barrier init: " );
        exit( EXIT_FAILURE );
    }

    pthread_mutex_t rmonitor_accept_lock;
    if ( pthread_mutex_init( &rmonitor_accept_lock, nullptr ) != 0 ) {
        perror( "MAIN remote monitor mutex init: " );
        exit( EXIT_FAILURE );
    }

    Socket remote_listening_socket( AF_INET, SOCK_STREAM );

    int optval = 1;
    if ( setsockopt( remote_listening_socket.get_channel( ),
                     SOL_SOCKET, SO_REUSEADDR, &optval,
                     sizeof (optval ) ) == -1 ) {

    }

    struct sockaddr_in remote_listening_addr = { };
    remote_listening_addr.sin_family = AF_INET;
    remote_listening_addr.sin_addr.s_addr = htonl( INADDR_ANY );
    remote_listening_addr.sin_port = htons( atoi( Ports[MY_NODE_ID] ) );


    if ( bind( remote_listening_socket.get_channel( ),
               ( sockaddr* ) & remote_listening_addr,
               sizeof (sockaddr_in ) ) != 0 ) {

        perror( "MAIN remote monitor bind: " );
        exit( EXIT_FAILURE );

    }

    constexpr unsigned BACKLOG = 10;
    listen( remote_listening_socket.get_channel( ), BACKLOG );


    remoteMonitor::thread_arg remoteMonitor_args[NUMPEERS];

    MessageBuffer<remoteMonitor::thread_arg::NROWS,
            remoteMonitor::thread_arg::BUFFSIZE> to_incoming_handler;

    remoteMonitor::accept_info accInfo( remote_listening_socket,
                                        &rmonitor_accept_lock );


    remoteMonitor::connect_info connInfo[NUMPEERS];


    pthread_t remoteMonitorThreads[NUMPEERS];
    pthread_attr_t remoteMonitor_attrs[NUMPEERS];

    switch ( MY_NODE_ID ) {
        case 0:
        {

            for ( int itr = 0; itr < NUMPEERS; itr++ ) {

                remoteMonitor_args[itr].peerID = remotePeer_IDs[itr];
                remoteMonitor_args[itr].action = remoteMonitor::comm_actions::ACCEPT;
                remoteMonitor_args[itr].c_data = nullptr;
                remoteMonitor_args[itr].a_data = & accInfo;
                remoteMonitor_args[itr].comm_barrier = &comm_barrier;
                remoteMonitor_args[itr].out_buffer = &to_incoming_handler;

                if ( pthread_attr_init( &remoteMonitor_attrs[itr] ) != 0 ) {
                    perror( "MAIN remote monitor pthread_attr_init: " );
                    exit( EXIT_FAILURE );
                }

                if ( pthread_create( ( worker_thread = &remoteMonitorThreads[itr] ),
                                     &remoteMonitor_attrs[itr],
                                     remote_peer_monitor,
                                     &remoteMonitor_args[itr] ) ) {
                    perror( "MAIN remote monitor pthread_create: " );
                    exit( EXIT_FAILURE );
                }
                ALL_WORKER_THREADS.push_back( worker_thread );


            }

            break;
        }
        case 1:
        {
            for ( int itr = 0; itr < NUMPEERS; itr++ ) {

                connInfo[itr].IPaddr = Hosts[remotePeer_IDs[itr]];
                connInfo[itr].port = atoi( Ports[remotePeer_IDs[itr]] );
                remoteMonitor_args[itr].peerID = remotePeer_IDs[itr];
                remoteMonitor_args[itr].action = remoteMonitor::comm_actions::CONNECT;
                remoteMonitor_args[itr].c_data = &connInfo[itr];
                remoteMonitor_args[itr].a_data = nullptr;
                remoteMonitor_args[itr].comm_barrier = &comm_barrier;
                remoteMonitor_args[itr].out_buffer = &to_incoming_handler;

                if ( pthread_attr_init( &remoteMonitor_attrs[itr] ) != 0 ) {
                    perror( "MAIN remote monitor pthread_attr_init: " );
                    exit( EXIT_FAILURE );
                }

                if ( pthread_create( ( worker_thread = &remoteMonitorThreads[itr] ),
                                     &remoteMonitor_attrs[itr],
                                     remote_peer_monitor,
                                     &remoteMonitor_args[itr] ) ) {
                    perror( "MAIN remote monitor pthread_create: " );
                    exit( EXIT_FAILURE );
                }
                ALL_WORKER_THREADS.push_back( worker_thread );

            }

            break;
        }
        case 2:
        {

            //ASSUMING 3 PEERS for TOTAL OF 4 NODES
            remoteMonitor_args[0].peerID = remotePeer_IDs[1];
            remoteMonitor_args[0].action = remoteMonitor::comm_actions::ACCEPT;
            remoteMonitor_args[0].c_data = nullptr;
            remoteMonitor_args[0].a_data = & accInfo;
            remoteMonitor_args[0].comm_barrier = &comm_barrier;
            remoteMonitor_args[0].out_buffer = &to_incoming_handler;

            connInfo[1].IPaddr = Hosts[remotePeer_IDs[0]];
            connInfo[1].port = atoi( Ports[remotePeer_IDs[0]] );
            remoteMonitor_args[1].peerID = remotePeer_IDs[0];
            remoteMonitor_args[1].action = remoteMonitor::comm_actions::ACCEPT;
            remoteMonitor_args[1].c_data = &connInfo[1];
            remoteMonitor_args[1].a_data = nullptr;
            remoteMonitor_args[1].comm_barrier = &comm_barrier;
            remoteMonitor_args[1].out_buffer = &to_incoming_handler;

            connInfo[2].IPaddr = Hosts[remotePeer_IDs[3]];
            connInfo[2].port = atoi( Ports[remotePeer_IDs[3]] );
            remoteMonitor_args[2].peerID = remotePeer_IDs[3];
            remoteMonitor_args[2].action = remoteMonitor::comm_actions::ACCEPT;
            remoteMonitor_args[2].c_data = &connInfo[2];
            remoteMonitor_args[2].a_data = nullptr;
            remoteMonitor_args[2].comm_barrier = &comm_barrier;
            remoteMonitor_args[2].out_buffer = &to_incoming_handler;




            for ( int itr = 0; itr < NUMPEERS; itr++ ) {
                if ( pthread_attr_init( &remoteMonitor_attrs[itr] ) != 0 ) {
                    perror( "MAIN remote monitor pthread_attr_init: " );
                    exit( EXIT_FAILURE );
                }

                if ( pthread_create( worker_thread = &remoteMonitorThreads[itr],
                                     &remoteMonitor_attrs[itr],
                                     remote_peer_monitor,
                                     &remoteMonitor_args[itr] ) ) {
                    perror( "MAIN remote monitor pthread_create: " );
                    exit( EXIT_FAILURE );
                }

                ALL_WORKER_THREADS.push_back( worker_thread );
            }



            break;
        }
        case 3:
        {

            //ASSUMING 3 PEERS for TOTAL OF 4 NODES
            remoteMonitor_args[0].peerID = remotePeer_IDs[1];
            remoteMonitor_args[0].action = remoteMonitor::comm_actions::ACCEPT;
            remoteMonitor_args[0].c_data = nullptr;
            remoteMonitor_args[0].a_data = & accInfo;
            remoteMonitor_args[0].comm_barrier = &comm_barrier;
            remoteMonitor_args[0].out_buffer = &to_incoming_handler;

            connInfo[1].IPaddr = Hosts[remotePeer_IDs[0]];
            connInfo[1].port = atoi( Ports[remotePeer_IDs[0]] );
            remoteMonitor_args[1].peerID = remotePeer_IDs[0];
            remoteMonitor_args[1].action = remoteMonitor::comm_actions::ACCEPT;
            remoteMonitor_args[1].c_data = &connInfo[1];
            remoteMonitor_args[1].a_data = nullptr;
            remoteMonitor_args[1].comm_barrier = &comm_barrier;
            remoteMonitor_args[1].out_buffer = &to_incoming_handler;


            remoteMonitor_args[2].peerID = remotePeer_IDs[3];
            remoteMonitor_args[2].action = remoteMonitor::comm_actions::ACCEPT;
            remoteMonitor_args[2].c_data = nullptr;
            remoteMonitor_args[2].a_data = & accInfo;
            remoteMonitor_args[2].comm_barrier = &comm_barrier;
            remoteMonitor_args[2].out_buffer = &to_incoming_handler;




            for ( int itr = 0; itr < NUMPEERS; itr++ ) {
                if ( pthread_attr_init( &remoteMonitor_attrs[itr] ) != 0 ) {
                    perror( "MAIN remote monitor pthread_attr_init: " );
                    exit( EXIT_FAILURE );
                }

                if ( pthread_create( worker_thread = &remoteMonitorThreads[itr],
                                     &remoteMonitor_attrs[itr],
                                     remote_peer_monitor,
                                     &remoteMonitor_args[itr] ) ) {
                    perror( "MAIN remote monitor pthread_create: " );
                    exit( EXIT_FAILURE );
                }
                ALL_WORKER_THREADS.push_back( worker_thread );
            }

            break;
        }
        default:
            fprintf( stderr, "MAIN remote monitor invalid node ID value\n" );
            exit( EXIT_FAILURE );
    }


    /////////////RING BUFFER INSTANTIATIONS////////////////////////
    MessageBuffer<incomingMsgHandler::thread_arg::outPeer_NROWS,
            incomingMsgHandler::thread_arg::outPeer_BUFFSIZE>
            to_remote_sender_handler;

    MessageBuffer<incomingMsgHandler::thread_arg::rqstQ_NROWS,
            incomingMsgHandler::thread_arg::rqstQ_BUFFSIZE>
            to_rqst_queuebuff;

    MessageBuffer<requestQueue_notifier::thread_arg::outLocal_NROWS,
            requestQueue_notifier::thread_arg::outLocal_BUFFSIZE>
            to_local_sender;

    MessageBuffer<requestQueue_notifier::thread_arg::outPeer_NROWS,
            requestQueue_notifier::thread_arg::outPeer_BUFFSIZE>
            to_outgoingReply_buff;








    ///////////////////LOCAL CLIENT MONITOR THREADS//////////////////////////
    Socket local_listening_socket( AF_LOCAL, SOCK_DGRAM );

    struct sockaddr_un local_listening_addr;

    local_listening_addr.sun_family = AF_LOCAL;

    snprintf( local_listening_addr.sun_path,
              sizeof (local_listening_addr.sun_path ) - 1,
              nodeController::LocalListensock_template.c_str( ),
              MY_NODE_ID  );

    unlink( local_listening_addr.sun_path );

    if ( bind( local_listening_socket.get_channel( ),
               ( sockaddr* ) & local_listening_addr, sizeof (sockaddr_un ) ) != 0 ) {
        perror( "MAIN local_listen_socket bind: " );
        exit( EXIT_FAILURE );
    }

    pthread_mutex_t local_accept_lock;
    if ( pthread_mutex_init( &local_accept_lock, nullptr ) != 0 ) {
        perror( "MAIN local_accept_lock mutex init" );
        exit( EXIT_FAILURE );
    }

    localMonitor::accept_info local_acc_info( local_listening_socket,
                                              &local_accept_lock );

    constexpr int NUM_LOCAL_MONITORS = 2;
    localMonitor::thread_arg local_thread_arg[NUM_LOCAL_MONITORS];
    pthread_t local_monitor_TID[NUM_LOCAL_MONITORS];
    pthread_attr_t local_monitor_attr[NUM_LOCAL_MONITORS];

    for ( int itr = 0; itr < NUM_LOCAL_MONITORS; itr++ ) {

        local_thread_arg[itr] = { &local_acc_info,
                                 &to_incoming_handler };

        pthread_attr_init( &local_monitor_attr[itr] );
        pthread_create( worker_thread = &local_monitor_TID[itr], &local_monitor_attr[itr],
                        local_process_monitor, &local_thread_arg[itr] );
        ALL_WORKER_THREADS.push_back( worker_thread );

    }



    ///////////////////INCOMING MESSAGES HANDLER THREAD/////////////////////////
    pthread_t incomingMsgHandler_TID;
    pthread_attr_t incomingMSgHandler_attr;

    incomingMsgHandler::thread_arg
    incomingMsgHandler_arg{ &to_remote_sender_handler,
                           &to_incoming_handler,
                           &to_rqst_queuebuff };

    pthread_attr_init( &incomingMSgHandler_attr );
    pthread_create( worker_thread = &incomingMsgHandler_TID, &incomingMSgHandler_attr,
                    incoming_msg_handler, &incomingMsgHandler_arg );
    ALL_WORKER_THREADS.push_back( worker_thread );



    /////////////////REMOTE PEER MESSAGE SENDER//////////////////////////////
    pthread_t remoteSender_TID;
    pthread_attr_t remoteSender_attr;

    remoteSender::thread_arg remoteSender_arg{ &to_remote_sender_handler };

    pthread_attr_init( &remoteSender_attr );
    pthread_create( worker_thread = &remoteSender_TID, &remoteSender_attr,
                    remote_peer_sender, &remoteSender_arg );
    ALL_WORKER_THREADS.push_back( worker_thread );


    /////////////////OUTGOING REPLY MESSAGES HANDLER THREAD///////////////////
    pthread_t outgoingReply_TID;
    pthread_attr_t outgoingReply_attr;

    outgoingReplyHandler::thread_arg outgoingReply_arg{ &to_remote_sender_handler,
                                                       &to_outgoingReply_buff };

    pthread_attr_init( &outgoingReply_attr );
    pthread_create( worker_thread = &outgoingReply_TID, &outgoingReply_attr,
                    outgoing_peerMsg_handler, &outgoingReply_arg );
    ALL_WORKER_THREADS.push_back( worker_thread );







    ///////////////REQUEST QUEUE MANAGER THREADS////////////////////////////
    pthread_t rqstQ_manager_TID[2 * NUMFLAVORS];
    pthread_attr_t rqstQ_manager_attr[2 * NUMFLAVORS];


    requestQueue_manager::thread_arg rqstQ_manager_arg[2 * NUMFLAVORS];

    for ( int itr = 0; itr < 2 * NUMFLAVORS; itr++ ) {

        rqstQ_manager_arg[itr].rqstQ_buffer = & to_rqst_queuebuff;
        rqstQ_manager_arg[itr].whichrow = itr;

        pthread_attr_init( &rqstQ_manager_attr[itr] );
        pthread_create( worker_thread = &rqstQ_manager_TID[itr],
                        &rqstQ_manager_attr[itr],
                        request_queue_manager, &rqstQ_manager_arg[itr] );
        ALL_WORKER_THREADS.push_back( worker_thread );

    }


    //////////////////REQUEST QUEUE NOTIFIER THREADS//////////////////////////
    pthread_t rqstQ_notifier_TID[2 * NUMFLAVORS];
    pthread_attr_t rqstQ_notifier_attr[2 * NUMFLAVORS];


    requestQueue_notifier::thread_arg rqstQ_notifier_arg[2 * NUMFLAVORS];


    for ( int j = 0, itr = 1; itr < 3; itr++ ) {
        for ( int itr2 = 0; itr2 < 4; itr2++ ) {
            rqstQ_notifier_arg[j].outLocal_buffer = & to_local_sender;
            rqstQ_notifier_arg[j].outPeer_buffer = &to_outgoingReply_buff;
            rqstQ_notifier_arg[j].act = static_cast<Action> ( itr );
            rqstQ_notifier_arg[j].sub_act = static_cast<subAction> ( itr2 );

            pthread_attr_init( &rqstQ_notifier_attr[j] );
            pthread_create( worker_thread = &rqstQ_notifier_TID[j], &rqstQ_notifier_attr[j],
                            request_queue_notifier, &rqstQ_notifier_arg[j] );
            ALL_WORKER_THREADS.push_back( worker_thread );

            j++;

        }
    }



    ///////////////LOCAL CLIENT MESSAGE SENDER/////////////////////////////////
    pthread_t localSender_TID[2 * NUMFLAVORS];
    pthread_attr_t localSender_attr[2 * NUMFLAVORS];


    localSender::thread_arg localSender_arg[2 * NUMFLAVORS];



    for ( int itr = 1, j = 0; itr < 3; itr++ ) {
        for ( int itr2 = 0; itr2 < 4; itr2++ ) {
            localSender_arg[j].in_buffer = & to_local_sender;
            localSender_arg[j].act = static_cast<Action> ( itr );
            localSender_arg[j].sub_act = static_cast<subAction> ( itr2 );

            pthread_attr_init( &localSender_attr[j] );
            pthread_create( worker_thread =  &localSender_TID[j], &localSender_attr[j],
                            local_process_sender, &localSender_arg[j] );
            ALL_WORKER_THREADS.push_back( worker_thread );

            j++;

        }
    }

    for ( auto TID : ALL_WORKER_THREADS ) {
        pthread_join( *TID, nullptr );
        cerr << "MAIN jointed TID: " << *TID << endl;

    }




    return 0;
}

