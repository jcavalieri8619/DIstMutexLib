
#include "node_controller.hpp"
#include "client_utils.hpp"
#include "globals.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include "message_buffer.hpp"

#include <iostream>

extern unsigned MY_NODE_ID;

void * nodeController::local_process_monitor( void* argument )
{
    using namespace nodeController::localMonitor;
    using namespace std;
    thread_arg arg = *(thread_arg *) argument;
    //delete (thread_arg *) argument;


    struct sockaddr_un cliaddr = { };
    socklen_t clilen;
    int numread;

    while ( true ) {

        clilen = sizeof (cliaddr );
        clientUtils::Message cli_msg;
        char buffer[sizeof (cli_msg )] = { };

        if ( pthread_mutex_lock(arg.a_data->accept_lock)!=0){
            perror("localComm_monitor mutex lock");
            exit(EXIT_FAILURE);
        }

        if ( ( numread = recvfrom( arg.a_data->listen_socket.get_channel( ),
                                   buffer,
                                   sizeof (buffer ), MSG_WAITALL,
                                   (struct sockaddr*) &cliaddr,
                                   &clilen ) ) != sizeof (buffer ) ) {
            if ( numread == -1 ) {
                perror( "unix domain socket recvfrom error in local monitor" );

            }

            fprintf( stderr, "unix domain socket recvfrom received"
                     " incorrect number of bytes in local monitor\n" );
            exit( EXIT_FAILURE );

        }

        if ( pthread_mutex_unlock(arg.a_data->accept_lock)!=0){
            perror("localComm_monitor mutex lock");
            exit(EXIT_FAILURE);
        }



        clientUtils::network2host_msg( cli_msg, buffer );



        nodeController::Message internal_msg;




        client2internal_msg( internal_msg, cli_msg, cliaddr, clilen );





        arg.out_buffer->produce_msg( 0, internal_msg );

    }

    return nullptr;

}