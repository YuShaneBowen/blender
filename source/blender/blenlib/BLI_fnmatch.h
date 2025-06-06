/* SPDX-FileCopyrightText: 1991 1992 1993 Free Software Foundation, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/** \file
 * \ingroup bli
 * \note The canonical source of this file is maintained with the GNU C Library.
 * Bugs can be reported to <bug-glibc@prep.ai.mit.edu>.
 */

#if defined WIN32 && !defined _LIBC

#  undef __P
#  define __P(protos) protos

/* We #undef these before defining them because some losing systems
 * (HP-UX A.08.07 for example) define these in <unistd.h>. */
#  undef FNM_PATHNAME
#  undef FNM_NOESCAPE
#  undef FNM_PERIOD

/* Bits set in the FLAGS argument to `fnmatch'. */
#  define FNM_PATHNAME (1 << 0) /* No wildcard can ever match `/'. */
#  define FNM_NOESCAPE (1 << 1) /* Backslashes don't quote special chars. */
#  define FNM_PERIOD (1 << 2)   /* Leading `.' is matched only explicitly. */

#  if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 2 || defined(_GNU_SOURCE)
#    define FNM_FILE_NAME FNM_PATHNAME /* Preferred GNU name. */
#    define FNM_LEADING_DIR (1 << 3)   /* Ignore `/...' after a match. */
#    define FNM_CASEFOLD (1 << 4)      /* Compare without regard to case. */
#  endif

/* Value returned by `fnmatch' if STRING does not match PATTERN. */
#  define FNM_NOMATCH 1

/* Match STRING against the filename pattern PATTERN,
 * returning zero if it matches, FNM_NOMATCH if not. */
extern int fnmatch __P((const char *__pattern, const char *__string, int __flags));

#else
#  ifndef _GNU_SOURCE
#    define _GNU_SOURCE
#  endif
#  include <fnmatch.h>  // IWYU pragma: export
#endif                  /* defined WIN32 && !defined _LIBC */
