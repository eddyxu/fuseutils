#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(fuseutils, 0.1, [eddyxu@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([wrapperfs.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([fuse], [fuse >= 2.7.0], [], [AC_MSG_ERROR(fuse was not found)])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stddef.h stdlib.h string.h sys/time.h unistd.h utime.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([memset mkdir rmdir])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
