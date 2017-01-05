

/*
 * File:   RicartRequestQ_Tests.cpp
 * Author: jcavalie
 *
 * Created on Dec 13, 2016, 9:51:42 AM
 */

#include "RicartRequestQ_Tests.hpp"
#include "../node_controller.hpp"
#include "../globals.hpp"
#include <iostream>

CPPUNIT_TEST_SUITE_REGISTRATION( RicartRequestQ_Tests );
using std::cerr;
using std::endl;
using namespace nodeController;

RicartRequestQ_Tests::RicartRequestQ_Tests( ) : ricart_RequestQueue( 2 * queueSize ){ }

RicartRequestQ_Tests::~RicartRequestQ_Tests( ){ }

void RicartRequestQ_Tests::setUp( ){ }

void RicartRequestQ_Tests::tearDown( ){ }

void RicartRequestQ_Tests::testRicart_RequestQueue( )
{


    if ( true /*check result*/ ) {
        CPPUNIT_ASSERT( true );
    }
}

void RicartRequestQ_Tests::testGet_size( )
{
    Action act = Action::CONSUME;
    subAction sub_act = subAction::FLAVOR_1;

}

void RicartRequestQ_Tests::testWait_local_client_finished( ){
    //    Action act;
    //    subAction sub_act;

    //    ricart_RequestQueue.wait_local_client_finished( act, sub_act );
    //    if ( true /*check result*/ ) {
    //        CPPUNIT_ASSERT( true );
    //    }
}

void RicartRequestQ_Tests::testNotify_local_client_finished( ){
    //    Action act;
    //    subAction sub_act;

    //    ricart_RequestQueue.notify_local_client_finished( act, sub_act );
    //    if ( true /*check result*/ ) {
    //        CPPUNIT_ASSERT( true );
    //    }
}

//OK, I am NODE_ID =1 and remote peer NODE_ID=3

void RicartRequestQ_Tests::testEnqueue_request( )
{




    //RQSTS (P,0), (C,1), (P,2), (C,3)
    for ( int msgcount = 0 ; msgcount < queueSize / 2; msgcount++ ) {


        rqst_msgs[msgcount].act = ( msgcount % 2 ) ? Action::CONSUME : Action::PRODUCE;
        rqst_msgs[msgcount].sub_act = static_cast<subAction> ( msgcount % 4 );
        rqst_msgs[msgcount].node_ID = ( ( msgcount + 1 ) % 3 ) ? 1 : 3;
        rqst_msgs[msgcount].type = nodeController::MESSAGE_T::REQUEST;
        rqst_msgs[msgcount].time_stamp = LogicalClock::UNITIALIZED;

        if ( rqst_msgs[msgcount].node_ID == 1/*MY_NODE_ID*/ ) {
            //simulating local client sends me a message and I
            //initialize his timestamp
            clock( rqst_msgs[msgcount].time_stamp );
            //bits numbered from right to left
            rqst_msgs[msgcount].rcvdReplies = 0b10111;


        } else if ( rqst_msgs[msgcount].node_ID == 3/*remote ID*/ ) {
            //simulating remote peer initializing his msg then
            //sending msg to me and when it arrives, my incoming msg
            //handler analyzes his time stamp
            remote_clock( rqst_msgs[msgcount].time_stamp );
            clock( rqst_msgs[msgcount].time_stamp );
            nodeController::update_rcvd_replies_array(
                                                       rqst_msgs[msgcount].node_ID ,
                                                       rqst_msgs[msgcount].rcvdReplies );
        }

        //everyone gets their own node ID reply check mark

        ricart_RequestQueue.enqueue_request( rqst_msgs[msgcount].act,
                                             rqst_msgs[msgcount].sub_act,
                                             rqst_msgs[msgcount] );

    }

    cerr << "printing ENQUEUE test to file" << endl;
    ricart_RequestQueue.print_queue_to_file( "./ENQUEUE_TEST_QUEUE.txt" );

    //CPPUNIT_ASSERT( Qmsg == rqst_msg );

}

