
/*
 * File:   IncomingMsgTester.hpp
 * Author: jcavalie
 *
 * Created on Dec 11, 2016, 11:37:43 PM
 */

#ifndef INCOMINGMSGTESTER_HPP
#define INCOMINGMSGTESTER_HPP
//
#include <cppunit/extensions/HelperMacros.h>
#include "../node_controller.hpp"
#include "../globals.hpp"
#include "../message_buffer.hpp"
#include <thread>

class IncomingMsgTester : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( IncomingMsgTester );

    CPPUNIT_TEST( testIncoming_msg_handler );

    CPPUNIT_TEST_SUITE_END( );

public:
    IncomingMsgTester( );
    virtual ~IncomingMsgTester( );
    void setUp( ) override;
    void tearDown( ) override;

    void run_remote_comm( );
    void run_local_comm( );


private:
    void testIncoming_msg_handler( );


    nodeController::incomingMsgHandler::thread_arg* arg;


    std::thread handler_thread;



    nodeController::LogicalClock nodeClock;
    nodeController::remoteMonitor::thread_arg* r_arg;
    nodeController::remoteMonitor::connect_info* connInfo;
    std::thread r_monitor_thread;


    Socket listen_sock;
    nodeController::localMonitor::accept_info* accInfo;
    nodeController::localMonitor::thread_arg* l_arg;
    std::thread l_monitor_thread;

};

#endif /* INCOMINGMSGTESTER_HPP */

