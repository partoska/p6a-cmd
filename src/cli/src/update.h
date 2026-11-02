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

#ifndef PKC_UPDATE_H
#define PKC_UPDATE_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "partoska.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Partially updates an event on Partoska.
 *
 * Only non-NULL string fields and non-(-1) integer fields are sent.
 *
 * @param client   Logged-in client.
 * @param id     UUID of the event to update.
 * @param name   New display name, or NULL to leave unchanged.
 * @param from   New start date string, or NULL to leave unchanged.
 * @param to     New end date string, or NULL to leave unchanged.
 * @param pub    Public flag: 1 to set public, 0 to set private, -1 to leave
 *               unchanged.
 * @param fav    Favorite flag: 1 to favorite, 0 to unfavorite, -1 to leave
 *               unchanged.
 * @param mod    Moderated flag: 1 to enable moderation, 0 to disable, -1 to
 *               leave unchanged.
 * @return PK_OK on success, or a PK_E* error code on failure.
 */
PKInt pkcUpdate (PKClient *client, const PKChar *id, const PKChar *name,
                 const PKChar *from, const PKChar *to, PKInt pub, PKInt fav,
                 PKInt mod);

#endif
