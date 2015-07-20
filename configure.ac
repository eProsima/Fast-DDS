#
# Autoconf script FastRTPS
#

#
# Define variables for the FastRTPS version number.
#
m4_define([version_major],[1])
m4_define([version_minor],[0])
m4_define([version_micro],[1])

AC_INIT([fastrtps], [version_major.version_minor.version_micro], [support@eprosima.com], [eProsima FastRTPS], [http://eprosima.com/])
CONFIG_ARGS="$*"
AC_SUBST(CONFIG_ARGS)

#Minimum autoconf version we require.
AC_PREREQ([2.59])

AM_INIT_AUTOMAKE([1.10 no-define foreign subdir-objects])

# Makes version available in config.h
AC_DEFINE(VERSION_MAJOR, version_major, [fastrtps version major])
AC_DEFINE(VERSION_MINOR, version_minor, [fastrtps version minor])
AC_DEFINE(VERSION_MICRO, version_micro, [fastrtps version micro])

# Check stdcxx
AX_CXX_COMPILE_STDCXX_11(,optional)

# Check endianess
AC_C_BIGENDIAN(
  AC_DEFINE(__BIG_ENDIAN__, 1, [machine is bigendian]),
  AC_DEFINE(__BIG_ENDIAN__, 0, [machine is littleendian]),
  AC_MSG_ERROR(unknown endianess),
  AC_MSG_ERROR(universial endianess not supported)
)

AX_BOOST_BASE([1.53],
	AC_MSG_NOTICE([Found Boost]),
	AC_MSG_ERROR([Could not find Boost]))
AX_BOOST_SYSTEM
AX_BOOST_THREAD
AX_BOOST_DATE_TIME

# Check for libraries used in the main build process
AC_PROG_CXX
AC_PROG_CPP

# Init libtool
AC_CONFIG_MACRO_DIR([m4])
LT_INIT

AC_CONFIG_HEADERS(include/fastrtps/config.h:include/fastrtps/config.h.in)
AC_CONFIG_FILES([Makefile
                 include/Makefile
                 src/cpp/Makefile])

AC_OUTPUT
