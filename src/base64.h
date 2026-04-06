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

#ifndef BASE64_H
#define BASE64_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "types.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define PL_BASE64_ENCODED_LEN(a) ((((a) + 2) / 3) * 4)
#define PL_BASE64_DECODED_SIZE(s) (((s) * 3) / 4)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Returns the base64 character for the given 6-bit index.
 *
 * @param indexs 6-bit index (0–63).
 * @return Corresponding base64 character.
 */
PLChar plBase64Char (PLDword indexs);

/**
 * Computes the length of a base64-encoded string for a given byte count.
 *
 * @param size Number of input bytes.
 * @return Required string length (excluding the null terminator).
 */
PLSize plBase64EncodedLen (PLSize size);

/**
 * Computes the maximum decoded byte count for a given base64 string length.
 *
 * @param len Base64 string length.
 * @return Maximum number of decoded bytes.
 */
PLSize plBase64DecodedSize (PLSize len);

/**
 * Encodes bytes as standard base64 into a caller-supplied string buffer.
 *
 * @param str   Output buffer.
 * @param len   Capacity of the output buffer including the null terminator.
 * @param bytes Input bytes to encode.
 * @param size  Number of input bytes.
 * @return PL_TRUE on success, PL_FALSE if the buffer is too small.
 */
PLBool plBase64Encode (PLChar *str, PLSize len, const PLByte *bytes,
                       PLSize size);

/**
 * Encodes bytes as URL-safe base64 (RFC 4648 §5) into a caller-supplied
 * buffer.
 *
 * @param str   Output buffer.
 * @param len   Capacity of the output buffer including the null terminator.
 * @param bytes Input bytes to encode.
 * @param size  Number of input bytes.
 * @return PL_TRUE on success, PL_FALSE if the buffer is too small.
 */
PLBool plBase64UrlEncode (PLChar *str, PLSize len, const PLByte *bytes,
                          PLSize size);

/**
 * Decodes a standard base64 string into a caller-supplied byte buffer.
 *
 * @param bytes Output buffer.
 * @param size  Capacity of the output buffer in bytes.
 * @param str   Base64 string to decode.
 * @param len   Length of the base64 string.
 * @return PL_TRUE on success, PL_FALSE on invalid input or insufficient
 * buffer.
 */
PLBool plBase64Decode (PLByte *bytes, PLSize size, const PLChar *str,
                       PLSize len);

/**
 * Decodes a URL-safe base64 string into a caller-supplied byte buffer.
 *
 * @param bytes Output buffer.
 * @param size  Capacity of the output buffer in bytes.
 * @param str   URL-safe base64 string to decode.
 * @param len   Length of the base64 string.
 * @return PL_TRUE on success, PL_FALSE on invalid input or insufficient
 * buffer.
 */
PLBool plBase64UrlDecode (PLByte *bytes, PLSize size, const PLChar *str,
                          PLSize len);

#endif
