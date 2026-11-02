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

#include "base64.h"
#include "test_runner.h"
#include <stdio.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Helpers
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Encode helper: returns the encoded string in a static buffer. */
static const PKChar *
b64Enc (const PKChar *input)
{
  static PKChar buf[256];
  PKSize size = strlen (input);
  PKSize len = pkBase64EncodedLen (size) + 1;
  pkBase64Encode (buf, len, (const PKByte *)input, size);
  return buf;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Encode length
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testEncodedLen (void)
{
  // 0 bytes → 0 output chars
  PK_ASSERT_EQ (pkBase64EncodedLen (0), (PKSize)0);
  // 1 byte → 4 chars (padded to multiple of 4)
  PK_ASSERT_EQ (pkBase64EncodedLen (1), (PKSize)4);
  // 2 bytes → 4 chars
  PK_ASSERT_EQ (pkBase64EncodedLen (2), (PKSize)4);
  // 3 bytes → 4 chars (perfect block)
  PK_ASSERT_EQ (pkBase64EncodedLen (3), (PKSize)4);
  // 4 bytes → 8 chars
  PK_ASSERT_EQ (pkBase64EncodedLen (4), (PKSize)8);
  // 6 bytes → 8 chars (two perfect blocks)
  PK_ASSERT_EQ (pkBase64EncodedLen (6), (PKSize)8);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - RFC 4648 encode vectors
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testEncodeRfc4648 (void)
{
  // RFC 4648 §10 test vectors for standard base64.
  PK_ASSERT_STR_EQ (b64Enc (""), "");
  PK_ASSERT_STR_EQ (b64Enc ("f"), "Zg==");
  PK_ASSERT_STR_EQ (b64Enc ("fo"), "Zm8=");
  PK_ASSERT_STR_EQ (b64Enc ("foo"), "Zm9v");
  PK_ASSERT_STR_EQ (b64Enc ("foob"), "Zm9vYg==");
  PK_ASSERT_STR_EQ (b64Enc ("fooba"), "Zm9vYmE=");
  PK_ASSERT_STR_EQ (b64Enc ("foobar"), "Zm9vYmFy");
}

static PKVoid
testEncodeBinary (void)
{
  // All-zero bytes.
  const PKByte zeros[3] = { 0x00, 0x00, 0x00 };
  PKChar buf[8];
  PK_ASSERT_EQ (pkBase64Encode (buf, sizeof (buf), zeros, 3), PK_OK);
  PK_ASSERT_STR_EQ (buf, "AAAA");

  // All-ones bytes.
  const PKByte ones[3] = { 0xFF, 0xFF, 0xFF };
  PK_ASSERT_EQ (pkBase64Encode (buf, sizeof (buf), ones, 3), PK_OK);
  PK_ASSERT_STR_EQ (buf, "////");

  // Mixed bytes: 0x00 0x10 0x83 → base64 "ABCD"
  const PKByte abcd[3] = { 0x00, 0x10, 0x83 };
  PK_ASSERT_EQ (pkBase64Encode (buf, sizeof (buf), abcd, 3), PK_OK);
  PK_ASSERT_STR_EQ (buf, "ABCD");
}

static PKVoid
testEncodeBufferTooSmall (void)
{
  // Buffer one byte too small should fail.
  PKChar buf[3]; // needs 4 chars + NUL = 5, so 3 is definitely too small
  const PKByte data[1] = { 0x42 };
  PK_ASSERT (pkBase64Encode (buf, sizeof (buf), data, 1) != PK_OK);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Main
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
main (void)
{
  PK_SUITE ("base64 - encoded length");
  PK_RUN (testEncodedLen);

  PK_SUITE ("base64 - standard encode");
  PK_RUN (testEncodeRfc4648);
  PK_RUN (testEncodeBinary);
  PK_RUN (testEncodeBufferTooSmall);

  PK_SUMMARY ();
}
