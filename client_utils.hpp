
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




    extern const char* ClientSock_template;

    enum class MESSAGE_T : uint32_t
    {
        LOCAL_REQUEST, PERMISSION_GRANTED, FINISHED
    };

    std::ostream& operator<<( std::ostream& os, MESSAGE_T mtype );

    struct Message
    {
        pid_t localPID;
        MESSAGE_T type;
        Action act;
        subAction subact;

        friend std::ostream& operator<<( std::ostream& os, const Message& msg );

    };


    void host2network_msg( const Message&, char* );

    void network2host_msg( Message&, const char* );


}
#endif /* CLIENT_UTILS_HPP */

