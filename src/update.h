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

#ifndef UPDATE_H
#define UPDATE_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "types.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Partially updates an event on Partoska.
 *
 * Only non-NULL string fields and non-(-1) integer fields are sent.
 *
 * @param base   API base URL.
 * @param bearer OAuth bearer token.
 * @param id     UUID of the event to update.
 * @param name   New display name, or NULL to leave unchanged.
 * @param from   New start date string, or NULL to leave unchanged.
 * @param to     New end date string, or NULL to leave unchanged.
 * @param pub    Public flag: 1 to set public, 0 to set private, -1 to leave
 *               unchanged.
 * @param fav    Favorite flag: 1 to favorite, 0 to unfavorite, -1 to leave
 *               unchanged.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plUpdate (const PLChar *base, const PLChar *bearer, const PLChar *id,
                const PLChar *name, const PLChar *from, const PLChar *to,
                PLInt pub, PLInt fav);

#endif
