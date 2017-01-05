#include "globals.hpp"
#include "node_controller.hpp"
#include "client_utils.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>

using namespace std;

namespace
{

    union MessageContainer
    {
        clientUtils::Message message;

        char bytestream[sizeof (message )];
    } ;



}



const char*
clientUtils::ClientSock_template( "/tmp/local_client_sock_%u.sock" );

namespace clientUtils
{




    //these do not need to convert byte ordres--just use the
    //union to convert the bytestream to the client_msg then
    //client2internal will convert client_msg to internal_msg

    void host2network_msg( const Message& msg, char* buffer )
    {
        MessageContainer converter;
        converter.message = msg;

        for ( int itr = 0; itr < sizeof (converter.bytestream );
              ( buffer[itr] = converter.bytestream[itr] ), itr++ );


        return;
    }

    void network2host_msg( Message& msg, const char* buffer )
    {
        MessageContainer converter;

        for ( int itr = 0; itr < sizeof (converter.bytestream );
              ( converter.bytestream[itr] = buffer[itr] ), itr++ );

        msg = converter.message;
        return;
    }

    std::ostream& operator<<( std::ostream& os, MESSAGE_T mtype )
    {
        switch ( mtype ) {
            case MESSAGE_T::FINISHED:
                os << "CLIENT_FINISHED";
                break;
            case MESSAGE_T::LOCAL_REQUEST:
                os << "CLIENT_REQUEST";
                break;
            case MESSAGE_T::PERMISSION_GRANTED:
                os << "CLIENT_PERMISSION_GRANTED";
                break;

        }
        os << endl;

        return os;
    }

    std::ostream& operator<<( std::ostream& os, const Message& msg )
    {
        os << "Message Type: " << ( msg.type )
                << "Action: " << ( msg.act )
                << "subAction: " << ( msg.subact )
                << "local PID: " << msg.localPID << endl;

        return os;

    }

}

