#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/cli_interface.o \
	${OBJECTDIR}/client_utils.o \
	${OBJECTDIR}/incomingMsg_handler.o \
	${OBJECTDIR}/internal_utils.o \
	${OBJECTDIR}/localComm_monitor.o \
	${OBJECTDIR}/localComm_sender.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/outgoingReply_handler.o \
	${OBJECTDIR}/remoteComm_monitor.o \
	${OBJECTDIR}/remoteComm_sender.o \
	${OBJECTDIR}/requestQueue_manager.o \
	${OBJECTDIR}/requestQueue_notifier.o

# Test Directory
TESTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}/tests

# Test Files
TESTFILES= \
	${TESTDIR}/TestFiles/f3 \
	${TESTDIR}/TestFiles/f2 \
	${TESTDIR}/TestFiles/f1 \
	${TESTDIR}/TestFiles/f4

# Test Object Files
TESTOBJECTFILES= \
	${TESTDIR}/request_queue_tests/RicartRequestQ_Tests.o \
	${TESTDIR}/request_queue_tests/newtestrunner2.o \
	${TESTDIR}/tests/IncomingMsgTester.o \
	${TESTDIR}/tests/localMonitorTester.o \
	${TESTDIR}/tests/newtestrunner.o \
	${TESTDIR}/tests/newtestrunner1.o \
	${TESTDIR}/tests/remoteCommMonitor_tester.o \
	${TESTDIR}/tests/remoteMonitorTestClass.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nodecontroller

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nodecontroller: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nodecontroller ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/cli_interface.o: cli_interface.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cli_interface.o cli_interface.cpp

${OBJECTDIR}/client_utils.o: client_utils.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/client_utils.o client_utils.cpp

${OBJECTDIR}/incomingMsg_handler.o: incomingMsg_handler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/incomingMsg_handler.o incomingMsg_handler.cpp

${OBJECTDIR}/internal_utils.o: internal_utils.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/internal_utils.o internal_utils.cpp

${OBJECTDIR}/localComm_monitor.o: localComm_monitor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/localComm_monitor.o localComm_monitor.cpp

${OBJECTDIR}/localComm_sender.o: localComm_sender.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/localComm_sender.o localComm_sender.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/outgoingReply_handler.o: outgoingReply_handler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/outgoingReply_handler.o outgoingReply_handler.cpp

${OBJECTDIR}/remoteComm_monitor.o: remoteComm_monitor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/remoteComm_monitor.o remoteComm_monitor.cpp

${OBJECTDIR}/remoteComm_sender.o: remoteComm_sender.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/remoteComm_sender.o remoteComm_sender.cpp

${OBJECTDIR}/requestQueue_manager.o: requestQueue_manager.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/requestQueue_manager.o requestQueue_manager.cpp

${OBJECTDIR}/requestQueue_notifier.o: requestQueue_notifier.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/requestQueue_notifier.o requestQueue_notifier.cpp

# Subprojects
.build-subprojects:

# Build Test Targets
.build-tests-conf: .build-tests-subprojects .build-conf ${TESTFILES}
.build-tests-subprojects:

${TESTDIR}/TestFiles/f3: ${TESTDIR}/tests/IncomingMsgTester.o ${TESTDIR}/tests/newtestrunner1.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f3 $^ ${LDLIBSOPTIONS} `cppunit-config --libs`   

${TESTDIR}/TestFiles/f2: ${TESTDIR}/tests/localMonitorTester.o ${TESTDIR}/tests/newtestrunner.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f2 $^ ${LDLIBSOPTIONS} `cppunit-config --libs`   

${TESTDIR}/TestFiles/f1: ${TESTDIR}/tests/remoteCommMonitor_tester.o ${TESTDIR}/tests/remoteMonitorTestClass.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f1 $^ ${LDLIBSOPTIONS} `cppunit-config --libs`   

${TESTDIR}/TestFiles/f4: ${TESTDIR}/request_queue_tests/RicartRequestQ_Tests.o ${TESTDIR}/request_queue_tests/newtestrunner2.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f4 $^ ${LDLIBSOPTIONS} `cppunit-config --libs`   


${TESTDIR}/tests/IncomingMsgTester.o: tests/IncomingMsgTester.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 `cppunit-config --cflags` -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/IncomingMsgTester.o tests/IncomingMsgTester.cpp


${TESTDIR}/tests/newtestrunner1.o: tests/newtestrunner1.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 `cppunit-config --cflags` -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/newtestrunner1.o tests/newtestrunner1.cpp


${TESTDIR}/tests/localMonitorTester.o: tests/localMonitorTester.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 `cppunit-config --cflags` -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/localMonitorTester.o tests/localMonitorTester.cpp


${TESTDIR}/tests/newtestrunner.o: tests/newtestrunner.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 `cppunit-config --cflags` -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/newtestrunner.o tests/newtestrunner.cpp


${TESTDIR}/tests/remoteCommMonitor_tester.o: tests/remoteCommMonitor_tester.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 `cppunit-config --cflags` -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/remoteCommMonitor_tester.o tests/remoteCommMonitor_tester.cpp


