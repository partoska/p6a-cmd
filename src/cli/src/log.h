/*
 * Command Line Interface for Partoska.com media sharing service.
 * Copyright (C) 2026 Fabrika Charvat s.r.o. All rights reserved.
 * Developed by Partoska Laboratory team, <https://lab.partoska.com>
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * You can contact the author(s) via email at ask <at> partoska.com.
 */

#ifndef PKC_LOG_H
#define PKC_LOG_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "partoska.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Generic log prefix. */
#define _PKCL ""

/* Info prefix. */
#define _PKCLI ""

/* Warning prefix. */
#define _PKCLW "warning: "

/* Error prefix. */
#define _PKCLE "error: "

/* Fatal error prefix. */
#define _PKCLF "fatal: "

/* Debug message prefix. */
#define _PKCLD "debug: "

/* Verbose debug message prefix. */
#define _PKCLV "verbose: "

/* New line. */
#define _PKCLNL "\n"

/* Info log message mapping. */
#define _PKC_INFO_F(...) pkcInfo (__VA_ARGS__)

/* Warning log message mapping. */
#define _PKC_WARN_F(...) pkcWarn (__VA_ARGS__)

/* Error log message mapping. */
#define _PKC_ERROR_F(...) pkcError (__VA_ARGS__)

/* Fatal error log message mapping. */
#define _PKC_FATAL_F(...) pkcFatal (__VA_ARGS__)

#ifdef _DEBUG
/* Debug log message mapping. */
#define _PKC_DEBUG_F(...) pkcDebug (__VA_ARGS__)
#else
/* Debug log message mapping. */
#define _PKC_DEBUG_F(...)
#endif

#if DEBUG_SLOW
/* Verbose debug log message mapping. */
#define _PKC_DSLOW_F(...) pkcDebug (__VA_ARGS__)
#else
/* Verbose debug log message mapping. */
#define _PKC_DSLOW_F(...)
#endif

/**
 * Calculates the number of arguments.
 */
#define _PKC_NARG(...) _PKC_NARG_I (__VA_ARGS__, _PKC_RSEQ_N ())
#define _PKC_NARG_I(...) _PKC_ARG_N (__VA_ARGS__)
#define _PKC_ARG_N(_1, _2, _3, _4, _5, _6, N, ...) N
#define _PKC_RSEQ_N() 6, 5, 4, 3, 2, 1, 0

/**
 * Converts func to funcX where "X" denotes the argument count.
 */
#define _PKC_VFUNC_INNER(name, n) name##n
#define _PKC_VFUNC_OUTER(name, n) _PKC_VFUNC_INNER (name, n)
#define _PKC_VFUNC(func, ...)                                                 \
  _PKC_VFUNC_OUTER (func, _PKC_NARG (__VA_ARGS__)) (__VA_ARGS__)

#define _PKC_INFO1(fmt) _PKC_INFO_F (_PKCL _PKCLI fmt _PKCLNL)
#define _PKC_INFO2(fmt, a1) _PKC_INFO_F (_PKCL _PKCLI fmt _PKCLNL, a1)
#define _PKC_INFO3(fmt, a1, a2) _PKC_INFO_F (_PKCL _PKCLI fmt _PKCLNL, a1, a2)
#define _PKC_INFO4(fmt, a1, a2, a3)                                           \
  _PKC_INFO_F (_PKCL _PKCLI fmt _PKCLNL, a1, a2, a3)
#define _PKC_INFO5(fmt, a1, a2, a3, a4)                                       \
  _PKC_INFO_F (_PKCL _PKCLI fmt _PKCLNL, a1, a2, a3, a4)
#define _PKC_INFO6(fmt, a1, a2, a3, a4, a5)                                   \
  _PKC_INFO_F (_PKCL _PKCLI fmt _PKCLNL, a1, a2, a3, a4, a5)

#define _PKC_WARN1(fmt) _PKC_WARN_F (_PKCL _PKCLW fmt _PKCLNL)
#define _PKC_WARN2(fmt, a1) _PKC_WARN_F (_PKCL _PKCLW fmt _PKCLNL, a1)
#define _PKC_WARN3(fmt, a1, a2) _PKC_WARN_F (_PKCL _PKCLW fmt _PKCLNL, a1, a2)
#define _PKC_WARN4(fmt, a1, a2, a3)                                           \
  _PKC_WARN_F (_PKCL _PKCLW fmt _PKCLNL, a1, a2, a3)
#define _PKC_WARN5(fmt, a1, a2, a3, a4)                                       \
  _PKC_WARN_F (_PKCL _PKCLW fmt _PKCLNL, a1, a2, a3, a4)
#define _PKC_WARN6(fmt, a1, a2, a3, a4, a5)                                   \
  _PKC_WARN_F (_PKCL _PKCLW fmt _PKCLNL, a1, a2, a3, a4, a5)

#define _PKC_ERROR1(fmt) _PKC_ERROR_F (_PKCL _PKCLE fmt _PKCLNL)
#define _PKC_ERROR2(fmt, a1) _PKC_ERROR_F (_PKCL _PKCLE fmt _PKCLNL, a1)
#define _PKC_ERROR3(fmt, a1, a2)                                              \
  _PKC_ERROR_F (_PKCL _PKCLE fmt _PKCLNL, a1, a2)
#define _PKC_ERROR4(fmt, a1, a2, a3)                                          \
  _PKC_ERROR_F (_PKCL _PKCLE fmt _PKCLNL, a1, a2, a3)
