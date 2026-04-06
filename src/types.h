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

#ifndef TYPES_H
#define TYPES_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <sys/types.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef char PLChar;

typedef signed char PLSByte;
typedef signed short PLShort;
typedef signed int PLInt;
typedef signed long long PLLong;

typedef unsigned char PLByte;
typedef unsigned short PLWord;
typedef unsigned int PLDword;
typedef unsigned long long PLQword;

typedef unsigned char PLBool;
typedef size_t PLSize;
typedef ssize_t PLSSize;
typedef time_t PLTime;

typedef enum PLError
{
  PL_EOK = 0,

  PL_EARG = -1,
  PL_EMEM = -2,
  PL_EFS = -3,
  PL_ENET = -4,

  PL_EGEN = -32
} PLError;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define PL_XSTR(s) PL_STR (s)
#define PL_STR(s) #s

#define PL_VERSION_MAJOR 1
#define PL_VERSION_MINOR 7
#define PL_VERSION_PATCH 0
#define PL_VERSION_STRING                                                     \
  PL_XSTR (PL_VERSION_MAJOR)                                                  \
  "." PL_XSTR (PL_VERSION_MINOR) "." PL_XSTR (PL_VERSION_PATCH)

#define PL_TRUE (1)
#define PL_FALSE (0)
#define PL_UNUSED(x) (void)(x)
#define PL_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define PL_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define PL_SIZEOF(t) (sizeof (t))
#define PL_CHARSMAX(a) (PL_SIZEOF (a) - 1)

/**
 * Computes the offset of a member within the struct.
 *
 * @param s Struct type.
 * @param m Member name.
 */
#define PL_OFFSETOF(s, m) ((PLSize)((PLChar *)&((s *)0)->m - (PLChar *)0))

/**
 * Gets the count of elements for statically sized arrays.
 *
 * @param a Array to count.
 */
#define PL_COUNTOF(a)                                                         \
  ((PL_SIZEOF (a) / PL_SIZEOF (0 [a]))                                        \
   / ((PLSize)(!(PL_SIZEOF (a) % PL_SIZEOF (0 [a])))))

/**
 * Validates at compile time that the predicate is true without generating
 * code. It can be used at any point in a source file where typedef is legal.
 *
 * On success, compilation proceeds normally.
 *
 * On failure, attempts to typedef an array type of negative size.
 *
 * @param expr The expression to test. It must evaluate to something that can
 * be coerced to a normal C boolean.
 */
#define PL_COMPILE_ASSERT(expr) _PL_COMPILE_ASSERT (expr, __LINE__)
#define _PL_JOIN(a, b) a##b
#define _PL_COMPILE_ASSERT(expr, line)                                        \
  typedef char _PL_JOIN (assertion_failed_, line)[2 * !!(expr) - 1]

/**
 * Expectations from the defined types.
 */
PL_COMPILE_ASSERT (PL_SIZEOF (PLChar) == 1);
PL_COMPILE_ASSERT (PL_SIZEOF (PLBool) == 1);

PL_COMPILE_ASSERT (PL_SIZEOF (PLSByte) == 1);
PL_COMPILE_ASSERT (PL_SIZEOF (PLShort) == 2);
PL_COMPILE_ASSERT (PL_SIZEOF (PLInt) == 4);
PL_COMPILE_ASSERT (PL_SIZEOF (PLLong) == 8);

PL_COMPILE_ASSERT (PL_SIZEOF (PLByte) == 1);
PL_COMPILE_ASSERT (PL_SIZEOF (PLWord) == 2);
PL_COMPILE_ASSERT (PL_SIZEOF (PLDword) == 4);
PL_COMPILE_ASSERT (PL_SIZEOF (PLQword) == 8);

PL_COMPILE_ASSERT (PL_SIZEOF (PLSize) == PL_SIZEOF (void *));
PL_COMPILE_ASSERT (PL_SIZEOF (PLTime) >= 4);

#ifdef __clang__
#define PL_NO_SANITIZE_INTEGER __attribute__ ((no_sanitize ("integer")))
#else
#define PL_NO_SANITIZE_INTEGER
#endif

#endif
