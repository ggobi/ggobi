dnl This is the configuration script for ggobi

# Initialize autoconf
AC_INIT(ggobi, 2.1, ggobi-help@lists.ggobi.org)
AC_PREREQ(2.59)

# Tell autoconf where our sources are
AC_CONFIG_SRCDIR([src/transform_ui.c])

# Setup the header where our checks will be made available to the source
AC_CONFIG_HEADER(src/config.h)

# We will store build scripts in the 'config' directory, since it is cleaner
AC_CONFIG_AUX_DIR(config)
AC_SUBST(ac_aux_dir)

# Initialize automake
AM_INIT_AUTOMAKE

# Get the version information.
. VersionInfo

if test -d .svn ; then
 GGOBI_VERSION_STRING="SVN version (`date`)"
fi

AC_SUBST(GGOBI_VERSION_STRING)
AC_SUBST(GGOBI_RELEASE_DATE)
AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(PATCH_LEVEL)

dnl These go into config.h as #define entries (used in about dialog).
AC_DEFINE_UNQUOTED([GGOBI_VERSION_STRING], "$GGOBI_VERSION_STRING", [GGobi's full version])
AC_DEFINE_UNQUOTED([GGOBI_RELEASE_DATE], "$GGOBI_RELEASE_DATE", [Data of GGobi's release])
AC_DEFINE_UNQUOTED([MAJOR_VERSION], $MAJOR_VERSION, [The major x.*.* version])
AC_DEFINE_UNQUOTED([MINOR_VERSION], $MINOR_VERSION, [The minor *.x.* version])
AC_DEFINE_UNQUOTED([PATCH_LEVEL], $PATCH_LEVEL, [The micro *.*.x version])

# Configure options (debug? local? any plugins?)

AC_ARG_ENABLE(debug, [--enable-debug          Compile with debugging symbols])
AC_ARG_ENABLE(local, [--enable-local          Configure for use directly from this directory tree.])

PLUGIN_DIRS=""

AC_ARG_WITH(all-plugins, [--with-all-plugins  Enable all (supported) plugins])

#AC_ARG_WITH(mysql,
#  --with-mysql        Enable plugin for reading data from MySQL databases, 
#  MYSQL_PLUGIN=1; PLUGIN_DIRS="MySQL"; ENABLE_PLUGINS=1)

#AC_ARG_WITH(postgres,
#  --with-postgres          Enable plugin reading data from Postgres databases., 
#  POSTGRES_PLUGIN=1; PLUGIN_DIRS="${PLUGIN_DIRS} Postgres"; ENABLE_PLUGINS=1)

AC_ARG_WITH(dataviewer,
  --with-dataviewer          Enable plugin for viewing data in a datagrid., 
  DATAVIEWER_PLUGIN=1;  PLUGIN_DIRS="${PLUGIN_DIRS} DataViewer"; ENABLE_PLUGINS=1)

AC_ARG_WITH(describedisplay,
  --with-describedisplay     Convert GGobi plots to R plots., 
  DESCRIBEDISPLAY_PLUGIN=1;  PLUGIN_DIRS="${PLUGIN_DIRS} DescribeDisplay"; ENABLE_PLUGINS=1)
  
AC_ARG_WITH(graphlayout,
  [--with-graphlayout          Enable GraphLayout plugin], 
  [echo "With graphlayout"; GRAPHLAYOUT_PLUGIN=1;  PLUGIN_DIRS="${PLUGIN_DIRS} GraphLayout"; ENABLE_PLUGINS=1])

AC_ARG_WITH(ggvis,
  --with-ggvis          Enable ggvis plugin, 
  [GGVIS_PLUGIN=1;  PLUGIN_DIRS="${PLUGIN_DIRS} ggvis"; ENABLE_PLUGINS=1])

AC_ARG_WITH(graphaction,
  --with-graphaction          Enable graph operations plugin, 
  [GRAPHACTION_PLUGIN=1;  PLUGIN_DIRS="${PLUGIN_DIRS} GraphAction"; ENABLE_PLUGINS=1])

AC_ARG_WITH(varcloud,
  --with-varcloud          Enable variogram cloud plugin, 
  [VARCLOUD_PLUGIN=1;  PLUGIN_DIRS="${PLUGIN_DIRS} VarCloud"; ENABLE_PLUGINS=1])
AC_ARG_WITH(plugins, [--with-plugins          Enable all the plugins (no)], ENABLE_ALL_PLUGINS=1)

if test "$with_plugins" != "" && test "$with_plugins" != "yes" && test "$with_plugins" != "y" &&  test "$with_plugins" != "no" && test "$with_plugins" != "n" ; then
 PLUGIN_DIRS=$with_plugins
fi

if test -n "${with_all_plugins}" &&  test "${with_all_plugins}" != "no" ; then
 PLUGIN_DIRS="DataViewer DescribeDisplay GraphLayout GraphAction VarCloud ggvis"
fi

# This is obviously ugly, but AC_CONFIG_SUBDIRS *requires* a literal
for p in ${PLUGIN_DIRS} ; do 
if test "${p}" = "DataViewer"; then
  echo "DATAVIEWER PLUGIN"
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
AC_PROG_LIBTOOL

# Configure libltdl build
AC_SUBST(INCLTDL)
AC_SUBST(LIBLTDL)
AC_CONFIG_SUBDIRS(libltdl)

# Check for questionable functions (kind of paranoid)
AC_FUNC_MALLOC
AC_CHECK_FUNCS([pow sqrt strchr strrchr strstr memset])

# Check for some non-standard header files
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h strings.h unistd.h])

# Make sure our compiler doesn't suck
AC_C_CONST
AC_HEADER_STDBOOL

# A local install basically configures ggobi.pc to let us build plugins against
# this GGobi instead of the system one

echo "Enable local ${enable_local}"

if test -n "${enable_local}" &&  test "${enable_local}" != "no" ; then
 echo "Installing into build directory."
 AC_PREFIX_DEFAULT(`pwd`)
 GGOBI_CFLAGS="-I${ac_default_prefix}/src"
else
 GGOBI_CFLAGS="-I${prefix}/include/ggobi"
fi

AC_SUBST(GGOBI_CFLAGS)

# Debug?
if test -n "${enable_debug}" && ! test "${enable_debug}" = "no" ;  then
 SRC_DEBUG="-g -Wall"
fi
AC_SUBST(SRC_DEBUG)

# Find uname binary

AC_PATH_PROG(UNAME, uname,,$PATH)
if test -z "${UNAME}" ; then
   AC_MSG_ERROR(Cannot find uname)
fi

# Find GOB2 (don't fail if not found, not necessary for distributed builds)
AC_PATH_PROG(GOB2, gob2,,$PATH)

# FIND GTK2

 echo "Looking for gtk2"
 AC_PATH_PROG(PKG_CONFIG, pkg-config,, $PATH)
 if test -z "${PKG_CONFIG}" ; then
   AC_MSG_ERROR(Cannot find pkg-config)
 fi

PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= 2.6.0],,[AC_MSG_ERROR(You must have GTK+ >= 2.6.0)])

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

echo "Setting gtk flags: cflags=$GTK_CFLAGS, libs=$GTK_LIBS"

PKG_CHECK_MODULES(LIBXML, [libxml-2.0],,[AC_MSG_ERROR(You must have libxml2)])

AC_SUBST(LIBXML_CFLAGS)
AC_SUBST(LIBXML_LIBS)

 dnl Process the different .in files, substituting the values computed here in the configuration
 dnl script for the corresponding @variable@ in these files.
AC_CONFIG_FILES(Makefile src/Makefile plugins/Makefile ggobi.pc)
AC_OUTPUT

echo "*******************************"
echo ""
echo "GGobi"
echo " Version:                $GGOBI_VERSION_STRING"
echo " Plugins:                ${PLUGIN_DIRS}"
echo " Installation directory: $GGOBI_INSTALL_HOME"
echo ""
echo "*******************************"

