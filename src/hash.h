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

#ifndef HASH_H
#define HASH_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "types.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define PL_HASH_BLOCK_SIZE (64)
#define PL_HASH_STATE_SIZE (8)
#define PL_HASH_DIGEST_SIZE (32)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct PLHashCtx
{
  PLDword state[PL_HASH_STATE_SIZE];
  PLQword count;
  PLByte buff[PL_HASH_BLOCK_SIZE];
} PLHashCtx;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Initializes a SHA-256 hash context.
 *
 * @param ctx Hash context to initialize.
 */
void plHashInit (PLHashCtx *ctx);

/**
 * Feeds data into a SHA-256 hash context.
 *
 * @param ctx  Hash context to update.
 * @param data Pointer to the input data.
 * @param len  Number of bytes to process.
 */
void plHashUpdate (PLHashCtx *ctx, const PLByte *data, PLSize len);

/**
 * Finalizes the SHA-256 computation and writes the digest.
 *
 * @param ctx    Hash context to finalize.
 * @param digest Output buffer of at least PL_HASH_DIGEST_SIZE (32) bytes.
 */
void plHashFinal (PLHashCtx *ctx, PLByte *digest);

#endif
