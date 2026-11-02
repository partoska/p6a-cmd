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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "throttle.h"
#include "plat.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Milliseconds between two clock readings. The clock wraps around, and the
 * unsigned difference with it, which Clang's integer sanitizer would
 * otherwise report. */
#ifdef __clang__
__attribute__ ((no_sanitize ("integer")))
#endif
static PKDword
pkThrottleElapsed (PKDword now, PKDword last)
{
  return now - last;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKVoid
pkThrottleInit (PKThrottle *t, PKDword rate, PKDword capacity)
{
  t->interval = 1000u / rate;
  t->capacity = capacity * t->interval;
  t->tokens = t->capacity;
  t->last = pkClockMs ();
}

PKDword
pkThrottleTry (PKThrottle *t)
{
  PKDword now = pkClockMs ();
  PKDword elapsed = pkThrottleElapsed (now, t->last);
  PKDword tokens = t->tokens < t->capacity && elapsed < t->capacity - t->tokens
                       ? t->tokens + elapsed
                       : t->capacity;
  t->last = now;
  if (tokens >= t->interval)
    {
      t->tokens = tokens - t->interval;
      return 0;
    }

  t->tokens = tokens;

  return t->interval - tokens;
}
