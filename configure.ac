dnl This is the configuration script for ggobi

AC_INIT(ggobi, 2.0, ggobi-help@lists.ggobi.org)

AC_PREREQ(2.59)

# Get the version information.
. VersionInfo

TOP_DIR=`pwd`

if test -d .svn ; then
 GGOBI_VERSION_STRING="SVN version (`date`)"
fi

# Configure options (debug? local? any plugins?)

PLUGIN_DIRS=""

AC_ARG_ENABLE(debug, [--enable-debug          Compile with debugging symbols])
AC_ARG_ENABLE(local, [--enable-local          Configure for use directly from this directory tree.])

AC_ARG_WITH(xml,
  --with-xml          Enable reading and writing of XML datasets (yes),,with_xml="yes")

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
  --with-describedisplay          Enable plugin for viewing data in a datagrid., 
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


# Make sure we have a compiler
AC_PROG_CC

# Check for questionable functions (kind of paranoid)
AC_FUNC_MALLOC
AC_CHECK_FUNCS([pow sqrt strchr strrchr strstr memset])

# Check for some non-standard header files
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h strings.h unistd.h])

# Check for ability to create symbolic links (used in makefile for shortcuts)
AC_PROG_LN_S

# Misc checks
AC_C_CONST
AC_HEADER_STDBOOL


# Figure out where we are installing
if test "$prefix" = "NONE" ; then
 INSTALL_DIR=$ac_default_prefix
else
 INSTALL_DIR=$prefix
fi
AC_SUBST(INSTALL_DIR)

# Installing to current directory? That's local..
if test "${INSTALL_DIR}" = "`pwd`" ; then
 enable_local=yes
fi

echo "Enable local ${enable_local}"

if test -n "${enable_local}" &&  test "${enable_local}" != "no" ; then
 echo "Installing into build directory."
 INSTALL_DIR="`pwd`"
 GGOBI_INSTALL_HOME="${INSTALL_DIR}"
 LOCAL_INSTALL="yes"
 INSTALL_SRC_CFLAGS="-I${INSTALL_DIR}/src"
else
 INSTALL_DIR=$prefix
 INSTALL_SRC_CFLAGS="-I${INSTALL_DIR}/include/ggobi"
 GGOBI_INSTALL_HOME="${INSTALL_DIR}/ggobi/"
fi

INSTALL_SRC_LIBS="-L${INSTALL_DIR}/lib -lggobi"

dnl We need the Windows style path here for Inno
if test "`uname`" = "MINGW32_NT-5.1"; then
	GGOBI_HOME=`pwd -W`
else
	GGOBI_HOME=`pwd`
fi

echo "Install home: $GGOBI_INSTALL_HOME"

AC_SUBST(LOCAL_INSTALL)

# Look for plugins specified as a list

dnl The user might have given us a list of the plugins directly as the value of --with-plugins="a b c"

if test "$with_plugins" != "" && test "$with_plugins" != "yes" && test "$with_plugins" != "y" &&  test "$with_plugins" != "no" && test "$with_plugins" != "n" ; then
 PLUGIN_DIRS=$with_plugins
 ENABLE_ALL_PLUGINS=0
fi

if test -n "${ENABLE_ALL_PLUGINS}" && test ${ENABLE_ALL_PLUGINS} -gt 0 ; then
 PLUGIN_DIRS=""
 TMP=`cd plugins ; ls`
 for i in ${TMP}; do
   if test -d plugins/$i ; then
     PLUGIN_DIRS="${PLUGIN_DIRS} $i"
   fi
 done

echo "Plugin directories: $PLUGIN_DIRS"
fi

# Find uname binary

AC_PATH_PROG(UNAME, uname,,$PATH)
if test -z "${UNAME}" ; then
   AC_MSG_ERROR(Cannot find uname)
fi

# FIND GTK2

 echo "Looking for gtk2"
 AC_PATH_PROG(PKG_CONFIG, pkg-config,, $PATH)
 if test -z "${PKG_CONFIG}" ; then
   AC_MSG_ERROR(Cannot find pkg-config)
 fi

PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= 2.6.0],,[AC_MSG_ERROR(You must have GTK+ >= 2.6.0)])

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal, , $PATH)
AC_SUBST(GLIB_GENMARSHAL)

