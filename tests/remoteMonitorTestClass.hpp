
/*
 * File:   remoteMonitorTestClass.hpp
 * Author: jcavalie
 *
 * Created on Dec 10, 2016, 3:52:12 PM
 */

#ifndef REMOTEMONITORTESTCLASS_HPP
#define REMOTEMONITORTESTCLASS_HPP
#include "../node_controller.hpp"
#include "../message_buffer.hpp"
#include "../globals.hpp"
#include <thread>
#include <cppunit/extensions/HelperMacros.h>

class remoteMonitorTestClass : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( remoteMonitorTestClass );

    CPPUNIT_TEST( testRemote_peer_monitor );

    CPPUNIT_TEST_SUITE_END( );

public:

    remoteMonitorTestClass( );
    virtual ~remoteMonitorTestClass( );
    void setUp( );
    void tearDown( );

private:
    void testRemote_peer_monitor( );


    nodeController::remoteMonitor::thread_arg* arg;
    nodeController::remoteMonitor::connect_info* connInfo;

    nodeController::LogicalClock nodeClock;
    std::thread monitor_thread;

};

#endif /* REMOTEMONITORTESTCLASS_HPP */

