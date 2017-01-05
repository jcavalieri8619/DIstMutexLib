#include "node_controller.hpp"
#include "message_buffer.hpp"
#include "globals.hpp"
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>


extern Socket* peer_socket_array[];
extern unsigned MY_NODE_ID;
namespace
{
    Socket peer_socket;
}

void * nodeController::remote_peer_monitor( void * argument )
{
    using namespace nodeController::remoteMonitor;
    using namespace std;

    thread_arg arg = *(thread_arg *) argument;
    //delete (thread_arg *) argument;





    switch ( arg.action ) {

        case comm_actions::ACCEPT:
        {
            if ( !arg.a_data || !arg.a_data->accept_lock || !arg.out_buffer ||
                 !arg.comm_barrier ) {
                fprintf( stderr, "remote peer thread %d argument "
                         "is invalid\n", arg.peerID );
                exit( EXIT_FAILURE );
            }

            struct sockaddr_in cliaddr;
            socklen_t cliaddr_len = sizeof (cliaddr );

            pthread_mutex_lock( arg.a_data->accept_lock );

            Socket acc_socket( accept( arg.a_data->listen_socket.get_channel( ),
                                       (struct sockaddr*) &cliaddr,
                                       &cliaddr_len ) );

            pthread_mutex_unlock( arg.a_data->accept_lock );

            int nread;
            char IDbuffer[sizeof (int ) + 1] = { 0 };

            if ( ( nread = read( acc_socket.get_channel( ),
                                 IDbuffer, sizeof (int ) ) ) <= 0 ) {
                if ( nread == 0 ) {
                    fprintf( stderr, "remoteComm_monitor got EOF attempting to "
                             "read clients node_ID\n" );

                } else {
                    perror( "remoteComm_monitor reading nodeID: " );
                }
                exit( EXIT_FAILURE );
            }


            int whichnode = atoi( IDbuffer );

            peer_socket = acc_socket;

            peer_socket_array[whichnode] = &peer_socket;

            break;
        }

        case comm_actions::CONNECT:
        {
            if ( !arg.c_data || !arg.out_buffer || !arg.comm_barrier ) {
                fprintf( stderr, "remote peer thread %d argument "
                         "is invalid\n", arg.peerID );
                exit( EXIT_FAILURE );
            }

            struct sockaddr_in peer_addr = { };

            peer_addr.sin_family = AF_INET;
            peer_addr.sin_port = htons( arg.c_data->port );

            int rv;
            if ( ( rv = inet_pton( AF_INET, arg.c_data->IPaddr,
                                   &peer_addr.sin_addr ) ) != 1 ) {

                if ( rv == 0 ) {
                    fprintf( stderr, "remote_peer_monitor thread %d argument: "
                             "connect_info IP address has invalid format\n",
                             arg.peerID );
                    exit( EXIT_FAILURE );

                } else {
                    fprintf( stderr, "inet_pton failed in "
                             "remote_peer_monitor thread %d\n",
                             arg.peerID );
                    exit( EXIT_FAILURE );
                }
            }

            Socket conn_socket( AF_INET, SOCK_STREAM );

            if ( connect( conn_socket.get_channel( ),
                          (struct sockaddr*) &peer_addr,
                          sizeof (peer_addr ) ) < 0 ) {
                perror( "remote_peer_monitor connect" );
                exit( EXIT_FAILURE );
            }


            if ( write( conn_socket.get_channel( ),
                        to_string( MY_NODE_ID ).c_str( ),
                        sizeof (int ) ) < sizeof (int ) ) {

                perror( "remote_peer_sender write failed " );
                exit( EXIT_FAILURE );

            }

            peer_socket = conn_socket;

            peer_socket_array[arg.peerID] = &peer_socket;
            break;
        }
        default:
            //can never reach here because scoped enums do not allow
            //any other values other than ACCEPT and CONNECT
            exit( EXIT_FAILURE );

    }


    pthread_barrier_wait( arg.comm_barrier );

    //all remote monitor threads will block here until they have
    //all called barrier-wait; this ensures that global peer_socket_array
    //is in correct state with all elements containing internet sockets
    //to remote peers

    //now that all remote_peer_monitor threads have passed the barrier,
    //we can begin reading peer messages
    while ( true ) {

        Message msg;

        char buffer[sizeof (msg )] = { 0 };

        int nread;

        if ( ( nread = read( peer_socket_array[arg.peerID]->get_channel( ),
                             buffer,
                             sizeof (Message ) ) ) < sizeof (Message ) ) {

            if ( nread == 0 ) {
                fprintf( stderr, "remoteComm_monitor %d read EOF\n", MY_NODE_ID );

            } else if ( nread < 0 ) {
                perror( "remoteComm_monitor socket read " );

            } else {
                fprintf( stderr, "remoteComm_monitor %d read did not read enough"
                         "bytes for entire Message\n", MY_NODE_ID );

            }
            exit( EXIT_FAILURE );
        }



        network2host_msg( msg, buffer );


        //node_ID converted from receiver ID to sender ID
        //given that arg.ID is ID of remote peer that sent this msg to us
        msg.node_ID = arg.peerID;


        arg.out_buffer->produce_msg( 0, msg );


    }


    return nullptr;
}