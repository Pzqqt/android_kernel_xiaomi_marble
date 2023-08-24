ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign

AM_CFLAGS = -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -I../inc
#AM_CFLAGS += -g -DDEBUG -DNAT_DEBUG

common_CFLAGS =  -DUSE_GLIB @GLIB_CFLAGS@
common_LDFLAGS = -lrt @GLIB_LIBS@

library_includedir = $(pkgincludedir)

cpp_sources = ipa_nat_map.cpp

c_sources   = ipa_nat_drv.c \
              ipa_nat_drvi.c \
              ipa_nat_utils.c \
              ipa_table.c \
              ipa_mem_descriptor.c \
              ipa_ipv6ct.c \
              ipa_nat_statemach.c

library_include_HEADERS = ../inc/ipa_nat_drvi.h \
                          ../inc/ipa_nat_drv.h \
                          ../inc/ipa_nat_utils.h \
                          ../inc/ipa_table.h \
                          ../inc/ipa_mem_descriptor.h \
                          ../inc/ipa_ipv6ct.h \
                          ../inc/ipa_nat_statemach.h \
                          ../inc/ipa_nat_map.h

lib_LTLIBRARIES = libipanat.la
libipanat_la_C = @C@
libipanat_la_CC = @CC@
libipanat_la_SOURCES = $(c_sources) $(cpp_sources)
libipanat_la_CFLAGS = $(AM_CFLAGS) $(common_CFLAGS)
libipanat_la_CXXFLAGS = $(AM_CFLAGS) $(common_CPPFLAGS)
libipanat_la_LDFLAGS = -shared $(common_LDFLAGS) -version-info 1:0:0
