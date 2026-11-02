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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static const PKChar BASE64_CHARS[65]
    = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKSize
pkBase64EncodedLen (PKSize size)
{
  // Max. encoded length from input bytes (+ padding).
  return ((size + 2) / 3) * 4;
}

PKStatus
pkBase64Encode (PKChar *restrict str, PKSize len, const PKByte *restrict bytes,
                PKSize size)
{
  if (!str || !bytes)
    {
      return PK_EARG;
    }

  PKSize elen = pkBase64EncodedLen (size);
  if (len < elen)
    {
      return PK_EARG;
    }

  PKSize i = 0;
  PKSize j = 0;
  while (i < size)
    {
      PKDword octet_a = i < size ? bytes[i++] : 0;
      PKDword octet_b = i < size ? bytes[i++] : 0;
      PKDword octet_c = i < size ? bytes[i++] : 0;
      PKDword triple = (octet_a << 16) | (octet_b << 8) | octet_c;
      str[j++] = BASE64_CHARS[(triple >> 18) & 0x3F];
      str[j++] = BASE64_CHARS[(triple >> 12) & 0x3F];
      str[j++] = BASE64_CHARS[(triple >> 6) & 0x3F];
      str[j++] = BASE64_CHARS[triple & 0x3F];
    }

  // Add padding.
  PKSize mod = size % 3;
  if (mod == 1)
    {
      str[j - 1] = '=';
      str[j - 2] = '=';
    }
  else if (mod == 2)
    {
      str[j - 1] = '=';
    }

  // Optional null termination.
  if (len > j)
    {
      str[j] = '\0';
    }

  return PK_OK;
}
