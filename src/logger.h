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

#ifndef LOG_H
#define LOG_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "types.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Generic log prefix. */
#define _PLL ""

/* Info prefix. */
#define _PLLI ""

/* Warning prefix. */
#define _PLLW "warning: "

/* Error prefix. */
#define _PLLE "error: "

/* Fatal error prefix. */
#define _PLLF "fatal: "

/* Debug message prefix. */
#define _PLLD "debug: "

/* Verbose debug message prefix. */
#define _PLLV "verbose: "

/* New line. */
#define _PLLNL "\n"

/* Info log message mapping. */
#define _PL_INFO_F(...) plInfo (__VA_ARGS__)

/* Warning log message mapping. */
#define _PL_WARN_F(...) plWarn (__VA_ARGS__)

/* Error log message mapping. */
#define _PL_ERROR_F(...) plError (__VA_ARGS__)

/* Fatal error log message mapping. */
#define _PL_FATAL_F(...) plFatal (__VA_ARGS__)

#ifdef _DEBUG
/* Debug log message mapping. */
#define _PL_DEBUG_F(...) plDebug (__VA_ARGS__)
#else
/* Debug log message mapping. */
#define _PL_DEBUG_F(...)
#endif

#if DEBUG_SLOW
/* Verbose debug log message mapping. */
#define _PL_DSLOW_F(...) plDebug (__VA_ARGS__)
#else
/* Verbose debug log message mapping. */
#define _PL_DSLOW_F(...)
#endif

/**
 * Calculates the number of arguments.
 */
#define _PL_NARG(...) _PL_NARG_I (__VA_ARGS__, _PL_RSEQ_N ())
#define _PL_NARG_I(...) _PL_ARG_N (__VA_ARGS__)
#define _PL_ARG_N(_1, _2, _3, _4, _5, _6, N, ...) N
#define _PL_RSEQ_N() 6, 5, 4, 3, 2, 1, 0

/**
 * Converts func to funcX where "X" denotes the argument count.
 */
#define _PL_VFUNC_INNER(name, n) name##n
#define _PL_VFUNC_OUTER(name, n) _PL_VFUNC_INNER (name, n)
#define _PL_VFUNC(func, ...)                                                  \
  _PL_VFUNC_OUTER (func, _PL_NARG (__VA_ARGS__)) (__VA_ARGS__)

#define _PL_INFO1(fmt) _PL_INFO_F (_PLL _PLLI fmt _PLLNL)
#define _PL_INFO2(fmt, a1) _PL_INFO_F (_PLL _PLLI fmt _PLLNL, a1)
#define _PL_INFO3(fmt, a1, a2) _PL_INFO_F (_PLL _PLLI fmt _PLLNL, a1, a2)
#define _PL_INFO4(fmt, a1, a2, a3)                                            \
  _PL_INFO_F (_PLL _PLLI fmt _PLLNL, a1, a2, a3)
#define _PL_INFO5(fmt, a1, a2, a3, a4)                                        \
  _PL_INFO_F (_PLL _PLLI fmt _PLLNL, a1, a2, a3, a4)
#define _PL_INFO6(fmt, a1, a2, a3, a4, a5)                                    \
  _PL_INFO_F (_PLL _PLLI fmt _PLLNL, a1, a2, a3, a4, a5)

#define _PL_WARN1(fmt) _PL_WARN_F (_PLL _PLLW fmt _PLLNL)
#define _PL_WARN2(fmt, a1) _PL_WARN_F (_PLL _PLLW fmt _PLLNL, a1)
#define _PL_WARN3(fmt, a1, a2) _PL_WARN_F (_PLL _PLLW fmt _PLLNL, a1, a2)
#define _PL_WARN4(fmt, a1, a2, a3)                                            \
  _PL_WARN_F (_PLL _PLLW fmt _PLLNL, a1, a2, a3)
#define _PL_WARN5(fmt, a1, a2, a3, a4)                                        \
  _PL_WARN_F (_PLL _PLLW fmt _PLLNL, a1, a2, a3, a4)
#define _PL_WARN6(fmt, a1, a2, a3, a4, a5)                                    \
  _PL_WARN_F (_PLL _PLLW fmt _PLLNL, a1, a2, a3, a4, a5)

#define _PL_ERROR1(fmt) _PL_ERROR_F (_PLL _PLLE fmt _PLLNL)
#define _PL_ERROR2(fmt, a1) _PL_ERROR_F (_PLL _PLLE fmt _PLLNL, a1)
#define _PL_ERROR3(fmt, a1, a2) _PL_ERROR_F (_PLL _PLLE fmt _PLLNL, a1, a2)
#define _PL_ERROR4(fmt, a1, a2, a3)                                           \
  _PL_ERROR_F (_PLL _PLLE fmt _PLLNL, a1, a2, a3)
#define _PL_ERROR5(fmt, a1, a2, a3, a4)                                       \
  _PL_ERROR_F (_PLL _PLLE fmt _PLLNL, a1, a2, a3, a4)
