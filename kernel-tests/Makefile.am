ACLOCAL_AMFLAGS = -Im4
EXTRA_CFLAGS	 = -DDEBUG
AM_CXXFLAGS = -Wall -Wundef -Wno-trigraphs -Werror -std=c++14

if USE_GLIB
    ipa_kernel_tests_CPPFLAGS  = $(AM_CFLAGS) -DUSE_GLIB -Dstrlcpy=g_strlcpy @GLIB_CFLAGS@
    ipa_kernel_tests_LDFLAGS = -lpthread @GLIB_LIBS@
endif

requiredlibs = -lipanat
ipa_kernel_tests_LDADD =  $(requiredlibs)

ipa_kernel_testsdir            = $(prefix)
ipa_kernel_tests_PROGRAMS      = ipa_kernel_tests
dist_ipa_kernel_tests_SCRIPTS  = run.sh
ipa_kernel_tests_SOURCES =\
		TestManager.cpp \
		TestBase.cpp \
		InterfaceAbstraction.cpp \
		Pipe.cpp \
		PipeTestFixture.cpp \
		PipeTests.cpp \
		TLPAggregationTestFixture.cpp \
		TLPAggregationTests.cpp \
		MBIMAggregationTestFixtureConf11.cpp \
		MBIMAggregationTests.cpp \
		Logger.cpp \
		RoutingDriverWrapper.cpp \
		RoutingTests.cpp \
		IPAFilteringTable.cpp \
		Filtering.cpp \
		FilteringTest.cpp \
		HeaderInsertion.cpp \
		HeaderInsertionTests.cpp \
		TestsUtils.cpp \
		HeaderRemovalTestFixture.cpp \
		HeaderRemovalTests.cpp \
		IPv4Packet.cpp \
		RNDISAggregationTestFixture.cpp \
		RNDISAggregationTests.cpp \
		DataPathTestFixture.cpp \
		DataPathTests.cpp \
		IPAInterruptsTestFixture.cpp \
		IPAInterruptsTests.cpp \
		HeaderProcessingContextTestFixture.cpp \
		HeaderProcessingContextTests.cpp \
		FilteringEthernetBridgingTestFixture.cpp \
		FilteringEthernetBridgingTests.cpp \
		NatTest.cpp \
		IPv6CTTest.cpp \
		UlsoTest.cpp \
		main.cpp