#define _PKC_ERROR5(fmt, a1, a2, a3, a4)                                      \
  _PKC_ERROR_F (_PKCL _PKCLE fmt _PKCLNL, a1, a2, a3, a4)
#define _PKC_ERROR6(fmt, a1, a2, a3, a4, a5)                                  \
  _PKC_ERROR_F (_PKCL _PKCLE fmt _PKCLNL, a1, a2, a3, a4, a5)

#define _PKC_FATAL1(fmt) _PKC_FATAL_F (_PKCL _PKCLF fmt _PKCLNL)
#define _PKC_FATAL2(fmt, a1) _PKC_FATAL_F (_PKCL _PKCLF fmt _PKCLNL, a1)
#define _PKC_FATAL3(fmt, a1, a2)                                              \
  _PKC_FATAL_F (_PKCL _PKCLF fmt _PKCLNL, a1, a2)
#define _PKC_FATAL4(fmt, a1, a2, a3)                                          \
  _PKC_FATAL_F (_PKCL _PKCLF fmt _PKCLNL, a1, a2, a3)
#define _PKC_FATAL5(fmt, a1, a2, a3, a4)                                      \
  _PKC_FATAL_F (_PKCL _PKCLF fmt _PKCLNL, a1, a2, a3, a4)
#define _PKC_FATAL6(fmt, a1, a2, a3, a4, a5)                                  \
  _PKC_FATAL_F (_PKCL _PKCLF fmt _PKCLNL, a1, a2, a3, a4, a5)

#define _PKC_DEBUG1(fmt) _PKC_DEBUG_F (_PKCL _PKCLD fmt _PKCLNL)
#define _PKC_DEBUG2(fmt, a1) _PKC_DEBUG_F (_PKCL _PKCLD fmt _PKCLNL, a1)
#define _PKC_DEBUG3(fmt, a1, a2)                                              \
  _PKC_DEBUG_F (_PKCL _PKCLD fmt _PKCLNL, a1, a2)
#define _PKC_DEBUG4(fmt, a1, a2, a3)                                          \
  _PKC_DEBUG_F (_PKCL _PKCLD fmt _PKCLNL, a1, a2, a3)
#define _PKC_DEBUG5(fmt, a1, a2, a3, a4)                                      \
  _PKC_DEBUG_F (_PKCL _PKCLD fmt _PKCLNL, a1, a2, a3, a4)
#define _PKC_DEBUG6(fmt, a1, a2, a3, a4, a5)                                  \
  _PKC_DEBUG_F (_PKCL _PKCLD fmt _PKCLNL, a1, a2, a3, a4, a5)

#define _PKC_DSLOW1(fmt) _PKC_DSLOW_F (_PKCL _PKCLV fmt _PKCLNL)
#define _PKC_DSLOW2(fmt, a1) _PKC_DSLOW_F (_PKCL _PKCLV fmt _PKCLNL, a1)
#define _PKC_DSLOW3(fmt, a1, a2)                                              \
  _PKC_DSLOW_F (_PKCL _PKCLV fmt _PKCLNL, a1, a2)
#define _PKC_DSLOW4(fmt, a1, a2, a3)                                          \
  _PKC_DSLOW_F (_PKCL _PKCLV fmt _PKCLNL, a1, a2, a3)
#define _PKC_DSLOW5(fmt, a1, a2, a3, a4)                                      \
  _PKC_DSLOW_F (_PKCL _PKCLV fmt _PKCLNL, a1, a2, a3, a4)
#define _PKC_DSLOW6(fmt, a1, a2, a3, a4, a5)                                  \
  _PKC_DSLOW_F (_PKCL _PKCLV fmt _PKCLNL, a1, a2, a3, a4, a5)

#define PKC_INFO(...) _PKC_VFUNC (_PKC_INFO, __VA_ARGS__)
#define PKC_WARN(...) _PKC_VFUNC (_PKC_WARN, __VA_ARGS__)
#define PKC_ERROR(...) _PKC_VFUNC (_PKC_ERROR, __VA_ARGS__)
#define PKC_FATAL(...) _PKC_VFUNC (_PKC_FATAL, __VA_ARGS__)
#define PKC_DEBUG(...) _PKC_VFUNC (_PKC_DEBUG, __VA_ARGS__)
#define PKC_DSLOW(...) _PKC_VFUNC (_PKC_DSLOW, __VA_ARGS__)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Writes an informational message to stdout.
 *
 * @param fmt printf-style format string.
 * @return Number of characters written, or a negative value on error.
 */
PKInt pkcInfo (const PKChar *fmt, ...);

/**
 * Writes a warning message to stderr.
 *
 * @param fmt printf-style format string.
 * @return Number of characters written, or a negative value on error.
 */
PKInt pkcWarn (const PKChar *fmt, ...);

/**
 * Writes an error message to stderr.
 *
 * @param fmt printf-style format string.
 * @return Number of characters written, or a negative value on error.
 */
PKInt pkcError (const PKChar *fmt, ...);

/**
 * Writes a fatal error message to stderr.
 *
 * @param fmt printf-style format string.
 * @return Number of characters written, or a negative value on error.
 */
PKInt pkcFatal (const PKChar *fmt, ...);

/**
 * Writes a debug message to stderr (compiled out in release builds).
 *
 * @param fmt printf-style format string.
 * @return Number of characters written, or a negative value on error.
 */
PKInt pkcDebug (const PKChar *fmt, ...);

#endif
