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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "throttle.h"
#include "logger.h"
#include "types.h"
#include <time.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PLLong
plGetTimeMs (void)
{
  struct timespec ts;
  clock_gettime (CLOCK_MONOTONIC, &ts);
  return (PLLong)ts.tv_sec * (PLLong)1000
         + (PLLong)ts.tv_nsec / (PLLong)1000000;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
plThrottleInit (PLThrottle *t, PLDword rate, PLDword capacity)
{
  t->interval = (PLLong)1000 / rate;
  t->capacity = capacity * t->interval;
  t->tokens = t->capacity;
  t->last = plGetTimeMs ();
}

void
plThrottleAcquire (PLThrottle *t)
{
  for (;;)
    {
      PLLong now = plGetTimeMs ();
      PLDword elapsed = (PLDword)(now - t->last);
      PLDword tokens = t->tokens + elapsed;
      if (tokens > t->capacity)
        {
          tokens = t->capacity;
        }

      if (tokens >= t->interval)
        {
          t->tokens = tokens - t->interval;
          t->last = now;
          PL_DSLOW ("Throttle: acquired, tokens=%u", t->tokens);
          return;
        }

      t->tokens = tokens;
      t->last = now;

      PLDword wait = t->interval - t->tokens;
      PL_DSLOW ("Throttle: sleep, tokens=%u, wait=%u", t->tokens, wait);
      struct timespec ts;
      ts.tv_sec = (PLLong)wait / (PLLong)1000;
      ts.tv_nsec = ((PLLong)wait % (PLLong)1000) * (PLLong)1000000;
      nanosleep (&ts, NULL);
    }
}
