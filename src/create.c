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

#include "create.h"
#include "api.h"
#include "list.h"
#include "logger.h"
#include "types.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PLInt
plCreate (const PLChar *base, const PLChar *bearer, const PLChar *name,
          PLArgFmt fmt)
{
  if (!base || !bearer || !name)
    {
      PL_ERROR ("Endpoint, bearer, and/or name are invalid");
      return PL_EARG;
    }

  PLEvent event;
  PLInt result = plApiEventCreate (&event, base, bearer, name);
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to create event");
      return result;
    }

  plPrintEventHeader (fmt);
  plPrintEventRow (event.name, event.id, event.from, event.to, event.owner,
                   event.favorite, event.pub, event.guests, event.media, fmt,
                   PL_TRUE);
  plPrintEventFooter (fmt);
  if (fmt == PL_FMTPLAIN)
    {
      PL_INFO ("");
      PL_INFO ("1 event(s)");
    }

  return PL_EOK;
}
