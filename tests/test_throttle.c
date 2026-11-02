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

#include "plat.h"
#include "test_runner.h"
#include "throttle.h"
#include <stdio.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Helpers
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Take a token, sleeping as long as the throttle asks. */
static PKVoid
acquire (PKThrottle *t)
{
  PKDword wait;
  while ((wait = pkThrottleTry (t)) > 0)
    {
      pkSleep ((PKLong)wait);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Init
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testInitFields (void)
{
  PKThrottle t;

  // rate=10, capacity=5 → interval=100ms, bucket=500ms, tokens full
  pkThrottleInit (&t, 10, 5);
  PK_ASSERT_EQ (t.interval, (PKDword)100);
  PK_ASSERT_EQ (t.capacity, (PKDword)500);
  PK_ASSERT_EQ (t.tokens, t.capacity);

  // rate=100, capacity=3 → interval=10ms, bucket=30ms
  pkThrottleInit (&t, 100, 3);
  PK_ASSERT_EQ (t.interval, (PKDword)10);
  PK_ASSERT_EQ (t.capacity, (PKDword)30);
  PK_ASSERT_EQ (t.tokens, t.capacity);

  // rate=1000, capacity=1 → interval=1ms, bucket=1ms
  pkThrottleInit (&t, 1000, 1);
  PK_ASSERT_EQ (t.interval, (PKDword)1);
  PK_ASSERT_EQ (t.capacity, (PKDword)1);
  PK_ASSERT_EQ (t.tokens, t.capacity);
}

static PKVoid
testInitLastIsRecent (void)
{
  // t.last is a reading of pkClockMs() taken between before and after;
  // unsigned differences stay right when the clock wraps in between.
  PKThrottle t;
  PKDword before = pkClockMs ();
  pkThrottleInit (&t, 10, 5);
  PKDword after = pkClockMs ();
  PK_ASSERT ((PKDword)(t.last - before) <= (PKDword)(after - before));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Acquire from a full bucket (no sleeping)
 *
 * A freshly initialized throttle has tokens == capacity.  Each
 * acquire consumes one interval worth of tokens, so we can drain
 * the bucket (capacity) times without ever sleeping.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testAcquireFullBucketNoSleep (void)
{
  // rate=1000 (interval=1ms), capacity=20 → 20ms of credit up front.
  // 20 back-to-back acquires should complete in well under 20ms.
  PKThrottle t;
  pkThrottleInit (&t, 1000, 20);

  PKDword start = pkClockMs ();
  for (PKInt i = 0; i < 20; ++i)
    {
      acquire (&t);
    }
  PKDword elapsed = pkClockMs () - start;

  // Should complete in under 10ms — 20 trivial C calls take ~microseconds.
  PK_ASSERT (elapsed < 10);
}

static PKVoid
testAcquireDrainsTokens (void)
{
  // After draining the bucket, tokens should be near zero.
  PKThrottle t;
  // NOTE: interval=1ms, capacity=5ms, tokens=5
  pkThrottleInit (&t, 1000, 5);

  for (PKInt i = 0; i < 5; ++i)
    {
      acquire (&t);
    }

  // Tokens have been consumed; remaining balance is < one interval.
  PK_ASSERT (t.tokens < t.interval);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Rate limiting (real time, short sleep)
 *
 * With capacity=1 there is no burst credit: every acquire after the
 * first must wait one full interval.  We use rate=100 (10ms interval)
 * and do 4 acquires → expect at least 3 × 10ms = 30ms wall time.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testRateLimitsThroughput (void)
{
  // rate=100 → interval=10ms, capacity=1 → no burst.
  PKThrottle t;
  pkThrottleInit (&t, 100, 1);

  PKDword start = pkClockMs ();
  for (PKInt i = 0; i < 4; ++i)
    {
      acquire (&t);
    }
  PKDword elapsed = pkClockMs () - start;

  // 4 acquires with no burst: first is free, next 3 each wait 10ms → ≥30ms.
  // NOTE: 25ms lower bound allows for timer jitter.
  PK_ASSERT (elapsed >= 25);
}

static PKVoid
testRateIsBounded (void)
{
  // The same 4 acquires at rate=100 should not take more than ~200ms
  // (leaving generous headroom for a slow CI machine).
  PKThrottle t;
  pkThrottleInit (&t, 100, 1);

  PKDword start = pkClockMs ();
  for (PKInt i = 0; i < 4; ++i)
    {
      acquire (&t);
    }
  PKDword elapsed = pkClockMs () - start;

  PK_ASSERT (elapsed < 200);
}

static PKVoid
testTryReportsWait (void)
{
  // Without credit, a try takes nothing and tells how long to wait.
  PKThrottle t;
  pkThrottleInit (&t, 10, 1);

  PK_ASSERT_EQ (pkThrottleTry (&t), 0u);
  PKDword wait = pkThrottleTry (&t);
  PK_ASSERT (wait > 0 && wait <= 100);
}

static PKVoid
testClockWrap (void)
{
  // A reading behind the last one means the clock wrapped around: the
  // difference is huge, and the credit is capped, not overflowed.
  PKThrottle t;
  pkThrottleInit (&t, 10, 3);
  t.tokens = 0;
  t.last = pkClockMs () + 1000u;

  PK_ASSERT_EQ (pkThrottleTry (&t), 0u);
  PK_ASSERT_EQ (t.tokens, t.capacity - t.interval);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Capacity caps token refill
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testCapacityCapsTokens (void)
{
  // Initialize, then manually set tokens above capacity.
  // The next acquire must clamp tokens back to capacity before consuming.
  PKThrottle t;
  // NOTE: capacity=300ms
  pkThrottleInit (&t, 10, 3);
  // Force tokens well above capacity.
  t.tokens = t.capacity * 10;

  acquire (&t);

  // After one acquire, tokens must be <= capacity.
  PK_ASSERT (t.tokens <= t.capacity);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Main
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
main (void)
{
  PK_SUITE ("throttle - init");
  PK_RUN (testInitFields);
  PK_RUN (testInitLastIsRecent);

  PK_SUITE ("throttle - acquire (no sleep)");
  PK_RUN (testAcquireFullBucketNoSleep);
  PK_RUN (testAcquireDrainsTokens);
  PK_RUN (testCapacityCapsTokens);

  PK_SUITE ("throttle - rate limiting (~40ms real time)");
  PK_RUN (testRateLimitsThroughput);
  PK_RUN (testRateIsBounded);
  PK_RUN (testTryReportsWait);
  PK_RUN (testClockWrap);

  PK_SUMMARY ();
}
