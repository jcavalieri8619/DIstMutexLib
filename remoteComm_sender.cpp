#include "node_controller.hpp"
#include "globals.hpp"
#include "message_buffer.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
using std::cerr;
using std::endl;


extern Socket* peer_socket_array[];
extern unsigned MY_NODE_ID;

void* nodeController::remote_peer_sender( void* argument )
{

    using namespace nodeController::remoteSender;
    using nodeController::Message;

    thread_arg arg = *(thread_arg *) argument;
    //delete (thread_arg *) argument;

    while ( true ) {

        Message msg;
        char buffer[sizeof (msg )] = { 0 };

        msg = arg.in_buffer->consume_msg( 0 );


        switch (  msg.type ) {
            case MESSAGE_T::REPLY:
            {
                host2network_msg( msg, buffer );

                if ( write( peer_socket_array[msg.node_ID]->get_channel( ),
                            buffer, sizeof (buffer ) ) < sizeof (buffer ) ) {

                    perror( "remote_peer_sender write failed " );
                    exit( EXIT_FAILURE );

                }
                break;

            }
            case MESSAGE_T::REQUEST:
            {

                for ( int itr = 0 ; itr < NODECOUNT; itr++ ) {
                    if ( itr != MY_NODE_ID ) {

                        msg.node_ID = itr;
                        host2network_msg( msg, buffer );

                        if ( write( peer_socket_array[msg.node_ID]->get_channel( ),
                                    buffer, sizeof (buffer ) ) < sizeof (buffer ) ) {

                            perror( "remote_peer_sender write failed " );
                            exit( EXIT_FAILURE );

                        }

                        memset( buffer, 0, sizeof (buffer ) );

                    }
                }
                break;

            }
            default:
                cerr << "RemoteComm_sender invalid message type: " << msg << endl;
                exit( EXIT_FAILURE );

        }






    }



    return nullptr;

}

