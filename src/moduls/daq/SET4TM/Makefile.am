EXTRA_DIST = *.h po/*

#!!! Include module to OpenSCADA core process. It is typical!
if SET4TMIncl
#!!! The module result library name
noinst_LTLIBRARIES = daq_SET4TM.la
daq_SET4TM_la_CXXFLAGS = -DMOD_INCL -fpic
#!!! Libtool util's flags. It is typical!
daq_SET4TM_la_LIBTOOLFLAGS = --tag=disable-shared
#!!! The module link flags
daq_SET4TM_la_LDFLAGS = -module
else
#!!! The module result library name
oscd_modul_LTLIBRARIES = daq_SET4TM.la
daq_SET4TM_la_CXXFLAGS =
#!!! Libtool util's flags. It is typical!
daq_SET4TM_la_LIBTOOLFLAGS = --tag=disable-static
#!!! The module link flags
daq_SET4TM_la_LDFLAGS = -module -avoid-version $(top_builddir)/src/liboscada.la
endif

#!!! The module compiler flags
daq_SET4TM_la_CXXFLAGS += $(SET4TM_CFLAGS)
#!!! The module link flags
daq_SET4TM_la_LDFLAGS += $(SET4TM_LDLAGS)
#!!! The module source files list
daq_SET4TM_la_SOURCES = module.cpp


#!!! The module name for translation files
I18N_mod = $(oscd_modulpref)SET4TM
include ../../../../I18N.mk