void RicartRequestQ_Tests::testRemove_finished_peers( )
{


    testEnqueue_request( );

    for ( int msgcount = queueSize / 2 ; msgcount < queueSize ; msgcount++ ) {
        unsigned curr_val;
        constexpr unsigned MY_ID = 1;
        constexpr unsigned REMOTE_ID = 3;

        curr_val = ( ( msgcount + 1 ) % 3 ) ? 3 : 1;

        switch ( curr_val ) {
            case MY_ID:
            {
                //this case is for my NODE ID but I'll alternate
                //between myself and remote ID and make
                //more rqsts msgs here to add more rqsts msgs
                //for remote peer into the Q's given that
                //above I added more for myself.
                rqst_msgs[msgcount].node_ID = ( msgcount % 2 ) ? MY_ID : REMOTE_ID;
                rqst_msgs[msgcount].act = ( msgcount % 2 ) ? Action::CONSUME : Action::PRODUCE;
                rqst_msgs[msgcount].sub_act = static_cast<subAction> ( msgcount % 4 );
                rqst_msgs[msgcount].type = nodeController::MESSAGE_T::REQUEST;
                rqst_msgs[msgcount].time_stamp = LogicalClock::UNITIALIZED;

                if ( rqst_msgs[msgcount].node_ID == MY_ID ) {

                    clock( rqst_msgs[msgcount].time_stamp );

                } else {

                    remote_clock( rqst_msgs[msgcount].time_stamp );
                    clock( rqst_msgs[msgcount].time_stamp );
                }
                //everyone gets their own node ID reply check mark
                nodeController::update_rcvd_replies_array(
                                                           rqst_msgs[msgcount].node_ID ,
                                                           rqst_msgs[msgcount].rcvdReplies );

                ricart_RequestQueue.enqueue_request( rqst_msgs[msgcount].act,
                                                     rqst_msgs[msgcount].sub_act,
                                                     rqst_msgs[msgcount] );
                break;
            }
            case REMOTE_ID:
            {
                //here, remote is sending replies to me to indicate that
                //he already ran his CS for any msgs ahead of me on queue

                for ( int itr2 = 0 ; itr2 < queueSize / 2 ; itr2++ ) {
                    if ( rqst_msgs[itr2].node_ID == MY_ID ) {

                        rqst_msgs[msgcount].node_ID = REMOTE_ID;
                        rqst_msgs[msgcount].act = rqst_msgs[itr2].act;
                        rqst_msgs[msgcount].sub_act = rqst_msgs[itr2].sub_act;
                        rqst_msgs[msgcount].type = nodeController::MESSAGE_T::REPLY;
                        rqst_msgs[msgcount].time_stamp = LogicalClock::UNITIALIZED;
                        remote_clock( rqst_msgs[itr2].time_stamp );
                        remote_clock( rqst_msgs[msgcount].time_stamp );
                        clock( rqst_msgs[msgcount].time_stamp );
                        ricart_RequestQueue.remove_finished_peers( rqst_msgs[msgcount].act,
                                                                   rqst_msgs[msgcount].sub_act,
                                                                   rqst_msgs[msgcount] );

                    }
                }


                break;
            }
            default:
                cerr << "test ENQUEUE switch statement invalid NODE_ID" << endl;
        }

    }
    cerr << "printing REMOVE PEER test to file" << endl;
    ricart_RequestQueue.print_queue_to_file( "./REMOVEPEER_TEST_QUEUE.txt" );


}

void RicartRequestQ_Tests::testIs_local_client_request( ){
    //    const Message& msg;

    //    bool result = ricart_RequestQueue.is_local_client_request( msg );
    //    if ( true /*check result*/ ) {
    //        CPPUNIT_ASSERT( true );
    //    }
}

void RicartRequestQ_Tests::testPeek_at_head( )
{
    Action act;
    subAction sub_act;

    //    if ( ricart_RequestQueue.get_size( act, sub_act ) != 0 ) {
    //        Message result = ricart_RequestQueue.peek_at_head( act, sub_act );
    //    }
    if ( true /*check result*/ ) {
        CPPUNIT_ASSERT( true );
    }
}

void RicartRequestQ_Tests::testPop_local_client_from_head( )
{
    Action act;
    subAction sub_act;

    //ricart_RequestQueue.pop_local_client_from_head( act, sub_act );
    if ( true /*check result*/ ) {
        CPPUNIT_ASSERT( true );
    }
}

void RicartRequestQ_Tests::testPop_deferred_reply_list( )
{
    Action act;
    subAction sub_act;
    //    std::list<Message>& deferred_list;

    //    ricart_RequestQueue.pop_deferred_reply_list( act, sub_act, deferred_list );
    //    if ( true /*check result*/ ) {
    //        CPPUNIT_ASSERT( true );
    //    }
}