echo "Setting gtk flags: cflags=$GTK_CFLAGS, libs=$GTK_LIBS"

PKG_CHECK_MODULES(LIBXML, [libxml-2.0],,[AC_MSG_ERROR(You must have libxml2)])

AC_PATH_PROG(INSTALL, install)

if test -z "${INSTALL}" ; then
  echo "******* Cannot find the executable 'install'. Will not be able to install the different pieces."
fi

AC_SUBST(INSTALL)

LO_DIR=.

c_rules_frag=.CRulesFrag
SHLIB_EXT="so"

if test "`uname`" = "Darwin" ; then
 echo "Darwin operating system detected."
 PIC_CFLAG=-fno-common
 AC_PATH_PROG(ARCH, arch)
 if test -z "${ARCH}" ; then
  AC_MSG_WARN(Guessing ppc arch)
  ARCH="ppc"
 else
  ARCH=`arch`
 fi
 SHLIB_LDFLAGS="-dynamic -arch_only ${ARCH}"
 OTHER_CFLAGS="${OTHER_CFLAGS} -DDarwin"
# SHLIB_LDFLAGS="-dylib -flat_namespace -undefined suppress"
 SHLIB_EXT="dylib"

cat << \EOF > ${c_rules_frag}
%.o: %.c
	$(CC) $(CFLAGS) $(OTHER_CFLAGS) -c $< -o $@

%.lo: %.c
	$(CC) $(CFLAGS) $(OTHER_CFLAGS) $(PIC_CFLAG) -c $< -o $@

EOF

elif test "${GCC}" = "yes" ; then
if test "`uname`" = "MINGW32_NT-5.1"; then
  SHLIB_EXT="dll"
  EXE_EXT=".exe"
  OTHER_CFLAGS="${OTHER_CFLAGS} -mms-bitfields"
  LDFLAGS="-mwindows"
else 
  PIC_CFLAG=-fPIC
fi
# SHLIB_LDFLAGS=-shared
 LO_DIR=.


cat << \EOF > ${c_rules_frag}
%.lo: %.c
	$(CC) $(CFLAGS) $(OTHER_CFLAGS) -c $< -o $@
	
%.o: %.c
	$(CC) $(CFLAGS) $(OTHER_CFLAGS) -c $< -o $@

# Just copy the .o to .lo since the .o is position independent
# and relocatable by default with gcc.
# (Could be a symbolic link also)
#%.lo: %.o
#	cp $*.o $@

EOF
else
   dnl assume the Solaris cc compiler
 PIC_CFLAG=-KPIC
 OTHER_CFLAGS="-DG_INLINE_FUNC=extern"
# SHLIB_LDFLAGS=-G
 LO_DIR=.libs

cat << \EOF > ${c_rules_frag}
%.lo: %.c
	@test -d ${LO_DIR} || mkdir ${LO_DIR}
	$(CC) $(PIC_CFLAG) $(CFLAGS) $(OTHER_CFLAGS) -c $< -o ${LO_DIR}/$*.o
	mv ${LO_DIR}/$*.o $*.lo
EOF

fi

AC_SUBST(SHLIB_EXT)

dnl whether dlopen needs -ldl (such as on Darwin)
AC_CHECK_LIB(dl, dlopen)

dnl This will hopefully check whether the -shared flag works
dnl to create a shared library or whether we should use -G.
dnl Unchecked on the platforms that want to use -G.
if test -z "${SHLIB_LDFLAGS}" ; then
 echo "Checking flag for linking shared libraries"
 OLD_LDFLAGS="${LDFLAGS}"
 LDFLAGS="-shared"
 AC_TRY_LINK(,[int i=0; i++;], SHLIB_LDFLAGS="-shared", [USE_G="no"; echo "-G doesn't work"])
 if test "${USE_G}" = "no" ; then
   LDFLAGS="-G"
   AC_TRY_LINK(,[int i=0; i++;], SHLIB_LDFLAGS="-G",
        [echo "Cannot determine flag for creating shared libraries. Please specify this via the environment variable SHLIB_LDFLAGS"])
 fi
 echo "SHLIB linker flags: ${SHLIB_LDFLAGS}"
 LDFLAGS="${OLD_LDFLAGS}"
fi

AC_SUBST(OTHER_CFLAGS)
AC_SUBST_FILE(c_rules_frag)

