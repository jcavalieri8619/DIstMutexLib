#include "globals.hpp"
#include "node_controller.hpp"
#include "message_buffer.hpp"
#include <iostream>

extern nodeController::LogicalClock node_clock;
extern unsigned MY_NODE_ID;

void * nodeController::incoming_msg_handler( void* argument )
{
    using std::cerr;
    using std::endl;
    using namespace nodeController::incomingMsgHandler;
    using nodeController::Message;

    thread_arg arg = *(thread_arg *) argument;
    //delete (thread_arg *) argument;

    while ( true ) {
        Message incoming;

        incoming = arg.in_buffer->consume_msg( 0 );

        //handles both uninit timestamp from local clients and
        //compares peer's timestamps with local node's clock
        node_clock( incoming.time_stamp );


        int row_offset = ( incoming.type == MESSAGE_T::REPLY ) ? 1 : 0;
        int whichrow = row_offset + ( 2 * static_cast<int> ( incoming.sub_act ) );



        arg.rqstQ_buffer->produce_msg( whichrow,
                                       incoming );

        if ( incoming.loc_msg ) {
            //need to set loc_msg pointer to NULL before sending over network
            //but loc_msg pointer remains for local RQST queue
            Message localOut = incoming;
            localOut.loc_msg = nullptr;

            arg.out_buffer->produce_msg( 0, localOut );




        }



    }



    return nullptr;
}