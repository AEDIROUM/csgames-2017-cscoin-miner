dnl $Id$
dnl
dnl Copyright (C) 2009 Alexios Chouchoulas
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2, or (at your option)
dnl any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software Foundation,
dnl Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  

AC_INIT(src/astar.c)

AC_CANONICAL_SYSTEM
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl Magic to obtain the version from debian/changelog, where it should
dnl reside.
AM_INIT_AUTOMAKE($PACKAGE,$$PACKAGE_VERSION)

dnl Essential info (some autodetected)
PACKAGE=`grep urgency= debian/changelog|head -1 | sed 's/ .*//'`
PACKAGE_VERSION=`grep urgency= debian/changelog |head -1|sed -e 's/^.*(\(.*\)).*$/\1/'`
LIBVERSION=0:0:0
ISODATE=`date +%Y-%m-%d`

dnl Export essential info.
AC_SUBST(PACKAGE)
AC_SUBST(PACKAGE_VERSION)
AC_SUBST(LIBVERSION)
AC_SUBST(ISODATE)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl C compiler
AM_PROG_CC_C_O
AC_C_CONST
AC_C_INLINE

dnl Types
AC_TYPE_UINT8_T
AC_TYPE_INT8_T
AC_TYPE_UINT16_T
AC_TYPE_INT16_T
AC_TYPE_UINT32_T
AC_TYPE_INT32_T
AC_TYPE_UINT64_T
AC_TYPE_INT64_T

dnl Check for functions.
AC_CHECK_FUNCS(program_invocation_name program_invocation_short_name vsnprintf snprintf)

dnl If we have groff, we can build HTML documentation
AC_CHECK_PROG(groff, groff, groff, no)

dnl Additional arguments (and substitutions)
dnl Allow the pkg-config directory to be set
dnl (stanza taken from libpng)
AC_ARG_WITH(pkgconfigdir,
	AC_HELP_STRING([--with-pkgconfigdir],
	[Use the specified pkgconfig dir (default is libdir/pkgconfig)]),
	[pkgconfigdir=${withval}],
	[pkgconfigdir='${libdir}/pkgconfig'])

AC_SUBST([pkgconfigdir])
AC_MSG_NOTICE([pkgconfig directory is ${pkgconfigdir}])

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER(src/astar_config.h)

AC_MSG_CHECKING(package name)
AC_MSG_RESULT($PACKAGE)

dnl Write files.
AC_OUTPUT(Makefile src/Makefile src/libastar.pc)

dnl End of file.
