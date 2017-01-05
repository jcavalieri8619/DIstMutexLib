
/*
 * File:   client_utils.hpp
 * Author: jcavalie
 *
 * Created on November 14, 2016, 2:50 PM
 */

#ifndef CLIENT_UTILS_HPP
#define CLIENT_UTILS_HPP
#include <pthread.h>
#include "globals.hpp"
#include <iostream>
#include <cstdint>



namespace clientUtils
{

    /**
     * Unix domain socket address template that is differentiated
     * by substituting the local clients PID into the template. This is the
     * address that the node controller will always use to communicate with
     * a particlar local client
     */
    static const char*
    ClientSock_template( "/tmp/local_client_sock_%u.sock" );


    /**
     * Message type for messages passed back and forth between local client
     * and node controller.
     * LOCAL_REQUEST is used for making a request
     * PERMISSION_GRANTED is sent to local client when it can enter its critical Sect
     * FINISHED is sent to node controller when finished executing critical sect
     */
    enum class MESSAGE_T : uint32_t
    {
        LOCAL_REQUEST, PERMISSION_GRANTED, FINISHED
    };

    std::ostream& operator<<( std::ostream& os, MESSAGE_T mtype );

    /**
     * Message structure containing fields required by node controller like
     * whether Action is produce or consume and the type of "flavor" indicated
     * by subAction.
     */
    struct Message
    {
        pid_t localPID;
        MESSAGE_T type;
        Action act;
        subAction subact;

        friend std::ostream& operator<<( std::ostream& os, const Message& msg );

    };


    /**
     * serializes Message structure for sending over socket
     * @param Message is prepared Message structure
     * @param char* is string of bytes representing structure 
     */
    void host2network_msg( const Message&, char* );

    void network2host_msg( Message&, const char* );


}
#endif /* CLIENT_UTILS_HPP */

