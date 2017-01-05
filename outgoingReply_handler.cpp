#include <locale>

#include "globals.hpp"
#include "node_controller.hpp"
#include "message_buffer.hpp"



extern nodeController::LogicalClock node_clock;

void * nodeController::outgoing_peerMsg_handler( void* argument )
{
    using namespace nodeController::outgoingReplyHandler;


    thread_arg arg = *(thread_arg *) argument;
    //delete (thread_arg *) argument;

    while ( true ) {

        Message msg;

        msg = arg.in_buffer->consume_msg( 0 );

        msg.type = MESSAGE_T::REPLY;

        node_clock.clear_timestamp( msg.time_stamp );

        node_clock( msg.time_stamp );

        arg.out_buffer->produce_msg( 0, msg );




    }


}