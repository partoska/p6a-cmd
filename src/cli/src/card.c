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

#include "card.h"
#include "log.h"
#include "partoska.h"
#include "pktypes.h"
#include "plat.h"
#include "report.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define OUTPUT_MAX (1024)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKInt
pkcCard (PKClient *client, const PKChar *event, const PKChar *output,
         const PKChar *design, const PKChar *locale, const PKChar *layout,
         const PKChar *paper, PKBool jpg, PKBool nobg)
{
  if (!client || !event || !design)
    {
      PKC_ERROR ("Client, event, and/or design are invalid");
      return PK_EARG;
    }

  PKChar path[OUTPUT_MAX];
  if (output != NULL)
    {
      pkcStrFmt (path, sizeof (path), "%s", output);
    }
  else
    {
      pkcStrFmt (path, PK_CHARSMAX (path), jpg ? "%s-card.jpg" : "%s-card.pdf",
                 event);
      path[PK_CHARSMAX (path)] = '\0';
    }

  PKInt result = pkEventCard (client, event, path, design, locale, layout,
                              paper, jpg, nobg);
  if (result != PK_OK)
    {
      pkcReport (client, "Failed to download share card");
      return result;
    }

  return PK_OK;
}
