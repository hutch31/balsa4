## Process this file with automake to produce Makefile.in

AM_CFLAGS = -I$(prefix)/include -I$(top_srcdir)

standard_ldflags = -static -avoid-version
shared_sources = builtin_verilog.c

if HAVE_VCS_HEADERS
VCS_LIB = libbalsasimverilog_vcs.la
libbalsasimverilog_vcs_la_SOURCES = $(shared_sources)
libbalsasimverilog_vcs_la_LDFLAGS = $(standard_ldflags)
libbalsasimverilog_vcs_la_CFLAGS = -I@vcs_headers_dir@ $(AM_CFLAGS) -DVCS $(VCS_HEADER_DEF)
tabdir = $(datadir)/verilog/vcs
tab_DATA = builtin.tab
if HAVE_VCS_VPI_HEADERS
VCS_HEADER_DEF = -DVPI
else
if HAVE_VCS_ACC_HEADERS
VCS_HEADER_DEF = -DACC
endif
endif
endif

if HAVE_CDS_HEADERS
CDS_LIB = libbalsasimverilog_vxl.la libbalsasimverilog_ncv.la
libbalsasimverilog_vxl_la_SOURCES = $(shared_sources)
libbalsasimverilog_ncv_la_SOURCES = $(shared_sources)
libbalsasimverilog_vxl_la_LDFLAGS = $(standard_ldflags)
libbalsasimverilog_vxl_la_CFLAGS = -I@cds_headers_dir@ $(AM_CFLAGS) -DVXL $(CDS_HEADER_DEF)
libbalsasimverilog_ncv_la_LDFLAGS = $(standard_ldflags)
libbalsasimverilog_ncv_la_CFLAGS = -I@cds_headers_dir@ $(AM_CFLAGS) -DNCV $(CDS_HEADER_DEF)
if HAVE_CDS_VPI_HEADERS
CDS_HEADER_DEF = -DVPI
else
if HAVE_CDS_ACC_HEADERS
CDS_HEADER_DEF = -DACC
endif
endif
endif

if HAVE_ICARUS_HEADERS
ICARUS_LIB = libbalsasimverilog_icarus.la
libbalsasimverilog_icarus_la_SOURCES = $(shared_sources)
libbalsasimverilog_icarus_la_LDFLAGS = $(standard_ldflags)
libbalsasimverilog_icarus_la_CFLAGS = -I@icarus_headers_dir@ $(AM_CFLAGS) -DICARUS $(ICARUS_HEADER_DEF)
if HAVE_ICARUS_VPI_HEADERS
ICARUS_HEADER_DEF = -DVPI
else
if HAVE_ICARUS_ACC_HEADERS
ICARUS_HEADER_DEF = -DACC
endif
endif
endif

if HAVE_CVER_HEADERS
CVER_LIB = libbalsasimverilog_cver.la
libbalsasimverilog_cver_la_SOURCES = $(shared_sources)
libbalsasimverilog_cver_la_LDFLAGS = $(standard_ldflags)
libbalsasimverilog_cver_la_CFLAGS = -I@cver_headers_dir@ $(AM_CFLAGS) -DCVER $(CVER_HEADER_DEF)
if HAVE_CVER_VPI_HEADERS
CVER_HEADER_DEF = -DVPI
else
if HAVE_CVER_ACC_HEADERS
CVER_HEADER_DEF = -DACC
endif
endif
endif

if HAVE_MODELSIM_HEADERS
MODELSIM_LIB = libbalsasimverilog_modelsim.la
libbalsasimverilog_modelsim_la_SOURCES = $(shared_sources)
libbalsasimverilog_modelsim_la_LDFLAGS = $(standard_ldflags)
libbalsasimverilog_modelsim_la_CFLAGS = -I@modelsim_headers_dir@ $(AM_CFLAGS) -DMODELSIM $(MODELSIM_HEADER_DEF)
if HAVE_MODELSIM_VPI_HEADERS
MODELSIM_HEADER_DEF = -DVPI
else
if HAVE_MODELSIM_ACC_HEADERS
MODELSIM_HEADER_DEF = -DACC
endif
endif
endif

lib_LTLIBRARIES = \
	$(VCS_LIB) \
	$(CDS_LIB) \
	$(MODELSIM_LIB) \
	$(ICARUS_LIB) \
	$(CVER_LIB)

EXTRA_DIST = builtin.tab
