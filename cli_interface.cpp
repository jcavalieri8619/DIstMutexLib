
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "globals.hpp"
#include "client_interface.hpp"
#include "client_utils.hpp"
#include "node_controller.hpp"
#include <iostream>

using std::cerr;
using std::endl;
extern unsigned MY_NODE_ID;

clientUtils::Message construct_client_msg( Action act,
                                           subAction sub_act,
                                           clientUtils::MESSAGE_T msg_type )
{
    return clientUtils::Message{ getpid( ), msg_type, act, sub_act };
}

void enter_critical_section( Action act, subAction subact )
{
    using namespace clientUtils;

    Socket mysock( AF_LOCAL, SOCK_DGRAM );
    struct sockaddr_un local_monitor_addr, myaddr;
    socklen_t sock_len;


    bzero( &myaddr, sizeof (myaddr ) );
    bzero( &local_monitor_addr, sizeof (local_monitor_addr ) );


    local_monitor_addr.sun_family = AF_LOCAL;


    snprintf( local_monitor_addr.sun_path, sizeof (local_monitor_addr.sun_path ) - 1,
              nodeController::LocalListensock_template.c_str( ), MY_NODE_ID  );



    myaddr.sun_family = AF_LOCAL;

    snprintf( myaddr.sun_path, sizeof (myaddr.sun_path ) - 1,
              ClientSock_template, getpid( ) );

    unlink( myaddr.sun_path );


    if ( bind( mysock.get_channel( ), (struct sockaddr*) &myaddr,
               sizeof (struct sockaddr_un ) ) != 0 ) {
        perror( "enter_crit_sect: bind to Unix Domain socket failed: " );
        exit( EXIT_FAILURE );
    }


    char buffer[sizeof (Message )];

    host2network_msg( construct_client_msg( act,
                                            subact,
                                            MESSAGE_T::LOCAL_REQUEST ),
                      buffer );

    //here we are asking node controller for permission to enter CS
    if ( sendto( mysock.get_channel( ), buffer,
                 sizeof (buffer ), 0,
                 (struct sockaddr *) &local_monitor_addr,
                 sizeof (local_monitor_addr ) ) != sizeof (Message ) ) {
        cerr << "enter_crit_section: sendto did not transmit"
                " all bytes" << endl;
        exit( EXIT_FAILURE );

    }

    memset( buffer, 0, sizeof (buffer ) );

    //here we are waiting for node controller to give us
    //permission to enter CS
    if ( recvfrom( mysock.get_channel( ), buffer,
                   sizeof (buffer ), MSG_WAITALL,
                   nullptr, nullptr ) != sizeof (Message ) ) {
        cerr << "enter_crit_section: recvfrom did not receive"
                " all bytes" << endl;
        exit( EXIT_FAILURE );

    }


    unlink( myaddr.sun_path );


    //now okay to enter critical section
    return;

}

void leave_critical_section( Action act, subAction subact )
{
    using namespace clientUtils;

    struct sockaddr_un local_sender_addr, myaddr;

    bzero( &local_sender_addr, sizeof (local_sender_addr ) );

    local_sender_addr.sun_family = AF_LOCAL;

    snprintf( local_sender_addr.sun_path,
              sizeof (local_sender_addr.sun_path ),
              nodeController::LocalSendersock_template.c_str( ), MY_NODE_ID,
              static_cast<int> ( act ), static_cast<int> ( subact ) );




    Socket mysock( AF_LOCAL, SOCK_DGRAM );
    bzero( &myaddr, sizeof (myaddr ) );
    myaddr.sun_family = AF_LOCAL;

    snprintf( myaddr.sun_path, sizeof (myaddr.sun_path ) - 1,
              ClientSock_template, getpid( ) );


    unlink( myaddr.sun_path );

    if ( bind( mysock.get_channel( ), (struct sockaddr*) &myaddr,
               sizeof (struct sockaddr_un ) ) != 0 ) {
        perror( "enter_crit_sect: bind to Unix Domain socket failed: " );
        exit( EXIT_FAILURE );
    }

    char buffer[sizeof (Message )];

    host2network_msg( construct_client_msg( act,
                                            subact,
                                            MESSAGE_T::FINISHED ),
                      buffer );

    //here we are telling NC that we are finished executing C.S.
    if ( sendto( mysock.get_channel( ), buffer,
                 sizeof (buffer ), 0,
                 (struct sockaddr *) &local_sender_addr,
                 sizeof (local_sender_addr ) ) != sizeof (Message ) ) {
        cerr << "leave_crit_section: sendto did not transmit"
                " all bytes" << endl;
        exit( EXIT_FAILURE );

    }

    //now okay for other processes to enter critical section
    //associated with act, subact

    return;
}

