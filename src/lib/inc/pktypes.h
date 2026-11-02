/*
 * Partoska Library - Client library for Partoska.com media sharing service.
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

#ifndef PKTYPES_H
#define PKTYPES_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <time.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef __cplusplus
typedef bool PKBool;
#else
typedef _Bool PKBool;
#endif
typedef char PKChar;
typedef unsigned char PKByte;
typedef int PKInt;
typedef unsigned int PKDword;
typedef long PKLong;
typedef unsigned long PKULong;

typedef void PKVoid;
typedef size_t PKSize;
typedef va_list PKVaList;
typedef time_t PKTime;
typedef sig_atomic_t PKSigAtomic;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define PK_TRUE ((PKBool)1)
#define PK_FALSE ((PKBool)0)

#define PK_UNUSED(x) ((void)(x))
#define PK_XSTR(s) PK_STR (s)
#define PK_STR(s) #s

/* Variable arguments of a function, kept in a PKVaList. */
#define PK_VA_START(args, last) va_start (args, last)
#define PK_VA_ARG(args, type) va_arg (args, type)
#define PK_VA_END(args) va_end (args)

/* Capacity of a character array without its terminating NUL. */
#define PK_CHARSMAX(a) (sizeof (a) - 1)

/* Number of elements of an array; fails to compile for pointers. */
#define PK_COUNTOF(a)                                                         \
  ((sizeof (a) / sizeof (0 [a])) / ((PKSize)(!(sizeof (a) % sizeof (0 [a])))))

/* Fails to compile when expr is false; usable where a typedef is. */
#define PK_COMPILE_ASSERT(expr) _PK_COMPILE_ASSERT (expr, __LINE__)
#define _PK_JOIN(a, b) a##b
#define _PK_COMPILE_ASSERT(expr, line)                                        \
  typedef char _PK_JOIN (assertion_failed_, line)[2 * !!(expr) - 1]

/* Format checking of printf-style functions. MinGW names the archetype of
 * the C library it links (__MINGW_PRINTF_FORMAT, from the system headers
 * included above). */
#if defined(__MINGW_PRINTF_FORMAT)
#define PK_PRINTF(fmt, args)                                                  \
  __attribute__ ((format (__MINGW_PRINTF_FORMAT, fmt, args)))
#elif defined(__GNUC__) || defined(__clang__)
#define PK_PRINTF(fmt, args) __attribute__ ((format (printf, fmt, args)))
#else
#define PK_PRINTF(fmt, args)
#endif

#endif
