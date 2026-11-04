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

#ifndef PKC_TRUSTED_H
#define PKC_TRUSTED_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "arg.h"
#include "partoska.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Prints the trusted list of an event, or changes it and prints the
 * list the server saved. A change empties the list (clear: every entry not
 * added again is removed), removes entries, then adds entries, one request
 * each; each value may hold several entries, separated by commas,
 * semicolons or white space. Adding also approves the pending requests the
 * list matches.
 *
 * @param client  Logged-in client.
 * @param event   UUID of the event.
 * @param add     Values with entries to add.
 * @param adds    Number of add values.
 * @param remove  Values with entries to remove.
 * @param removes Number of remove values.
 * @param clear   PK_TRUE to start from an empty list.
 * @param fmt     Output format: plain, JSON, CSV, or the entries only.
 * @return PK_OK on success, or a PK_E* error code on failure.
 */
PKInt pkcTrusted (PKClient *client, const PKChar *event,
                  const PKChar *const *add, PKSize adds,
                  const PKChar *const *remove, PKSize removes, PKBool clear,
                  PKCArgFmt fmt);

#endif
