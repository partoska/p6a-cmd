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

#include "hash.h"
#include "test_runner.h"
#include <stdio.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Helpers
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* One-shot SHA-256 helper. */
static void
sha256 (const void *data, PLSize len, PLByte digest[PL_HASH_DIGEST_SIZE])
{
  PLHashCtx ctx;
  plHashInit (&ctx);
  plHashUpdate (&ctx, (const PLByte *)data, len);
  plHashFinal (&ctx, digest);
}

/* Parse a lowercase hex string into bytes. */
static void
hexToBytes (const PLChar *hex, PLByte *out, PLSize n)
{
  for (PLSize i = 0; i < n; ++i)
    {
      PLDword byte;
      sscanf (hex + i * 2, "%02x", &byte);
      out[i] = (PLByte)byte;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - NIST FIPS 180-4 vectors
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testEmptyString (void)
{
  // SHA-256("") =
  // e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
  const PLChar *hex
      = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
  PLByte expected[PL_HASH_DIGEST_SIZE];
  PLByte actual[PL_HASH_DIGEST_SIZE];

  hexToBytes (hex, expected, PL_HASH_DIGEST_SIZE);
  sha256 ("", 0, actual);

  PL_ASSERT_MEM_EQ (actual, expected, PL_HASH_DIGEST_SIZE);
}

static void
testAbc (void)
{
  // SHA-256("abc") =
  // ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
  const PLChar *hex
      = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
  PLByte expected[PL_HASH_DIGEST_SIZE];
  PLByte actual[PL_HASH_DIGEST_SIZE];

  hexToBytes (hex, expected, PL_HASH_DIGEST_SIZE);
  sha256 ("abc", 3, actual);

  PL_ASSERT_MEM_EQ (actual, expected, PL_HASH_DIGEST_SIZE);
}

static void
testLongMessage (void)
{
  // SHA-256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")
  // = 248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1
  const PLChar *msg
      = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
  const PLChar *hex
      = "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";
  PLByte expected[PL_HASH_DIGEST_SIZE];
  PLByte actual[PL_HASH_DIGEST_SIZE];

  hexToBytes (hex, expected, PL_HASH_DIGEST_SIZE);
  sha256 (msg, strlen (msg), actual);

  PL_ASSERT_MEM_EQ (actual, expected, PL_HASH_DIGEST_SIZE);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Streaming (multi-update)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testStreamingEqualsOneshot (void)
{
  // Feeding data in chunks must produce the same digest as one shot.
  const PLChar *msg = "The quick brown fox jumps over the lazy dog";
  PLSize len = strlen (msg);

  PLByte oneshot[PL_HASH_DIGEST_SIZE];
  sha256 (msg, len, oneshot);

  // Feed byte-by-byte.
  PLHashCtx ctx;
  PLByte streamed[PL_HASH_DIGEST_SIZE];
  plHashInit (&ctx);
  for (PLSize i = 0; i < len; ++i)
    {
      plHashUpdate (&ctx, (const PLByte *)msg + i, 1);
    }
  plHashFinal (&ctx, streamed);

  PL_ASSERT_MEM_EQ (streamed, oneshot, PL_HASH_DIGEST_SIZE);
}

static void
testStreamingChunks (void)
{
  // Feed in irregular chunk sizes; result must match one-shot.
  const PLChar *msg = "abcdefghijklmnopqrstuvwxyz0123456789";
  PLSize len = strlen (msg);
  PLSize cuts[] = { 1, 7, 3, 10, len - 21 }; // sum = len

  PLByte oneshot[PL_HASH_DIGEST_SIZE];
  sha256 (msg, len, oneshot);

  PLHashCtx ctx;
  PLByte chunked[PL_HASH_DIGEST_SIZE];
  PLSize offset = 0;
  plHashInit (&ctx);
  for (PLSize i = 0; i < sizeof (cuts) / sizeof (cuts[0]); ++i)
    {
      plHashUpdate (&ctx, (const PLByte *)msg + offset, cuts[i]);
      offset += cuts[i];
    }
  plHashFinal (&ctx, chunked);

  PL_ASSERT_MEM_EQ (chunked, oneshot, PL_HASH_DIGEST_SIZE);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Distinct inputs produce distinct digests
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testDistinctInputs (void)
{
  PLByte d1[PL_HASH_DIGEST_SIZE];
  PLByte d2[PL_HASH_DIGEST_SIZE];

  sha256 ("abc", 3, d1);
  sha256 ("ABC", 3, d2);
  PL_ASSERT (memcmp (d1, d2, PL_HASH_DIGEST_SIZE) != 0);

  sha256 ("a", 1, d1);
  sha256 ("aa", 2, d2);
  PL_ASSERT (memcmp (d1, d2, PL_HASH_DIGEST_SIZE) != 0);
}

static void
testDeterministic (void)
{
  // Same input hashed twice must produce the same digest.
  PLByte d1[PL_HASH_DIGEST_SIZE];
  PLByte d2[PL_HASH_DIGEST_SIZE];

  sha256 ("hello world", 11, d1);
  sha256 ("hello world", 11, d2);
  PL_ASSERT_MEM_EQ (d1, d2, PL_HASH_DIGEST_SIZE);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Cross-boundary blocks
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testBlockBoundary (void)
{
  // SHA-256 processes 64-byte blocks. Test messages that straddle the
  // boundary to exercise the buffering and carry-over logic.
  PLByte a[PL_HASH_DIGEST_SIZE];
  PLByte b[PL_HASH_DIGEST_SIZE];

  // 63 bytes (one byte short of a full block).
  const PLChar msg63[63]
      = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  sha256 (msg63, 63, a);

  // 64 bytes (exactly one full block).
  const PLChar msg64[64]
      = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  sha256 (msg64, 64, b);

  PL_ASSERT (memcmp (a, b, PL_HASH_DIGEST_SIZE) != 0);

  // 65 bytes (one byte into the second block).
  PLByte c[PL_HASH_DIGEST_SIZE];
  const PLChar msg65[65]
      = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  sha256 (msg65, 65, c);
  PL_ASSERT (memcmp (b, c, PL_HASH_DIGEST_SIZE) != 0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Main
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
main (void)
{
  PL_SUITE ("hash - NIST FIPS 180-4 vectors");
  PL_RUN (testEmptyString);
  PL_RUN (testAbc);
  PL_RUN (testLongMessage);

  PL_SUITE ("hash - streaming");
  PL_RUN (testStreamingEqualsOneshot);
  PL_RUN (testStreamingChunks);

  PL_SUITE ("hash - properties");
  PL_RUN (testDistinctInputs);
  PL_RUN (testDeterministic);
  PL_RUN (testBlockBoundary);

  PL_SUMMARY ();
}