MV=mv

dnl At this point, we have done most of the system-specific computations
dnl and are ready to start exporting the variables to be used in the different
dnl compilation and executable scripts.

GGOBI_ROOT=`pwd`
AC_SUBST(GGOBI_ROOT)

AC_SUBST(MV)

AC_SUBST(TOP_DIR)

AC_SUBST(UNAME)

AC_SUBST(LIBXML_CFLAGS)
AC_SUBST(LIBXML_LIBS)

AC_SUBST(GGOBI_HOME)
AC_SUBST(GGOBI_INSTALL_HOME)

AC_SUBST(INSTALL_DIR)

AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(PATCH_LEVEL)

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

if test -n "${enable_debug}" && ! test "${enable_debug}" = "no" ;  then
 SRC_DEBUG="-g"
fi
AC_SUBST(SRC_DEBUG)

AC_SUBST(INSTALL_SRC_LIBS)
AC_SUBST(INSTALL_SRC_CFLAGS)

AC_SUBST(PIC_CFLAG)
AC_SUBST(LDFLAGS)
AC_SUBST(SHLIB_LDFLAGS)
AC_SUBST(LO_DIR)

AC_SUBST(GGOBI_VERSION_STRING)
AC_SUBST(GGOBI_RELEASE_DATE)

AC_SUBST(EXE_EXT)

 dnl These go into config.h as #define entries.
AC_DEFINE_UNQUOTED(GGOBI_VERSION_STRING, "$GGOBI_VERSION_STRING")
AC_DEFINE_UNQUOTED(GGOBI_RELEASE_DATE, "$GGOBI_RELEASE_DATE")
AC_DEFINE_UNQUOTED(MAJOR_VERSION, $MAJOR_VERSION)
AC_DEFINE_UNQUOTED(MINOR_VERSION, $MINOR_VERSION)
AC_DEFINE_UNQUOTED(PATCH_LEVEL, $PATCH_LEVEL)

AC_CONFIG_HEADER(src/config.h)
AC_CONFIG_SRCDIR([src/transform_ui.c])

AC_SUBST(PLUGIN_DIRS)

if test -n "${PLUGIN_DIRS}" ; then
 echo "Enabling plugins"
 export GGOBI_ROOT

dnl 
dnl AC_CONFIG_SUBDIRS($PLUGIN_DIRS)
dnl might do the same thing. 

 INSTALLABLE_PLUGINS=""
 for p in ${PLUGIN_DIRS} ; do 
   echo

   if ! test -d "plugins/$p" ; then
    echo "No such plugin directory: plugins/$p"
    continue
   fi

   echo "Configuring plugin $p"
   
   if test -r plugins/$p/configure.in && ! test -x plugins/$p/configure; then
      cd plugins/$p
	  echo "Autoconfing plugin $p"
	  aclocal && autoconf
	  cd ../..
   fi
   
   if test -x plugins/$p/configure ; then
      cd plugins/$p 
      ./configure
      ok=$? 
      cd ../..
   else
      ok="0"
   fi
   if test "${ok}" = "0" ; then
     echo "Configured plugin $p"
     INSTALLABLE_PLUGINS="${INSTALLABLE_PLUGINS} $p"
   else
     echo "Failed to configure plugin $p ($ok)"
   fi

   echo

 done 

 PLUGIN_DIRS=$INSTALLABLE_PLUGINS
 echo "Installable plugins: $PLUGIN_DIRS"

fi

 dnl Process the different .in files, substituting the values computed here in the configuration
 dnl script for the corresponding @variable@ in these files.
AC_OUTPUT(Makeconf GNUmakefile src/GNUmakefile bin/setLD.csh bin/setLD.bsh bin/ggobi bin/ggobidb local.config plugin ggobi.pc win32/ggobi.iss PluginList)

dnl Ensure the resulting ggobi scripts executable.
chmod +x bin/ggobi
chmod +x bin/ggobidb
chmod +x bin/ggobi-config
chmod +x plugin



echo "*******************************"
echo ""
echo "GGobi"
echo " Version:                $GGOBI_VERSION_STRING"
echo " Plugins:                ${PLUGIN_DIRS}"
echo " Installation directory: $GGOBI_INSTALL_HOME"
echo ""
echo "*******************************"

