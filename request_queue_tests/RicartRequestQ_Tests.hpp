/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   RicartRequestQ_Tests.hpp
 * Author: jcavalie
 *
 * Created on Dec 13, 2016, 9:51:42 AM
 */

#ifndef RICARTREQUESTQ_TESTS_HPP
#define RICARTREQUESTQ_TESTS_HPP

#include <cppunit/extensions/HelperMacros.h>
#include "../node_controller.hpp"

constexpr size_t queueSize = 20;

class RicartRequestQ_Tests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( RicartRequestQ_Tests );

    CPPUNIT_TEST( testRicart_RequestQueue );
    CPPUNIT_TEST( testGet_size );
    CPPUNIT_TEST( testWait_local_client_finished );
    CPPUNIT_TEST( testNotify_local_client_finished );
    CPPUNIT_TEST( testEnqueue_request );
    CPPUNIT_TEST( testRemove_finished_peers );
    CPPUNIT_TEST( testIs_local_client_request );
    CPPUNIT_TEST( testPeek_at_head );
    CPPUNIT_TEST( testPop_local_client_from_head );
    CPPUNIT_TEST( testPop_deferred_reply_list );

    CPPUNIT_TEST_SUITE_END( );

public:
    RicartRequestQ_Tests( );
    virtual ~RicartRequestQ_Tests( );
    void setUp( );
    void tearDown( );

private:
    void testRicart_RequestQueue( );
    void testGet_size( );
    void testWait_local_client_finished( );
    void testNotify_local_client_finished( );
    void testEnqueue_request( );
    void testRemove_finished_peers( );
    void testIs_local_client_request( );
    void testPeek_at_head( );
    void testPop_local_client_from_head( );
    void testPop_deferred_reply_list( );

    nodeController::Ricart_RequestQueue ricart_RequestQueue;
    nodeController::LogicalClock clock;
    nodeController::LogicalClock remote_clock;

    nodeController::Message rqst_msgs[queueSize];

};

#endif /* RICARTREQUESTQ_TESTS_HPP */

