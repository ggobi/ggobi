dnl This is the configuration script for ggobi

# Configure the version information here
m4_define([ggobi_release_date], ["Jan 16 2007"])
m4_define([ggobi_major_version], [2])
m4_define([ggobi_minor_version], [2])
m4_define([ggobi_micro_version], [0])
m4_define([ggobi_version],
          [ggobi_major_version.ggobi_minor_version.ggobi_micro_version])

# Initialize autoconf
AC_INIT(ggobi, ggobi_version, ggobi-help@lists.ggobi.org)
AC_PREREQ(2.59)

# Tell autoconf where our sources are
AC_CONFIG_SRCDIR([src/ggobi.c])

# Setup the header where our checks will be made available to the source
AC_CONFIG_HEADER(src/config.h)

# We will store build scripts in the 'config' directory, since it is cleaner
AC_CONFIG_AUX_DIR(config)
AC_SUBST(ac_aux_dir)

# Initialize automake
AM_INIT_AUTOMAKE

# Figure out where we are installing immediately
if test "${prefix}" = "NONE"; then
  prefix=$ac_default_prefix
fi

# Remember the prefix
AC_DEFINE_UNQUOTED([GGOBI_PREFIX], "$prefix", [GGobi installation prefix])

# Remember where our GGobi data is installed
# We evaluate datadir twice to resolve any symbols inside datadir
GGOBI_DATADIR_TMP=`eval echo "${datadir}"`
GGOBI_DATADIR=`eval echo "${GGOBI_DATADIR_TMP}"`
AC_DEFINE_UNQUOTED([GGOBI_DATADIR], "$GGOBI_DATADIR", [Location of GGobi data installation])

# Initialize gettext
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.14.5)
MKINSTALLDIRS="../$ac_aux_dir/mkinstalldirs"
AC_SUBST(MKINSTALLDIRS)
GGOBI_LOCALEDIR="${GGOBI_DATADIR}/locale"
AC_DEFINE_UNQUOTED([GGOBI_LOCALEDIR], "$GGOBI_LOCALEDIR", [Location of locale files])

MAJOR_VERSION=ggobi_major_version
MINOR_VERSION=ggobi_minor_version
MICRO_VERSION=ggobi_micro_version

GGOBI_RELEASE_DATE=ggobi_release_date

AC_SUBST(GGOBI_RELEASE_DATE)
AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(MICRO_VERSION)

dnl These go into config.h as #define entries (used in about dialog).
AC_DEFINE_UNQUOTED([GGOBI_RELEASE_DATE], "${GGOBI_RELEASE_DATE}", [Date of GGobi's release])
AC_DEFINE_UNQUOTED([MAJOR_VERSION], $MAJOR_VERSION, [The major x.*.* version])
AC_DEFINE_UNQUOTED([MINOR_VERSION], $MINOR_VERSION, [The minor *.x.* version])
AC_DEFINE_UNQUOTED([MICRO_VERSION], $MICRO_VERSION, [The micro *.*.x version])

# Configure options (debug? mudflap? any plugins?)

AC_ARG_ENABLE(debug, [  --enable-debug          Compile with debugging symbols])
AC_ARG_ENABLE(debug-extra, [  --enable-debug-extra    Compile with debugging symbols and extra warnings])
AC_ARG_ENABLE(mudflap, [  --enable-mudflap        Compile with mudflap instrumentation])

PLUGIN_DIRS=""

AC_ARG_WITH(all-plugins, [--with-all-plugins  Enable all (supported) plugins])

#AC_ARG_WITH(mysql,
#  --with-mysql        Enable plugin for reading data from MySQL databases,
#  MYSQL_PLUGIN=1; PLUGIN_DIRS="MySQL")

#AC_ARG_WITH(postgres,
#  --with-postgres          Enable plugin reading data from Postgres databases.,
#  POSTGRES_PLUGIN=1; PLUGIN_DIRS="${PLUGIN_DIRS} Postgres")

AC_ARG_WITH(dataviewer,
  AC_HELP_STRING([--with-dataviewer], [Enable plugin for viewing data in a datagrid.]),
  DATAVIEWER_PLUGIN=1;  PLUGIN_DIRS="${PLUGIN_DIRS} DataViewer")

AC_ARG_WITH(describedisplay,
  --with-describedisplay     Convert GGobi plots to R plots.,
  DESCRIBEDISPLAY_PLUGIN=1;  PLUGIN_DIRS="${PLUGIN_DIRS} DescribeDisplay")

AC_ARG_WITH(graphlayout,
  [--with-graphlayout          Enable GraphLayout plugin],
  [echo "With graphlayout"; GRAPHLAYOUT_PLUGIN=1;  PLUGIN_DIRS="${PLUGIN_DIRS} GraphLayout"])

AC_ARG_WITH(ggvis,
  --with-ggvis          Enable ggvis plugin,
  [GGVIS_PLUGIN=1;  PLUGIN_DIRS="${PLUGIN_DIRS} ggvis"])

AC_ARG_WITH(graphaction,
  --with-graphaction          Enable graph operations plugin,
  [GRAPHACTION_PLUGIN=1;  PLUGIN_DIRS="${PLUGIN_DIRS} GraphAction"])

AC_ARG_WITH(varcloud,
  --with-varcloud          Enable variogram cloud plugin,
  [VARCLOUD_PLUGIN=1;  PLUGIN_DIRS="${PLUGIN_DIRS} VarCloud"])

AC_ARG_WITH(plugins, [--with-plugins          With plugins in a quoted, space-separated list])

if test "$with_plugins" != "" && test "$with_plugins" != "yes" && test "$with_plugins" != "y" &&  test "$with_plugins" != "no" && test "$with_plugins" != "n" ; then
 PLUGIN_DIRS=$with_plugins
fi

if test -n "${with_all_plugins}" &&  test "${with_all_plugins}" != "no" ; then
 PLUGIN_DIRS="DataViewer DescribeDisplay GraphLayout GraphAction VarCloud ggvis"
fi

# This is obviously ugly, but AC_CONFIG_SUBDIRS *requires* a literal
for p in ${PLUGIN_DIRS} ; do
if test "${p}" = "DataViewer"; then
  AC_CONFIG_SUBDIRS(plugins/DataViewer)
fi
if test "${p}" = "VarCloud"; then
  AC_CONFIG_SUBDIRS(plugins/VarCloud)
fi
if test "${p}" = "ggvis"; then
  AC_CONFIG_SUBDIRS(plugins/ggvis)
fi
if test "${p}" = "GraphAction"; then
  AC_CONFIG_SUBDIRS(plugins/GraphAction)
fi
if test "${p}" = "GraphLayout"; then
  AC_CONFIG_SUBDIRS(plugins/GraphLayout)
fi
if test "${p}" = "DescribeDisplay"; then
  AC_CONFIG_SUBDIRS(plugins/DescribeDisplay)
fi
done

AC_SUBST(PLUGIN_DIRS)

# Make sure we a C compiler and symbolic link capability
AC_PROG_CC
AC_PROG_LN_S

# Set up libtool to create only dynamic libraries
# and to support DLLs on Windows. Also build libltdl for loading plugins.
AC_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AC_LIBLTDL_CONVENIENCE
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

# Configure libltdl build
AC_SUBST(INCLTDL)
AC_SUBST(LIBLTDL)
AC_CONFIG_SUBDIRS(libltdl)

# Configure for building against local libintl
AC_SUBST(INCINTL)

# Check for questionable functions (kind of paranoid)
AC_FUNC_MALLOC
AC_CHECK_FUNCS([pow sqrt strchr strrchr strstr memset])

# Check for some non-standard header files
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h strings.h unistd.h])

