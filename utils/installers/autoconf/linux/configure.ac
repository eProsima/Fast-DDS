m4_define([version_major],[0])
m4_define([version_minor],[5])
m4_define([version_micro],[1])

AC_INIT([eprosimartps], [VERSION], [support@eprosima.com], [eprosimartps], [http://eprosima.com/])
AC_PREREQ([2.59])
AM_INIT_AUTOMAKE([1.10 no-define foreign]):
AC_PROG_CXX
AC_CONFIG_MACRO_DIR([m4])
LT_INIT


# Check endianess
AC_C_BIGENDIAN(
  AC_DEFINE(__BIG_ENDIAN__, 1, [machine is bigendian]),
  AC_DEFINE(__LITTLE_ENDIAN__, 1, [machine is littleendian]),
  AC_MSG_ERROR(unknown endianess),
  AC_MSG_ERROR(universial endianess not supported)
)

AX_BOOST_BASE([1.53],
	AC_MSG_NOTICE([Found Boost]),
	AC_MSG_ERROR([Could not find Boost]))
AX_BOOST_THREAD
AX_BOOST_SYSTEM

AC_CONFIG_FILES([Makefile include/Makefile])

AC_OUTPUT
