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

#include "types.h"
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static const PLChar BASE64_CHARS[65]
    = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const PLChar BASE64_URL_CHARS[65]
    = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PLChar
plBase64Char (PLDword index)
{
  return BASE64_CHARS[index % PL_CHARSMAX (BASE64_CHARS)];
}

PLSize
plBase64EncodedLen (PLSize size)
{
  // Max. encoded length from input bytes (+ padding).
  return ((size + 2) / 3) * 4;
}

PLSize
plBase64DecodedSize (PLSize len)
{
  // Max. decoded size from encoded length.
  return (len * 3) / 4;
}

PLInt
plBase64Encode (PLChar *restrict str, PLSize len, const PLByte *restrict bytes,
                PLSize size)
{
  if (!str || !bytes)
    {
      return PL_EMEM;
    }

  PLSize elen = plBase64EncodedLen (size);
  if (len < elen)
    {
      return PL_EMEM;
    }

  PLSize i = 0;
  PLSize j = 0;
  while (i < size)
    {
      PLDword octet_a = i < size ? bytes[i++] : 0;
      PLDword octet_b = i < size ? bytes[i++] : 0;
      PLDword octet_c = i < size ? bytes[i++] : 0;
      PLDword triple = (octet_a << 16) | (octet_b << 8) | octet_c;
      str[j++] = BASE64_CHARS[(triple >> 18) & 0x3F];
      str[j++] = BASE64_CHARS[(triple >> 12) & 0x3F];
      str[j++] = BASE64_CHARS[(triple >> 6) & 0x3F];
      str[j++] = BASE64_CHARS[triple & 0x3F];
    }

  // Add padding.
  PLSize mod = size % 3;
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

  return PL_EOK;
}

PLInt
plBase64UrlEncode (PLChar *restrict str, PLSize len,
                   const PLByte *restrict bytes, PLSize size)
{
  if (!str || !bytes)
    {
      return PL_EMEM;
    }

  PLSize elen = plBase64EncodedLen (size);
  if (len < elen)
    {
      return PL_EMEM;
    }

  PLSize i = 0;
  PLSize j = 0;
  while (i < size)
    {
      PLDword octa = i < size ? bytes[i++] : 0;
      PLDword octb = i < size ? bytes[i++] : 0;
      PLDword octc = i < size ? bytes[i++] : 0;
      PLDword triple = (octa << 16) | (octb << 8) | octc;
      str[j++] = BASE64_URL_CHARS[(triple >> 18) & 0x3F];
      str[j++] = BASE64_URL_CHARS[(triple >> 12) & 0x3F];
      str[j++] = BASE64_URL_CHARS[(triple >> 6) & 0x3F];
      str[j++] = BASE64_URL_CHARS[triple & 0x3F];
    }

  // Remove padding.
  PLSize mod = size % 3;
  if (mod > 0)
    {
      j -= (3 - mod);
    }

  // Optional null termination.
  if (len > j)
    {
      str[j] = '\0';
    }

  return PL_EOK;
}

PLBool
plBase64Decode (PLByte *restrict bytes, PLSize size,
                const PLChar *restrict str, PLSize len)
{
  if (!bytes || !str)
    {
      return PL_EMEM;
    }

  PLSize dsize = plBase64DecodedSize (len);
  if (size < dsize)
    {
      return PL_EMEM;
    }

  // Decode table.
  static PLBool initialized = PL_FALSE;
  static PLByte table[256];
  if (!initialized)
    {
      memset (table, 0xFF, sizeof (table));
      for (PLSize i = 0; i < sizeof (BASE64_CHARS) - 1; ++i)
        {
          table[(PLByte)BASE64_CHARS[i]] = i;
        }
      table[(PLByte)'='] = 0;

      initialized = PL_TRUE;
    }

  PLSize i = 0;
  PLSize j = 0;
  while (i < len)
    {
      PLDword sexta = table[(PLByte)str[i++]];
      PLDword sextb = i < len ? table[(PLByte)str[i++]] : 0;
      PLDword sextc = i < len ? table[(PLByte)str[i++]] : 0;
      PLDword sextd = i < len ? table[(PLByte)str[i++]] : 0;
      PLDword triple = (sexta << 18) | (sextb << 12) | (sextc << 6) | sextd;
      if (j < dsize)
        {
          bytes[j++] = (triple >> 16) & 0xFF;
        }
      if (j < dsize)
        {
          bytes[j++] = (triple >> 8) & 0xFF;
        }
      if (j < dsize)
        {
          bytes[j++] = triple & 0xFF;
        }
    }

  return PL_EOK;
}

PLBool
plBase64UrlDecode (PLByte *restrict bytes, PLSize size,
                   const PLChar *restrict str, PLSize len)
{
  if (!bytes || !str)
    {
      return PL_EMEM;
    }

  PLSize dsize = plBase64DecodedSize (len);
  if (size < dsize)
    {
      return PL_EMEM;
    }

  // Create decode table.
  static PLBool initialized = PL_FALSE;
  static PLByte table[256];
  if (!initialized)
    {
      memset (table, 0xFF, sizeof (table));
      for (PLSize i = 0; i < sizeof (BASE64_URL_CHARS) - 1; ++i)
        {
          table[(PLByte)BASE64_URL_CHARS[i]] = i;
        }

      initialized = PL_TRUE;
    }

  PLSize i = 0;
  PLSize j = 0;
  while (i < len)
    {
      PLDword sextet_a = table[(PLByte)str[i++]];
      PLDword sextet_b = i < len ? table[(PLByte)str[i++]] : 0;
      PLDword sextet_c = i < len ? table[(PLByte)str[i++]] : 0;
      PLDword sextet_d = i < len ? table[(PLByte)str[i++]] : 0;
      PLDword triple
          = (sextet_a << 18) | (sextet_b << 12) | (sextet_c << 6) | sextet_d;
      if (j < dsize)
        {
          bytes[j++] = (triple >> 16) & 0xFF;
        }
      if (j < dsize)
        {
          bytes[j++] = (triple >> 8) & 0xFF;
        }
      if (j < dsize)
        {
          bytes[j++] = triple & 0xFF;
        }
    }

  return PL_EOK;
}
