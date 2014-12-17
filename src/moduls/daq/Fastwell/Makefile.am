EXTRA_DIST = *.h po/*

if FastwellIncl
noinst_LTLIBRARIES = daq_Fastwell.la
daq_Fastwell_la_CXXFLAGS = -DMOD_INCL -fpic
daq_Fastwell_la_LIBTOOLFLAGS = --tag=disable-shared
daq_Fastwell_la_LDFLAGS = -module
else
oscd_modul_LTLIBRARIES = daq_Fastwell.la
daq_Fastwell_la_CXXFLAGS =
daq_Fastwell_la_LIBTOOLFLAGS = --tag=disable-static
daq_Fastwell_la_LDFLAGS = -module -avoid-version $(top_builddir)/src/liboscada.la
endif

daq_Fastwell_la_SOURCES = module.cpp


I18N_mod = $(oscd_modulpref)Fastwell
include ../../../../I18N.mk