#define _PL_ERROR6(fmt, a1, a2, a3, a4, a5)                                   \
  _PL_ERROR_F (_PLL _PLLE fmt _PLLNL, a1, a2, a3, a4, a5)

#define _PL_FATAL1(fmt) _PL_FATAL_F (_PLL _PLLF fmt _PLLNL)
#define _PL_FATAL2(fmt, a1) _PL_FATAL_F (_PLL _PLLF fmt _PLLNL, a1)
#define _PL_FATAL3(fmt, a1, a2) _PL_FATAL_F (_PLL _PLLF fmt _PLLNL, a1, a2)
#define _PL_FATAL4(fmt, a1, a2, a3)                                           \
  _PL_FATAL_F (_PLL _PLLF fmt _PLLNL, a1, a2, a3)
#define _PL_FATAL5(fmt, a1, a2, a3, a4)                                       \
  _PL_FATAL_F (_PLL _PLLF fmt _PLLNL, a1, a2, a3, a4)
#define _PL_FATAL6(fmt, a1, a2, a3, a4, a5)                                   \
  _PL_FATAL_F (_PLL _PLLF fmt _PLLNL, a1, a2, a3, a4, a5)

#define _PL_DEBUG1(fmt) _PL_DEBUG_F (_PLL _PLLD fmt _PLLNL)
#define _PL_DEBUG2(fmt, a1) _PL_DEBUG_F (_PLL _PLLD fmt _PLLNL, a1)
#define _PL_DEBUG3(fmt, a1, a2) _PL_DEBUG_F (_PLL _PLLD fmt _PLLNL, a1, a2)
#define _PL_DEBUG4(fmt, a1, a2, a3)                                           \
  _PL_DEBUG_F (_PLL _PLLD fmt _PLLNL, a1, a2, a3)
#define _PL_DEBUG5(fmt, a1, a2, a3, a4)                                       \
  _PL_DEBUG_F (_PLL _PLLD fmt _PLLNL, a1, a2, a3, a4)
#define _PL_DEBUG6(fmt, a1, a2, a3, a4, a5)                                   \
  _PL_DEBUG_F (_PLL _PLLD fmt _PLLNL, a1, a2, a3, a4, a5)

#define _PL_DSLOW1(fmt) _PL_DSLOW_F (_PLL _PLLV fmt _PLLNL)
#define _PL_DSLOW2(fmt, a1) _PL_DSLOW_F (_PLL _PLLV fmt _PLLNL, a1)
#define _PL_DSLOW3(fmt, a1, a2) _PL_DSLOW_F (_PLL _PLLV fmt _PLLNL, a1, a2)
#define _PL_DSLOW4(fmt, a1, a2, a3)                                           \
  _PL_DSLOW_F (_PLL _PLLV fmt _PLLNL, a1, a2, a3)
#define _PL_DSLOW5(fmt, a1, a2, a3, a4)                                       \
  _PL_DSLOW_F (_PLL _PLLV fmt _PLLNL, a1, a2, a3, a4)
#define _PL_DSLOW6(fmt, a1, a2, a3, a4, a5)                                   \
  _PL_DSLOW_F (_PLL _PLLV fmt _PLLNL, a1, a2, a3, a4, a5)

#define PL_INFO(...) _PL_VFUNC (_PL_INFO, __VA_ARGS__)
#define PL_WARN(...) _PL_VFUNC (_PL_WARN, __VA_ARGS__)
#define PL_ERROR(...) _PL_VFUNC (_PL_ERROR, __VA_ARGS__)
#define PL_FATAL(...) _PL_VFUNC (_PL_FATAL, __VA_ARGS__)
#define PL_DEBUG(...) _PL_VFUNC (_PL_DEBUG, __VA_ARGS__)
#define PL_DSLOW(...) _PL_VFUNC (_PL_DSLOW, __VA_ARGS__)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Writes an informational message to stdout.
 *
 * @param fmt printf-style format string.
 * @return Number of characters written, or a negative value on error.
 */
PLInt plInfo (const PLChar *fmt, ...);

/**
 * Writes a warning message to stderr.
 *
 * @param fmt printf-style format string.
 * @return Number of characters written, or a negative value on error.
 */
PLInt plWarn (const PLChar *fmt, ...);

/**
 * Writes an error message to stderr.
 *
 * @param fmt printf-style format string.
 * @return Number of characters written, or a negative value on error.
 */
PLInt plError (const PLChar *fmt, ...);

/**
 * Writes a fatal error message to stderr.
 *
 * @param fmt printf-style format string.
 * @return Number of characters written, or a negative value on error.
 */
PLInt plFatal (const PLChar *fmt, ...);

/**
 * Writes a debug message to stderr (compiled out in release builds).
 *
 * @param fmt printf-style format string.
 * @return Number of characters written, or a negative value on error.
 */
PLInt plDebug (const PLChar *fmt, ...);

#endif
