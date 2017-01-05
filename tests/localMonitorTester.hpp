
/*
 * File:   localMonitorTester.hpp
 * Author: jcavalie
 *
 * Created on Dec 11, 2016, 12:40:05 AM
 */
//
#ifndef LOCALMONITORTESTER_HPP
#define LOCALMONITORTESTER_HPP
#include "../node_controller.hpp"
#include "../message_buffer.hpp"
#include <thread>

#include <cppunit/extensions/HelperMacros.h>

class localMonitorTester : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( localMonitorTester );

    CPPUNIT_TEST( testLocal_process_monitor );

    CPPUNIT_TEST_SUITE_END( );

public:
    localMonitorTester( );
    virtual ~localMonitorTester( );
    void setUp( );
    void tearDown( );

private:
    void testLocal_process_monitor( );

    Socket listen_sock;

    nodeController::localMonitor::accept_info* accInfo;

    nodeController::localMonitor::thread_arg* arg;

    std::thread monitor_thread;
};

#endif /* LOCALMONITORTESTER_HPP */

