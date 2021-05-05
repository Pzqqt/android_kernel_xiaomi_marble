AM_CPPFLAGS = -I./../inc \
	      -I$(top_srcdir)/ipanat/inc

AM_CPPFLAGS += -Wall -Wundef -Wno-trigraphs
AM_CPPFLAGS += -g -DDEBUG -DNAT_DEBUG

ipanattest_SOURCES = \
		ipa_nat_testREG.c \
		ipa_nat_test000.c \
		ipa_nat_test001.c \
		ipa_nat_test002.c \
		ipa_nat_test003.c \
		ipa_nat_test004.c \
		ipa_nat_test005.c \
		ipa_nat_test006.c \
		ipa_nat_test007.c \
		ipa_nat_test008.c \
		ipa_nat_test009.c \
		ipa_nat_test010.c \
		ipa_nat_test011.c \
		ipa_nat_test012.c \
		ipa_nat_test013.c \
		ipa_nat_test014.c \
		ipa_nat_test015.c \
		ipa_nat_test016.c \
		ipa_nat_test017.c \
		ipa_nat_test018.c \
		ipa_nat_test019.c \
		ipa_nat_test020.c \
		ipa_nat_test021.c \
		ipa_nat_test022.c \
		ipa_nat_test023.c \
		ipa_nat_test024.c \
		ipa_nat_test025.c \
		ipa_nat_test999.c \
		main.c

bin_PROGRAMS  =  ipanattest

requiredlibs =  ../src/libipanat.la

ipanattest_LDADD =  $(requiredlibs)

LOCAL_MODULE := libipanat
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)
