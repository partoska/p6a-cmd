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

#include "test_runner.h"
#include "throttle.h"
#include <stdio.h>
#include <time.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Helpers
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PLLong
nowMs (void)
{
  struct timespec ts;
  clock_gettime (CLOCK_MONOTONIC, &ts);
  return (PLLong)ts.tv_sec * 1000 + (PLLong)ts.tv_nsec / 1000000;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Init
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testInitFields (void)
{
  PLThrottle t;

  // rate=10, capacity=5 → interval=100ms, bucket=500ms, tokens full
  plThrottleInit (&t, 10, 5);
  PL_ASSERT_EQ (t.interval, (PLDword)100);
  PL_ASSERT_EQ (t.capacity, (PLDword)500);
  PL_ASSERT_EQ (t.tokens, t.capacity);

  // rate=100, capacity=3 → interval=10ms, bucket=30ms
  plThrottleInit (&t, 100, 3);
  PL_ASSERT_EQ (t.interval, (PLDword)10);
  PL_ASSERT_EQ (t.capacity, (PLDword)30);
  PL_ASSERT_EQ (t.tokens, t.capacity);

  // rate=1000, capacity=1 → interval=1ms, bucket=1ms
  plThrottleInit (&t, 1000, 1);
  PL_ASSERT_EQ (t.interval, (PLDword)1);
  PL_ASSERT_EQ (t.capacity, (PLDword)1);
  PL_ASSERT_EQ (t.tokens, t.capacity);
}

static void
testInitLastIsRecent (void)
{
  // t.last should be within 1 second of now.
  PLThrottle t;
  PLLong before = nowMs ();
  plThrottleInit (&t, 10, 5);
  PLLong after = nowMs ();
  PL_ASSERT (t.last >= before);
  PL_ASSERT (t.last <= after + 1);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Acquire from a full bucket (no sleeping)
 *
 * A freshly initialized throttle has tokens == capacity.  Each
 * acquire consumes one interval worth of tokens, so we can drain
 * the bucket (capacity) times without ever sleeping.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testAcquireFullBucketNoSleep (void)
{
  // rate=1000 (interval=1ms), capacity=20 → 20ms of credit up front.
  // 20 back-to-back acquires should complete in well under 20ms.
  PLThrottle t;
  plThrottleInit (&t, 1000, 20);

  PLLong start = nowMs ();
  for (PLInt i = 0; i < 20; ++i)
    {
      plThrottleAcquire (&t);
    }
  PLLong elapsed = nowMs () - start;

  // Should complete in under 10ms — 20 trivial C calls take ~microseconds.
  PL_ASSERT (elapsed < 10);
}

static void
testAcquireDrainsTokens (void)
{
  // After draining the bucket, tokens should be near zero.
  PLThrottle t;
  // NOTE: interval=1ms, capacity=5ms, tokens=5
  plThrottleInit (&t, 1000, 5);

  for (PLInt i = 0; i < 5; ++i)
    {
      plThrottleAcquire (&t);
    }

  // Tokens have been consumed; remaining balance is < one interval.
  PL_ASSERT (t.tokens < t.interval);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Rate limiting (real time, short sleep)
 *
 * With capacity=1 there is no burst credit: every acquire after the
 * first must wait one full interval.  We use rate=100 (10ms interval)
 * and do 4 acquires → expect at least 3 × 10ms = 30ms wall time.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testRateLimitsThroughput (void)
{
  // rate=100 → interval=10ms, capacity=1 → no burst.
  PLThrottle t;
  plThrottleInit (&t, 100, 1);

  PLLong start = nowMs ();
  for (PLInt i = 0; i < 4; ++i)
    {
      plThrottleAcquire (&t);
    }
  PLLong elapsed = nowMs () - start;

  // 4 acquires with no burst: first is free, next 3 each wait 10ms → ≥30ms.
  // NOTE: 25ms lower bound allows for timer jitter.
  PL_ASSERT (elapsed >= 25);
}

static void
testRateIsBounded (void)
{
  // The same 4 acquires at rate=100 should not take more than ~200ms
  // (leaving generous headroom for a slow CI machine).
  PLThrottle t;
  plThrottleInit (&t, 100, 1);

  PLLong start = nowMs ();
  for (PLInt i = 0; i < 4; ++i)
    {
      plThrottleAcquire (&t);
    }
  PLLong elapsed = nowMs () - start;

  PL_ASSERT (elapsed < 200);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Capacity caps token refill
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testCapacityCapsTokens (void)
{
  // Initialize, then manually set tokens above capacity.
  // The next acquire must clamp tokens back to capacity before consuming.
  PLThrottle t;
  // NOTE: capacity=300ms
  plThrottleInit (&t, 10, 3);
  // Force tokens well above capacity.
  t.tokens = t.capacity * 10;

  plThrottleAcquire (&t);

  // After one acquire, tokens must be <= capacity.
  PL_ASSERT (t.tokens <= t.capacity);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Main
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
main (void)
{
  PL_SUITE ("throttle - init");
  PL_RUN (testInitFields);
  PL_RUN (testInitLastIsRecent);

  PL_SUITE ("throttle - acquire (no sleep)");
  PL_RUN (testAcquireFullBucketNoSleep);
  PL_RUN (testAcquireDrainsTokens);
  PL_RUN (testCapacityCapsTokens);

  PL_SUITE ("throttle - rate limiting (~40ms real time)");
  PL_RUN (testRateLimitsThroughput);
  PL_RUN (testRateIsBounded);

  PL_SUMMARY ();
}
