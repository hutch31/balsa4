dnl Process this file with autoconf to produce a configure script

AC_INIT(README)

AM_INIT_AUTOMAKE(balsa-sim-verilog, 4.0)
AC_PATH_PROG(BALSA_CONFIG, balsa-config)
AM_CONFIG_HEADER(config.h)

CFLAGS="-fPIC $CFLAGS"

if test ${BALSA_CONFIG}; then :
else
	AC_MSG_ERROR([cannot find required balsa-config, install Balsa and add it to your path])
fi

AC_MSG_CHECKING(installed Balsa directory)
dnl Don't allow use of --prefix
ac_default_prefix=`${BALSA_CONFIG} -d`
prefix=${ac_default_prefix}
AC_MSG_RESULT(${ac_default_prefix})

supported_simulators=:

AC_ARG_WITH(vcs-includes, 
	[AC_HELP_STRING([--with-vcs-includes="/path/to/include/directory"],
					[path to Synopsys VCS include files])], 
	[
		if test -d "${withval}" ; then
			vcs_headers_dir=${withval}
			AC_MSG_CHECKING(Synopsys VCS `vcs_acc_user.h')
			if test -f "${withval}/vcs_acc_user.h" ; then
				has_some_verilog_headers=true
				vcs_acc_headers=true
				supported_simulators=${supported_simulators}:vcs
				AC_DEFINE_UNQUOTED(HAVE_VCS_ACC_USER_H, 1,[If Synopsys VCS Verilog header files for tf_* and acc_* functions were found])
				AC_MSG_RESULT(${withval}/vcs_acc_user.h)
			else
				AC_MSG_RESULT(not found)
			fi
			AC_MSG_CHECKING(Synopsys VCS `vpi_user.h')
			if test -f "${withval}/vpi_user.h" ; then
				has_some_verilog_headers=true
				vcs_vpi_headers=true
				supported_simulators=${supported_simulators}:vcs
				AC_DEFINE_UNQUOTED(HAVE_VCS_VPI_USER_H, 1,[If Synopsys VCS Verilog header files for vpi_* functions were found])
				AC_MSG_RESULT(${withval}/vpi_user.h)
			else
				AC_MSG_RESULT(not found)
			fi
		else
			AC_MSG_WARN([Pathname given for Synopsys VCS Verilog headers appears to be bad - it does not exist or is not a directory])
		fi
	]
)
AM_CONDITIONAL([HAVE_VCS_HEADERS], [test -n "${vcs_acc_headers}" -o -n "${vcs_vpi_headers}"])
AM_CONDITIONAL([HAVE_VCS_ACC_HEADERS], [test -n "${vcs_acc_headers}"])
AM_CONDITIONAL([HAVE_VCS_VPI_HEADERS], [test -n "${vcs_vpi_headers}"])
AC_SUBST(vcs_headers_dir)

AC_ARG_WITH(cds-includes, 
	[AC_HELP_STRING([--with-cds-includes="/path/to/include/directory"],
					[path to Cadence Verilog-XL and NCverilog include files])], 
	[
		if test -d "${withval}" ; then
			cds_headers_dir=${withval}
			AC_MSG_CHECKING(Cadence Verilog-XL and NCverilog `vxl_acc_user.h')
			if test -f "${withval}/vxl_acc_user.h" ; then
				has_some_verilog_headers=true
				cds_acc_headers=true
				cds_pliload_plusopt=loadpli1
				supported_simulators=${supported_simulators}:vxl:ncv
				AC_DEFINE_UNQUOTED(HAVE_CDS_ACC_USER_H, 1,[If Cadence Verilog header files for tf_* and acc_* functions were found])
				AC_MSG_RESULT(${withval}/vxl_acc_user.h)
			else
				AC_MSG_RESULT(not found)
			fi
			AC_MSG_CHECKING(Cadence Verilog-XL and NCverilog `vpi_user.h')
			if test -f "${withval}/vpi_user.h" ; then
				has_some_verilog_headers=true
				cds_vpi_headers=true
				cds_pliload_plusopt=loadvpi
				supported_simulators=${supported_simulators}:vxl:ncv
				AC_DEFINE_UNQUOTED(HAVE_CDS_VPI_USER_H, 1,[If Cadence Verilog header files for vpi_* functions were found])
				AC_MSG_RESULT(${withval}/vpi_user.h)
			else
				AC_MSG_RESULT(not found)
			fi
		else
			AC_MSG_WARN([Pathname given for Cadence Verilog headers appears to be bad - it does not exist or is not a directory])
		fi
	]
)
AM_CONDITIONAL([HAVE_CDS_HEADERS], [test -n "${cds_acc_headers}" -o -n "${cds_vpi_headers}"])
AM_CONDITIONAL([HAVE_CDS_ACC_HEADERS], [test -n "${cds_acc_headers}"])
AM_CONDITIONAL([HAVE_CDS_VPI_HEADERS], [test -n "${cds_vpi_headers}"])
AC_SUBST(cds_headers_dir)
AC_SUBST(cds_pliload_plusopt)

AC_ARG_WITH(icarus-includes, 
	[AC_HELP_STRING([--with-icarus-includes="/path/to/include/directory"],
					[path to Icarus Verilog include files])], 
	[
		if test -d "${withval}" ; then
			icarus_headers_dir=${withval}
			AC_MSG_CHECKING(Icarus Verilog `acc_user.h')
			if test -f "${withval}/acc_user.h" ; then
				has_some_verilog_headers=true
				icarus_acc_headers=true
				supported_simulators=${supported_simulators}:icarus
				AC_DEFINE_UNQUOTED(HAVE_ICARUS_ACC_USER_H, 1,[If Icarus Verilog header files for tf_* and acc_* functions were found])
				AC_MSG_RESULT(${withval}/acc_user.h)
			else
				AC_MSG_RESULT(not found)
			fi
			AC_MSG_CHECKING(Icarus Verilog `vpi_user.h')
			if test -f "${withval}/vpi_user.h" ; then
				has_some_verilog_headers=true
				icarus_vpi_headers=true
				supported_simulators=${supported_simulators}:icarus
				AC_DEFINE_UNQUOTED(HAVE_ICARUS_VPI_USER_H, 1,[If Icarus Verilog header files for vpi_* functions were found])
				AC_MSG_RESULT(${withval}/vpi_user.h)
			else
				AC_MSG_RESULT(not found)
			fi
		else
			AC_MSG_WARN([Pathname given for Icarus Verilog headers appears to be bad - it does not exist or is not a directory])
		fi
	]
)
AM_CONDITIONAL([HAVE_ICARUS_HEADERS], [test -n "${icarus_acc_headers}" -o -n "${icarus_vpi_headers}"])
AM_CONDITIONAL([HAVE_ICARUS_ACC_HEADERS], [test -n "${icarus_acc_headers}"])
AM_CONDITIONAL([HAVE_ICARUS_VPI_HEADERS], [test -n "${icarus_vpi_headers}"])
AC_SUBST(icarus_headers_dir)

AC_ARG_WITH(icarus-libs, 
	[AC_HELP_STRING([--with-icarus-libs="/path/to/lib/directory"],
					[path to Icarus Verilog library files])], 
	[ icarus_libs_dir=${withval} ]
)
AC_SUBST(icarus_libs_dir)

AC_ARG_WITH(cver-includes, 
	[AC_HELP_STRING([--with-cver-includes="/path/to/include/directory"],
					[path to Cver include files])], 
	[
		if test -d "${withval}" ; then
			cver_headers_dir=${withval}
			AC_MSG_CHECKING(Cver `acc_user.h')
			if test -f "${withval}/acc_user.h" ; then
				has_some_verilog_headers=true
				cver_acc_headers=true
				supported_simulators=${supported_simulators}:cver
				AC_DEFINE_UNQUOTED(HAVE_CVER_ACC_USER_H, 1,[If Cver header files for tf_* and acc_* functions were found])
				AC_MSG_RESULT(${withval}/acc_user.h)
			else
				AC_MSG_RESULT(not found)
			fi
			AC_MSG_CHECKING(Cver `vpi_user.h')
			if test -f "${withval}/vpi_user.h" ; then
				has_some_verilog_headers=true
				cver_vpi_headers=true
				supported_simulators=${supported_simulators}:cver
				AC_DEFINE_UNQUOTED(HAVE_CVER_VPI_USER_H, 1,[If Cver header files for vpi_* functions were found])
				AC_MSG_RESULT(${withval}/vpi_user.h)
			else
				AC_MSG_RESULT(not found)
			fi
		else
			AC_MSG_WARN([Pathname given for Cver headers appears to be bad - it does not exist or is not a directory])
		fi
	]
)
AM_CONDITIONAL([HAVE_CVER_HEADERS], [test -n "${cver_acc_headers}" -o -n "${cver_vpi_headers}"])
AM_CONDITIONAL([HAVE_CVER_ACC_HEADERS], [test -n "${cver_acc_headers}"])
AM_CONDITIONAL([HAVE_CVER_VPI_HEADERS], [test -n "${cver_vpi_headers}"])
AC_SUBST(cver_headers_dir)

AC_ARG_WITH(modelsim-includes, 
	[AC_HELP_STRING([--with-modelsim-includes="/path/to/include/directory"],
					[path to Modelsim Verilog include files])], 
	[
		if test -d "${withval}" ; then
			modelsim_headers_dir=${withval}
			AC_MSG_CHECKING(Modelsim Verilog `acc_user.h')
			if test -f "${withval}/acc_user.h" ; then
				has_some_verilog_headers=true
				modelsim_acc_headers=true
				supported_simulators=${supported_simulators}:modelsim
				AC_DEFINE_UNQUOTED(HAVE_MODELSIM_ACC_USER_H, 1,[If Modelsim Verilog header files for tf_* and acc_* functions were found])
				AC_MSG_RESULT(${withval}/acc_user.h)
			else
				AC_MSG_RESULT(not found)
			fi
			AC_MSG_CHECKING(Modelsim Verilog `vpi_user.h')
			if test -f "${withval}/vpi_user.h" ; then
				has_some_verilog_headers=true
				modelsim_vpi_headers=true
				supported_simulators=${supported_simulators}:modelsim
				AC_DEFINE_UNQUOTED(HAVE_MODELSIM_VPI_USER_H, 1,[If Modelsim Verilog header files for vpi_* functions were found])
				AC_MSG_RESULT(${withval}/vpi_user.h)
			else
				AC_MSG_RESULT(not found)
			fi
		else
			AC_MSG_WARN([Pathname given for Modelsim Verilog headers appears to be bad - it does not exist or is not a directory])
		fi
	]
)
AM_CONDITIONAL([HAVE_MODELSIM_HEADERS], [test -n "${modelsim_acc_headers}" -o -n "${modelsim_vpi_headers}"])
AM_CONDITIONAL([HAVE_MODELSIM_ACC_HEADERS], [test -n "${modelsim_acc_headers}"])
AM_CONDITIONAL([HAVE_MODELSIM_VPI_HEADERS], [test -n "${modelsim_vpi_headers}"])
AC_SUBST(modelsim_headers_dir)


if test -z ${has_some_verilog_headers} ; then
	AC_MSG_WARN([No Verilog header files found - falling back on extremely dodgy guesswork])
fi
AM_CONDITIONAL([HAVE_NO_HEADERS], [test -z "${has_some_verilog_headers}"])

AC_SUBST(supported_simulators)

dnl Checks for programs.
AC_PROG_CC
AM_PROG_CC_STDC
AC_PROG_INSTALL
AC_PATH_PROG(NM, nm)

AM_PROG_LIBTOOL

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(inttypes.h)

AC_DEFINE_UNQUOTED(BALSAHOME, "${prefix}", "Compiled Balsa install directory")
AC_DEFINE_UNQUOTED(DEFAULT_BALSAPATH, "${balsadatadir}:.", "Path to installed Balsa/Breeze files")

balsadatadir=${prefix}/share
AC_SUBST(balsadatadir)

AC_OUTPUT([
	Makefile
	bin/Makefile
	bin/balsa-verilog-make-builtin-lib
	bin/balsa-verilog-sim
	libs/Makefile
])

echo
echo Supported simulators:
echo ${supported_simulators} | tr '[:]' '[\012]' | sort | uniq | grep -v '^$' | sed 's/^/        /'
echo
