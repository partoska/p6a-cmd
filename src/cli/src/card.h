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

#ifndef PKC_CARD_H
#define PKC_CARD_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "partoska.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Downloads the share card for an event and writes it to a file.
 *
 * The output path defaults to "<id>-card.pdf" (or "<id>-card.jpg") in the
 * current directory if output is NULL.
 *
 * @param client   Logged-in client.
 * @param event  UUID of the event.
 * @param output Destination file path, or NULL to use the default name.
 * @param design Card design theme (bday, tech, match, forest, garden, gold,
 *               romantic, silver, neon, nineties, cottage, wish, velvet,
 *               hallows).
 * @param locale Language for card text (en, cs, sk, pl, ru, es, de, fr, it,
 *               nl, uk), or NULL for server default.
 * @param layout Card layout (single, business), or NULL for server default.
 * @param paper  Paper size (a4, letter), or NULL for server default.
 * @param jpg    When true, downloads JPEG format instead of PDF.
 * @param nobg   When true, requests a white background.
 * @return PK_OK on success, or a PK_E* error code on failure.
 */
PKInt pkcCard (PKClient *client, const PKChar *event, const PKChar *output,
               const PKChar *design, const PKChar *locale,
               const PKChar *layout, const PKChar *paper, PKBool jpg,
               PKBool nobg);

#endif
