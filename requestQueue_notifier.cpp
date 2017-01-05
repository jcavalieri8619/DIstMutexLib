#include "node_controller.hpp"
#include "globals.hpp"
#include "message_buffer.hpp"
#include <list>
#include <iostream>

extern nodeController::Ricart_RequestQueue RQST_Q;
size_t compute_buffer_index( Action act, subAction sub_act );
extern unsigned MY_NODE_ID;

void * nodeController::request_queue_notifier( void* argument )
{
    using namespace nodeController::requestQueue_notifier;
    using std::list;
    using std::cerr;
    using std::endl;

    thread_arg arg = *(thread_arg *) argument;
    //delete (thread_arg *) argument;

    size_t my_index = compute_buffer_index( arg.act, arg.sub_act );

    while ( true ) {




        if ( RQST_Q.peek_at_head( arg.act,
                                  arg.sub_act,
                                  &Ricart_RequestQueue::is_local_client_request  ) ) {

            //if here, then local client is at head of queue
            Message msg;

            RQST_Q.peek_at_head( arg.act, arg.sub_act,
                                 [&msg]( Message m ){
                                     return (msg = m );
                                 } );




            RQST_Q.pop_local_client_from_head( arg.act, arg.sub_act );


            arg.outLocal_buffer->produce_msg( my_index, msg );

            RQST_Q.wait_local_client_finished( arg.act, arg.sub_act );

            //now that client message completely finished,
            //we can free the loc_msg pointer
            delete msg.loc_msg;

        }

        //if here, then we've either found a local client at head of queue
        //and gave permission for local client to enter C.S. and local client
        //finished in C.S.

        //or there was not a local client at head of queue

        //in both cases, pop_deferred_reply will construct list of all
        //messages on queue up to but not including my next message and
        //these are exactly the messages I need to REPLY to in either case.

        list<Message> deferred_replies;
        RQST_Q.pop_deferred_reply_list( arg.act, arg.sub_act, deferred_replies );

        for ( auto next_msg : deferred_replies ) {

            if ( next_msg.loc_msg ) {
                if ( DO_DEBUG ) {
                    std::cerr << "ERROR: queue_notifier added local client message"
                            "to deferred reply list; here is the "
                            "local message: " << std::endl
                            << next_msg << std::endl;
                }
                exit( EXIT_FAILURE );
            }
            


            arg.outPeer_buffer->produce_msg( 0, next_msg );


        }


    }
    return nullptr;

}