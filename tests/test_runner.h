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

#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "types.h"
#include <stdio.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Helpers
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PLInt plAssertions = 0;
static PLInt plFailures = 0;

static void
plAssertImpl (PLInt cond, const PLChar *expr, const PLChar *file, PLInt line)
{
  plAssertions++;
  if (!cond)
    {
      fprintf (stderr, "\n    assertion failed: %s (%s:%d)", expr, file, line);
      plFailures++;
    }
}

#define PL_ASSERT(cond) plAssertImpl (!!(cond), #cond, __FILE__, __LINE__)
#define PL_ASSERT_EQ(a, b) PL_ASSERT ((a) == (b))
#define PL_ASSERT_STR_EQ(a, b) PL_ASSERT (strcmp ((a), (b)) == 0)
#define PL_ASSERT_MEM_EQ(a, b, n) PL_ASSERT (memcmp ((a), (b), (n)) == 0)

#define PL_RUN(fn)                                                            \
  do                                                                          \
    {                                                                         \
      PLInt nBefore = plFailures;                                             \
      printf ("  %-48s", #fn);                                                \
      fflush (stdout);                                                        \
      fn ();                                                                  \
      printf ("%s\n", plFailures == nBefore ? "ok" : "FAIL");                 \
    }                                                                         \
  while (0)

#define PL_SUITE(name) printf ("\n%s\n", name)

#define PL_SUMMARY()                                                          \
  do                                                                          \
    {                                                                         \
      printf ("\n%d assertions, %d failed\n", plAssertions, plFailures);      \
      return plFailures > 0 ? 1 : 0;                                          \
    }                                                                         \
  while (0)

#endif
