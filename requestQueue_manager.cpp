#include "node_controller.hpp"
#include "globals.hpp"
#include "message_buffer.hpp"
#include <iostream>

extern nodeController::Ricart_RequestQueue RQST_Q;
extern unsigned MY_NODE_ID;

void * nodeController::request_queue_manager( void* argument )
{
    using namespace nodeController::requestQueue_manager;
    using nodeController::Message;
    using std::cerr;
    using std::endl;

    thread_arg arg = *(thread_arg *) argument;
    //delete (thread_arg *) argument;

    while ( true ) {

        Message msg;

        msg = arg.rqstQ_buffer->consume_msg( arg.whichrow );

        switch ( msg.type ) {

            case MESSAGE_T::REPLY:

                if ( ( arg.whichrow % 2 ) == 0 ) {

                    std::cerr << "request_queue_manager:\n"
                            "whichrow arg should be odd for REPLY message" <<
                            std::endl << msg << std::endl;
                    exit( EXIT_FAILURE );


                }


                RQST_Q.remove_finished_peers( msg.act, msg.sub_act, msg );
                break;

            case MESSAGE_T::REQUEST:

                if ( ( arg.whichrow % 2 ) != 0 ) {

                    std::cerr << "request_queue_manager:\n"
                            "whichrow should be even for REQUEST message" <<
                            std::endl << msg << std::endl;

                    exit( EXIT_FAILURE );


                }
            

                RQST_Q.enqueue_request( msg.act, msg.sub_act, msg );
                break;

            default:
                cerr << "request_queue_manager:\n received msg with invalid "
                        "MESSAGE_T type\n" << msg << endl;
                exit( EXIT_FAILURE );


        }

    }

    return nullptr;
}