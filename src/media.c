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

#include "media.h"
#include "api.h"
#include "logger.h"
#include "types.h"

#include <stdio.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define DATE_WIDTH (10)
#define TYPE_WIDTH (16)
#define ROW_MAX (512)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
plPrintMediaHeader (PLArgFmt fmt)
{
  PLChar buf[ROW_MAX];
  switch (fmt)
    {
    case PL_FMTCSV:
      PL_INFO ("id,type,taken,uploaded,owner,favorite,favorites");
      break;
    case PL_FMTJSON:
      PL_INFO ("[");
      break;
    case PL_FMTONE:
      break;
    default:
      snprintf (buf, PL_CHARSMAX (buf),
                "%-36s  %-*s  %-10s  %-10s  %3s %3s  %4s", "ID", TYPE_WIDTH,
                "TYPE", "TAKEN", "UPLOADED", "OWN", "FAV", "FAVS");
      buf[PL_CHARSMAX (buf)] = '\0';
      PL_INFO ("%s", buf);
      break;
    }
}

static void
plPrintMediaRow (const PLChar *id, const PLChar *type, const PLChar *taken,
                 const PLChar *uploaded, PLBool owner, PLBool favorite,
                 PLInt favorites, PLArgFmt fmt, PLBool last)
{
  PLChar buf[ROW_MAX];

  switch (fmt)
    {
    case PL_FMTCSV:
      snprintf (buf, PL_CHARSMAX (buf), "%s,%s,%s,%s,%s,%s,%d", id, type,
                taken, uploaded, owner ? "true" : "false",
                favorite ? "true" : "false", favorites);
      buf[PL_CHARSMAX (buf)] = '\0';
      PL_INFO ("%s", buf);
      break;

    case PL_FMTONE:
      PL_INFO ("%s", id);
      break;

    case PL_FMTJSON:
      {
        const PLChar *comma = last ? "" : ",";
        snprintf (buf, PL_CHARSMAX (buf),
                  "  {\"id\":\"%s\",\"type\":\"%s\",\"taken\":\"%s\","
                  "\"uploaded\":\"%s\",\"owner\":%s,\"favorite\":%s,"
                  "\"favorites\":%d}%s",
                  id, type, taken, uploaded, owner ? "true" : "false",
                  favorite ? "true" : "false", favorites, comma);
        buf[PL_CHARSMAX (buf)] = '\0';
        PL_INFO ("%s", buf);
        break;
      }

    default:
      {
        PLChar ttype[TYPE_WIDTH + 1];
        strncpy (ttype, type, TYPE_WIDTH);
        ttype[TYPE_WIDTH] = '\0';

        PLChar ttaken[DATE_WIDTH + 1];
        strncpy (ttaken, taken, DATE_WIDTH);
        ttaken[DATE_WIDTH] = '\0';

        PLChar tuploaded[DATE_WIDTH + 1];
        strncpy (tuploaded, uploaded, DATE_WIDTH);
        tuploaded[DATE_WIDTH] = '\0';

        snprintf (buf, PL_CHARSMAX (buf),
                  "%-36s  %-*s  %-10s  %-10s  %3s %3s  %4d", id, TYPE_WIDTH,
                  ttype, ttaken, tuploaded, owner ? "yes" : "no",
                  favorite ? "yes" : "no", favorites);
        buf[PL_CHARSMAX (buf)] = '\0';
        PL_INFO ("%s", buf);
        break;
      }
    }
}

static void
plPrintMediaFooter (PLArgFmt fmt)
{
  switch (fmt)
    {
    case PL_FMTJSON:
      PL_INFO ("]");
      break;
    default:
      break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PLInt
plMediaList (const PLChar *base, const PLChar *bearer, const PLChar *event,
             PLBool owner, PLBool favorite, PLArgFmt fmt)
{
  if (!base || !bearer || !event)
    {
      PL_ERROR ("Endpoint, bearer, and/or event are invalid");
      return PL_EARG;
    }

  PLMediaList *list = plApiMediaList (base, bearer, event);
  if (!list)
    {
      PL_ERROR ("Failed to fetch media list");
      return PL_ENET;
    }

  PLSize filtered = 0;
  for (PLSize i = 0; i < list->count; ++i)
    {
      PLMedia *m = &list->media[i];
      if (owner && !m->owner)
        {
          continue;
        }
      if (favorite && !m->favorite)
        {
          continue;
        }
      ++filtered;
    }

  plPrintMediaHeader (fmt);

  PLSize printed = 0;
  for (PLSize i = 0; i < list->count; ++i)
    {
      PLMedia *m = &list->media[i];
      if (owner && !m->owner)
        {
          continue;
        }
      if (favorite && !m->favorite)
        {
          continue;
        }
      plPrintMediaRow (m->id, m->type, m->taken, m->uploaded, m->owner,
                       m->favorite, m->favorites, fmt,
                       printed == filtered - 1);
      ++printed;
    }

  plPrintMediaFooter (fmt);
  if (fmt == PL_FMTPLAIN)
    {
      PL_INFO ("");
      PL_INFO ("%u media item(s)", (PLDword)printed);
    }

  plFreeMediaList (list);

  return PL_EOK;
}
