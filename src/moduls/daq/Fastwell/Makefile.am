EXTRA_DIST = *.h po/*

oscd_modul_LTLIBRARIES = daq_Fastwell.la
daq_Fastwell_la_CXXFLAGS = $(FASTWELL_CFLAGS)
daq_Fastwell_la_LIBTOOLFLAGS = --tag=disable-static
daq_Fastwell_la_LDFLAGS = -module -avoid-version $(top_builddir)/src/liboscada.la

daq_Fastwell_la_LDFLAGS += $(FASTWELL_LDLAGS)
daq_Fastwell_la_SOURCES = module.cpp


I18N_mod = $(oscd_modulpref)Fastwell
include ../../../../I18N.mk