${TESTDIR}/tests/remoteMonitorTestClass.o: tests/remoteMonitorTestClass.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 `cppunit-config --cflags` -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/remoteMonitorTestClass.o tests/remoteMonitorTestClass.cpp


${TESTDIR}/request_queue_tests/RicartRequestQ_Tests.o: request_queue_tests/RicartRequestQ_Tests.cpp 
	${MKDIR} -p ${TESTDIR}/request_queue_tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 `cppunit-config --cflags` -MMD -MP -MF "$@.d" -o ${TESTDIR}/request_queue_tests/RicartRequestQ_Tests.o request_queue_tests/RicartRequestQ_Tests.cpp


${TESTDIR}/request_queue_tests/newtestrunner2.o: request_queue_tests/newtestrunner2.cpp 
	${MKDIR} -p ${TESTDIR}/request_queue_tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 `cppunit-config --cflags` -MMD -MP -MF "$@.d" -o ${TESTDIR}/request_queue_tests/newtestrunner2.o request_queue_tests/newtestrunner2.cpp


${OBJECTDIR}/cli_interface_nomain.o: ${OBJECTDIR}/cli_interface.o cli_interface.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/cli_interface.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cli_interface_nomain.o cli_interface.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/cli_interface.o ${OBJECTDIR}/cli_interface_nomain.o;\
	fi

${OBJECTDIR}/client_utils_nomain.o: ${OBJECTDIR}/client_utils.o client_utils.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/client_utils.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/client_utils_nomain.o client_utils.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/client_utils.o ${OBJECTDIR}/client_utils_nomain.o;\
	fi

${OBJECTDIR}/incomingMsg_handler_nomain.o: ${OBJECTDIR}/incomingMsg_handler.o incomingMsg_handler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/incomingMsg_handler.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/incomingMsg_handler_nomain.o incomingMsg_handler.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/incomingMsg_handler.o ${OBJECTDIR}/incomingMsg_handler_nomain.o;\
	fi

${OBJECTDIR}/internal_utils_nomain.o: ${OBJECTDIR}/internal_utils.o internal_utils.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/internal_utils.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/internal_utils_nomain.o internal_utils.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/internal_utils.o ${OBJECTDIR}/internal_utils_nomain.o;\
	fi

${OBJECTDIR}/localComm_monitor_nomain.o: ${OBJECTDIR}/localComm_monitor.o localComm_monitor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/localComm_monitor.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/localComm_monitor_nomain.o localComm_monitor.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/localComm_monitor.o ${OBJECTDIR}/localComm_monitor_nomain.o;\
	fi

${OBJECTDIR}/localComm_sender_nomain.o: ${OBJECTDIR}/localComm_sender.o localComm_sender.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/localComm_sender.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/localComm_sender_nomain.o localComm_sender.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/localComm_sender.o ${OBJECTDIR}/localComm_sender_nomain.o;\
	fi

${OBJECTDIR}/main_nomain.o: ${OBJECTDIR}/main.o main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/main.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_nomain.o main.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/main.o ${OBJECTDIR}/main_nomain.o;\
	fi

${OBJECTDIR}/outgoingReply_handler_nomain.o: ${OBJECTDIR}/outgoingReply_handler.o outgoingReply_handler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/outgoingReply_handler.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/outgoingReply_handler_nomain.o outgoingReply_handler.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/outgoingReply_handler.o ${OBJECTDIR}/outgoingReply_handler_nomain.o;\
	fi

${OBJECTDIR}/remoteComm_monitor_nomain.o: ${OBJECTDIR}/remoteComm_monitor.o remoteComm_monitor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/remoteComm_monitor.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/remoteComm_monitor_nomain.o remoteComm_monitor.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/remoteComm_monitor.o ${OBJECTDIR}/remoteComm_monitor_nomain.o;\
	fi

${OBJECTDIR}/remoteComm_sender_nomain.o: ${OBJECTDIR}/remoteComm_sender.o remoteComm_sender.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/remoteComm_sender.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/remoteComm_sender_nomain.o remoteComm_sender.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/remoteComm_sender.o ${OBJECTDIR}/remoteComm_sender_nomain.o;\
	fi

${OBJECTDIR}/requestQueue_manager_nomain.o: ${OBJECTDIR}/requestQueue_manager.o requestQueue_manager.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/requestQueue_manager.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/requestQueue_manager_nomain.o requestQueue_manager.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/requestQueue_manager.o ${OBJECTDIR}/requestQueue_manager_nomain.o;\
	fi

${OBJECTDIR}/requestQueue_notifier_nomain.o: ${OBJECTDIR}/requestQueue_notifier.o requestQueue_notifier.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/requestQueue_notifier.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/requestQueue_notifier_nomain.o requestQueue_notifier.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/requestQueue_notifier.o ${OBJECTDIR}/requestQueue_notifier_nomain.o;\
	fi

# Run Test Targets
.test-conf:
	@if [ "${TEST}" = "" ]; \
	then  \
	    ${TESTDIR}/TestFiles/f3 || true; \
	    ${TESTDIR}/TestFiles/f2 || true; \
	    ${TESTDIR}/TestFiles/f1 || true; \
	    ${TESTDIR}/TestFiles/f4 || true; \
	else  \
	    ./${TEST} || true; \
	fi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nodecontroller

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
