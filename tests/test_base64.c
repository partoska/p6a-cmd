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

#include "base64.h"
#include "test_runner.h"
#include <stdio.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Helpers
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Encode helper: returns the encoded string in a static buffer. */
static const PLChar *
b64Enc (const PLChar *input)
{
  static PLChar buf[256];
  PLSize size = strlen (input);
  PLSize len = plBase64EncodedLen (size) + 1;
  plBase64Encode (buf, len, (const PLByte *)input, size);
  return buf;
}

/* URL-safe encode helper. */
static const PLChar *
b64Url (const PLChar *input)
{
  static PLChar buf[256];
  PLSize size = strlen (input);
  PLSize len = plBase64EncodedLen (size) + 1;
  plBase64UrlEncode (buf, len, (const PLByte *)input, size);
  return buf;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Encode length
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testEncodedLen (void)
{
  // 0 bytes → 0 output chars
  PL_ASSERT_EQ (plBase64EncodedLen (0), (PLSize)0);
  // 1 byte → 4 chars (padded to multiple of 4)
  PL_ASSERT_EQ (plBase64EncodedLen (1), (PLSize)4);
  // 2 bytes → 4 chars
  PL_ASSERT_EQ (plBase64EncodedLen (2), (PLSize)4);
  // 3 bytes → 4 chars (perfect block)
  PL_ASSERT_EQ (plBase64EncodedLen (3), (PLSize)4);
  // 4 bytes → 8 chars
  PL_ASSERT_EQ (plBase64EncodedLen (4), (PLSize)8);
  // 6 bytes → 8 chars (two perfect blocks)
  PL_ASSERT_EQ (plBase64EncodedLen (6), (PLSize)8);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - RFC 4648 encode vectors
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testEncodeRfc4648 (void)
{
  // RFC 4648 §10 test vectors for standard base64.
  PL_ASSERT_STR_EQ (b64Enc (""), "");
  PL_ASSERT_STR_EQ (b64Enc ("f"), "Zg==");
  PL_ASSERT_STR_EQ (b64Enc ("fo"), "Zm8=");
  PL_ASSERT_STR_EQ (b64Enc ("foo"), "Zm9v");
  PL_ASSERT_STR_EQ (b64Enc ("foob"), "Zm9vYg==");
  PL_ASSERT_STR_EQ (b64Enc ("fooba"), "Zm9vYmE=");
  PL_ASSERT_STR_EQ (b64Enc ("foobar"), "Zm9vYmFy");
}

static void
testEncodeBinary (void)
{
  // All-zero bytes.
  const PLByte zeros[3] = { 0x00, 0x00, 0x00 };
  PLChar buf[8];
  PL_ASSERT_EQ (plBase64Encode (buf, sizeof (buf), zeros, 3), PL_EOK);
  PL_ASSERT_STR_EQ (buf, "AAAA");

  // All-ones bytes.
  const PLByte ones[3] = { 0xFF, 0xFF, 0xFF };
  PL_ASSERT_EQ (plBase64Encode (buf, sizeof (buf), ones, 3), PL_EOK);
  PL_ASSERT_STR_EQ (buf, "////");

  // Mixed bytes: 0x00 0x10 0x83 → base64 "ABCD"
  const PLByte abcd[3] = { 0x00, 0x10, 0x83 };
  PL_ASSERT_EQ (plBase64Encode (buf, sizeof (buf), abcd, 3), PL_EOK);
  PL_ASSERT_STR_EQ (buf, "ABCD");
}

static void
testEncodeBufferTooSmall (void)
{
  // Buffer one byte too small should fail.
  PLChar buf[3]; // needs 4 chars + NUL = 5, so 3 is definitely too small
  const PLByte data[1] = { 0x42 };
  PL_ASSERT (plBase64Encode (buf, sizeof (buf), data, 1) != PL_EOK);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - RFC 4648 decode
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testDecodeRfc4648 (void)
{
  PLByte out[64];
  memset (out, 0, sizeof (out));

  // "Zm9v" → "foo"
  PL_ASSERT_EQ (plBase64Decode (out, sizeof (out), "Zm9v", 4), PL_EOK);
  PL_ASSERT_EQ (memcmp (out, "foo", 3), 0);

  // "Zm9vYmFy" → "foobar"
  PL_ASSERT_EQ (plBase64Decode (out, sizeof (out), "Zm9vYmFy", 8), PL_EOK);
  PL_ASSERT_EQ (memcmp (out, "foobar", 6), 0);

  // "Zg==" → "f"
  PL_ASSERT_EQ (plBase64Decode (out, sizeof (out), "Zg==", 4), PL_EOK);
  PL_ASSERT_EQ (out[0], (PLByte)'f');

  // "AAAA" → 0x00 0x00 0x00
  memset (out, 0xFF, sizeof (out));
  PL_ASSERT_EQ (plBase64Decode (out, sizeof (out), "AAAA", 4), PL_EOK);
  PL_ASSERT_EQ (out[0], 0x00);
  PL_ASSERT_EQ (out[1], 0x00);
  PL_ASSERT_EQ (out[2], 0x00);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Roundtrip
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testRoundtrip (void)
{
  const PLChar *inputs[] = { "Hello, World!", "p6a", "\x01\x02\x03\x04",
                             "The quick brown fox jumps over the lazy dog" };
  PLSize n = sizeof (inputs) / sizeof (inputs[0]);

  for (PLSize i = 0; i < n; ++i)
    {
      PLSize size = strlen (inputs[i]);
      PLSize elen = plBase64EncodedLen (size) + 1;
      PLChar enc[256];
      PLByte dec[256];

      PL_ASSERT_EQ (
          plBase64Encode (enc, elen, (const PLByte *)inputs[i], size), PL_EOK);
      PL_ASSERT_EQ (plBase64Decode (dec, plBase64DecodedSize (strlen (enc)),
                                    enc, strlen (enc)),
                    PL_EOK);
      PL_ASSERT_EQ (memcmp (dec, inputs[i], size), 0);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - URL-safe encode (RFC 4648 §5, no padding)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testUrlEncode (void)
{
  // URL-safe variant: '+' → '-', '/' → '_', padding removed.
  PL_ASSERT_STR_EQ (b64Url (""), "");
  PL_ASSERT_STR_EQ (b64Url ("f"), "Zg");
  PL_ASSERT_STR_EQ (b64Url ("fo"), "Zm8");
  PL_ASSERT_STR_EQ (b64Url ("foo"), "Zm9v");
  PL_ASSERT_STR_EQ (b64Url ("foob"), "Zm9vYg");
  PL_ASSERT_STR_EQ (b64Url ("fooba"), "Zm9vYmE");
  PL_ASSERT_STR_EQ (b64Url ("foobar"), "Zm9vYmFy");
}

static void
testUrlEncodeSpecialChars (void)
{
  // Bytes that produce '+' in standard base64 become '-' in URL-safe.
  // NOTE: "+/+/" in std
  const PLByte plusBytes[3] = { 0xFB, 0xEF, 0xBE };
  PLChar buf[8];
  PL_ASSERT_EQ (plBase64UrlEncode (buf, sizeof (buf), plusBytes, 3), PL_EOK);
  // Verify no '+' or '/' appear in the URL-safe output.
  PL_ASSERT (strchr (buf, '+') == NULL);
  PL_ASSERT (strchr (buf, '/') == NULL);
  PL_ASSERT (strchr (buf, '=') == NULL);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - URL-safe decode
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testUrlDecode (void)
{
  PLByte out[64];

  // "Zm9v" → "foo"
  PL_ASSERT_EQ (plBase64UrlDecode (out, sizeof (out), "Zm9v", 4), PL_EOK);
  PL_ASSERT_EQ (memcmp (out, "foo", 3), 0);

  // "Zm9vYmFy" → "foobar"
  PL_ASSERT_EQ (plBase64UrlDecode (out, sizeof (out), "Zm9vYmFy", 8), PL_EOK);
  PL_ASSERT_EQ (memcmp (out, "foobar", 6), 0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Main
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
main (void)
{
  PL_SUITE ("base64 - encoded length");
  PL_RUN (testEncodedLen);

  PL_SUITE ("base64 - standard encode");
  PL_RUN (testEncodeRfc4648);
  PL_RUN (testEncodeBinary);
  PL_RUN (testEncodeBufferTooSmall);

  PL_SUITE ("base64 - standard decode");
  PL_RUN (testDecodeRfc4648);

  PL_SUITE ("base64 - roundtrip");
  PL_RUN (testRoundtrip);

  PL_SUITE ("base64 - URL-safe encode");
  PL_RUN (testUrlEncode);
  PL_RUN (testUrlEncodeSpecialChars);

  PL_SUITE ("base64 - URL-safe decode");
  PL_RUN (testUrlDecode);

  PL_SUMMARY ();
}
