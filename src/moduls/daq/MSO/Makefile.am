EXTRA_DIST = *.h po/*

if MSOIncl
noinst_LTLIBRARIES = daq_MSO.la
daq_MSO_la_CXXFLAGS = -DMOD_INCL -fpic
daq_MSO_la_LIBTOOLFLAGS = --tag=disable-shared
daq_MSO_la_LDFLAGS = -module
else
oscd_modul_LTLIBRARIES = daq_MSO.la
daq_MSO_la_LIBTOOLFLAGS = --tag=disable-static
daq_MSO_la_LDFLAGS = -module -avoid-version $(top_builddir)/src/liboscada.la
endif

daq_MSO_la_SOURCES = moduls.cpp mso_daq.cpp mso_prt.cpp MezTT.cpp MezTC.cpp MezTU.cpp MezTI.cpp MezTR.cpp MSOparam.cpp

I18N_mod = $(oscd_modulpref)MSO
include ../../../../I18N.mk
