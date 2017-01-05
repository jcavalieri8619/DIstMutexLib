#include "node_controller.hpp"
#include "globals.hpp"
#include "client_utils.hpp"
#include "message_buffer.hpp"
#include <iostream>
#include <stdexcept>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>


extern nodeController::Ricart_RequestQueue RQST_Q;

size_t compute_buffer_index( Action act, subAction sub_act );

extern unsigned MY_NODE_ID;

void * nodeController::local_process_sender( void* argument )
{
    using namespace nodeController::localSender;
    using std::cerr;
    using std::endl;

    thread_arg arg = *(thread_arg *) argument;
    //delete (thread_arg *) argument;

    size_t my_index = compute_buffer_index( arg.act, arg.sub_act );


    Socket mySocket( AF_LOCAL, SOCK_DGRAM );

    struct sockaddr_un myaddr = {  };

    myaddr.sun_family = AF_LOCAL;
    snprintf( myaddr.sun_path, sizeof (myaddr.sun_path ) - 1,
              LocalSendersock_template.c_str( ), MY_NODE_ID,
              static_cast<int> ( arg.act ), static_cast<int> ( arg.sub_act ) );

    unlink( myaddr.sun_path );


    if ( bind( mySocket.get_channel( ),
               ( sockaddr* ) & myaddr,
               sizeof (sockaddr_un ) ) == -1 ) {
        perror( "local_process_sender " );
        exit( EXIT_FAILURE );
    }

    while ( true ) {

        Message msg;
        clientUtils::Message cli_msg;
        char buffer[sizeof (cli_msg )] = { 0 };

        msg = arg.in_buffer->consume_msg( my_index );





        struct sockaddr_un cliaddr = {  };
        socklen_t clilen;

        if ( !msg.loc_msg ) {
            cerr << "local_process_sender: received message with "
                    "NULL loc_msg pointer; here is message:\n" << msg << endl;
            exit( EXIT_FAILURE );
        }

        cliaddr = msg.loc_msg->cliaddr;
        clilen = msg.loc_msg->clilen;



        internal2client_msg( msg, cli_msg,
                             clientUtils::MESSAGE_T::PERMISSION_GRANTED );

        clientUtils::host2network_msg( cli_msg, buffer );

        int numread;
        //here we granting permission to local client to enter C.S.
        if ( (numread=sendto( mySocket.get_channel( ), buffer,
                     sizeof (buffer ), 0,
                     (struct sockaddr *) &cliaddr,
                     sizeof (sockaddr_un ) )) != sizeof (buffer ) ) {
            cerr << "local_process_sender: sendto did not transmit"
                    " all bytes of client message:\n" << cli_msg << endl <<
                    "converted from internal message:\n" << msg << endl;
            if ( numread== -1)
                perror("localComm_sender: sendto failed");
            exit( EXIT_FAILURE );

        }

        memset( buffer, 0, sizeof (buffer ) );


        //here we are waiting for local client to finish in critical section
        if ( (numread=recvfrom( mySocket.get_channel( ), buffer,
                       sizeof (buffer ), 0, nullptr, nullptr )) != sizeof (cli_msg ) ) {
            cerr << "local_process_sender: recvfrom did not receive"
                    " all bytes of message" << endl;
            if ( numread==-1)
                perror("localComm_sender recvfrom failed");
            exit( EXIT_FAILURE );

        }

        clientUtils::Message rcvd_msg;

        clientUtils::network2host_msg( rcvd_msg, buffer );

        if ( rcvd_msg.type != clientUtils::MESSAGE_T::FINISHED ) {
            cerr << "localComm_sender: client returned invalid "
                    "message type instead of indicating that is finished"
                    " its critical section" << endl;
            exit( EXIT_FAILURE );
        }


        RQST_Q.notify_local_client_finished( arg.act, arg.sub_act );


    }


    return nullptr;
}