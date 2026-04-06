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

#include "qr.h"
#include "api.h"
#include "logger.h"
#include "types.h"

#include <stdio.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define OUTPUT_MAX (1024)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PLInt
plQr (const PLChar *base, const PLChar *bearer, const PLChar *event,
      const PLChar *output, PLBool svg)
{
  if (!base || !bearer || !event)
    {
      PL_ERROR ("Endpoint, bearer, and/or event are invalid");
      return PL_EARG;
    }

  PLChar path[OUTPUT_MAX];
  if (output != NULL)
    {
      strncpy (path, output, PL_CHARSMAX (path));
      path[PL_CHARSMAX (path)] = '\0';
    }
  else
    {
      snprintf (path, PL_CHARSMAX (path), svg ? "%s-qr.svg" : "%s-qr.png",
                event);
      path[PL_CHARSMAX (path)] = '\0';
    }

  PLInt result = plApiQrFetch (base, bearer, event, path, svg);
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to download QR code");
      return result;
    }

  return PL_EOK;
}