# Make sure our compiler doesn't suck
AC_C_CONST
AC_HEADER_STDBOOL

# Debug?
if test -n "${enable_debug}" && ! test "${enable_debug}" = "no" ;  then
 DEBUG_CFLAGS="-g -Wall"
fi
if test -n "${enable_debug_extra}" && ! test "${enable_debug_extra}" = "no" ;  then
 DEBUG_CFLAGS="-g -Wall -Wextra"
fi
AC_SUBST(DEBUG_CFLAGS)

# Mudflap?
if test -n "${enable_mudflap}" && ! test "${enable_mudflap}" = "no" ;  then
 MUDFLAP_CFLAGS="-fmudflap"
 MUDFLAP_LDFLAGS="-fmudflap -lmudflap"
fi
AC_SUBST(MUDFLAP_CFLAGS)
AC_SUBST(MUDFLAP_LDFLAGS)

# Find GOB2 (don't fail if not found, not necessary for distributed builds)
AC_PATH_PROG(GOB2, gob2,,$PATH)

# Find valac (the vala compiler)
AC_PATH_PROG(VALAC, valac,,$PATH)
AC_SUBST(VALAC)

AM_CONDITIONAL(HAS_VALA, test -n "${VALAC}")

PKG_PROG_PKG_CONFIG(0.21)

# FIND GTK2

 echo "Looking for gtk2"

PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= 2.6.0])

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

# Stuff for gtk doc
GTK_DOC_CHECK(1.4)
AC_CONFIG_MACRO_DIR(m4)

echo "Setting gtk flags: cflags=$GTK_CFLAGS, libs=$GTK_LIBS"

# Find libxml2

PKG_CHECK_MODULES(LIBXML, [libxml-2.0])

AC_SUBST(LIBXML_CFLAGS)
AC_SUBST(LIBXML_LIBS)

# Find gsf

PKG_CHECK_MODULES(LIBGSF, [libgsf-1])
AC_SUBST(LIBGSF_CFLAGS)
AC_SUBST(LIBGSF_LIBS)

# Find gsl

#PKG_CHECK_MODULES(GSL, [gsl])
AC_SUBST(GSL_CFLAGS)
AC_SUBST(GSL_LIBS)

# Are we on Windows?

AC_MSG_CHECKING([for native Win32])
case "$host" in
  *-*-mingw*)
    os_win32=yes
    ;;
  *)
    os_win32=no
    ;;
esac
echo "${os_win32}"
AM_CONDITIONAL(OS_WIN32, test "$os_win32" = "yes")

# Find glitz

if test "$os_win32" != "yes" ; then
  PKG_CHECK_MODULES(GLITZ, [glitz-glx], [HAVE_GLITZ=1], [HAVE_GLITZ=0])
else
  PKG_CHECK_MODULES(GLITZ, [glitz-wgl], [HAVE_GLITZ=1], [HAVE_GLITZ=0])
fi

AC_SUBST(GLITZ_CFLAGS)
AC_SUBST(GLITZ_LIBS)

 dnl Process the different .in files, substituting the values computed here in the configuration
 dnl script for the corresponding @variable@ in these files.
AC_CONFIG_FILES(Makefile src/Makefile plugins/Makefile ggobi.pc ggobi-system.pc po/Makefile.in doc/Makefile doc/reference/Makefile)
AC_OUTPUT

echo "*******************************"
echo ""
echo "GGobi"
echo " Plugins:                ${PLUGIN_DIRS}"
echo " Installation directory: $prefix"
echo ""
echo "*******************************"

