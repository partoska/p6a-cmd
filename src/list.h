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

#ifndef LIST_H
#define LIST_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "arg.h"
#include "types.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Prints the output header for an event table in the given format.
 *
 * @param fmt Output format.
 */
void plPrintEventHeader (PLArgFmt fmt);

/**
 * Prints a single event row in the given format.
 *
 * @param name     Event display name.
 * @param id       Event UUID.
 * @param from     Event start date string.
 * @param to       Event end date string.
 * @param owner    Whether the user owns the event.
 * @param favorite Whether the event is favorited.
 * @param pub      Whether the event is public.
 * @param guests   Number of guests.
 * @param media    Number of media items.
 * @param fmt      Output format.
 * @param last     Whether this is the last row (used for JSON array
 *                 formatting).
 */
void plPrintEventRow (const PLChar *name, const PLChar *id, const PLChar *from,
                      const PLChar *to, PLBool owner, PLBool favorite,
                      PLBool pub, PLInt guests, PLInt media, PLArgFmt fmt,
                      PLBool last);

/**
 * Prints the output footer for an event table in the given format.
 *
 * @param fmt Output format.
 */
void plPrintEventFooter (PLArgFmt fmt);

/**
 * Fetches and prints events from Partoska.
 *
 * @param base     API base URL.
 * @param bearer   OAuth bearer token.
 * @param query    Optional search query string, or NULL.
 * @param owner    When true, restricts output to events owned by the user.
 * @param favorite When true, restricts output to favorited events.
 * @param fmt      Output format.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plList (const PLChar *base, const PLChar *bearer, const PLChar *query,
              PLBool owner, PLBool favorite, PLArgFmt fmt);

#endif
