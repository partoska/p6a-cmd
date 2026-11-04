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

#ifndef PKC_LIST_H
#define PKC_LIST_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "arg.h"
#include "partoska.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Escapes a string for a JSON string value (" and \ get a backslash).
 *
 * @param dst  Buffer; always NUL-terminated, truncated to fit.
 * @param size Capacity of dst in bytes.
 * @param src  NUL-terminated string.
 */
PKVoid pkcJsonEsc (PKChar *dst, PKSize size, const PKChar *src);

/**
 * Quotes a string for a CSV field (wraps it in "..." and doubles ").
 *
 * @param dst  Buffer; always NUL-terminated, truncated to fit.
 * @param size Capacity of dst in bytes.
 * @param src  NUL-terminated string.
 */
PKVoid pkcCsvQuote (PKChar *dst, PKSize size, const PKChar *src);

/**
 * Prints the output header for an event table in the given format.
 *
 * @param fmt Output format.
 */
PKVoid pkcPrintEventHeader (PKCArgFmt fmt);

/**
 * Prints a single event row in the given format.
 *
 * @param event Event to print.
 * @param fmt   Output format.
 * @param last  Whether this is the last row (used for JSON array
 *              formatting).
 */
PKVoid pkcPrintEventRow (const PKEvent *event, PKCArgFmt fmt, PKBool last);

/**
 * Prints the output footer for an event table in the given format.
 *
 * @param fmt Output format.
 */
PKVoid pkcPrintEventFooter (PKCArgFmt fmt);

/**
 * Fetches and prints events from Partoska.
 *
 * @param client   Logged-in client.
 * @param query    Optional search query string, or NULL.
 * @param owner    When true, restricts output to events owned by the user.
 * @param favorite When true, restricts output to favorited events.
 * @param contributor When true, restricts output to events where the user
 *                    has the contributor role.
 * @param fmt      Output format.
 * @return PK_OK on success, or a PK_E* error code on failure.
 */
PKInt pkcList (PKClient *client, const PKChar *query, PKBool owner,
               PKBool favorite, PKBool contributor, PKCArgFmt fmt);

#endif
