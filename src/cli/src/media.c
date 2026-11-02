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
#include "log.h"
#include "partoska.h"
#include "pktypes.h"
#include "plat.h"
#include "report.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define DATE_WIDTH (10)
#define TYPE_WIDTH (16)
#define ROW_MAX (512)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
pkcPrintMediaHeader (PKCArgFmt fmt)
{
  PKChar buf[ROW_MAX];
  switch (fmt)
    {
    case PKC_FMTCSV:
      PKC_INFO ("id,type,taken,uploaded,owner,favorite,favorites,approved");
      break;
    case PKC_FMTJSON:
      PKC_INFO ("[");
      break;
    case PKC_FMTONE:
      break;
    default:
      pkcStrFmt (buf, PK_CHARSMAX (buf),
                 "%-36s  %-*s  %-10s  %-10s  %3s %3s  %4s  %3s", "ID",
                 TYPE_WIDTH, "TYPE", "TAKEN", "UPLOADED", "OWN", "FAV", "FAVS",
                 "APR");
      buf[PK_CHARSMAX (buf)] = '\0';
      PKC_INFO ("%s", buf);
      break;
    }
}

static PKVoid
pkcPrintMediaRow (const PKChar *id, const PKChar *type, const PKChar *taken,
                  const PKChar *uploaded, PKBool owner, PKBool favorite,
                  PKInt favorites, PKBool approved, PKCArgFmt fmt, PKBool last)
{
  PKChar buf[ROW_MAX];

  switch (fmt)
    {
    case PKC_FMTCSV:
      pkcStrFmt (buf, PK_CHARSMAX (buf), "%s,%s,%s,%s,%s,%s,%d,%s", id, type,
                 taken, uploaded, owner ? "true" : "false",
                 favorite ? "true" : "false", favorites,
                 approved ? "true" : "false");
      buf[PK_CHARSMAX (buf)] = '\0';
      PKC_INFO ("%s", buf);
      break;

    case PKC_FMTONE:
      PKC_INFO ("%s", id);
      break;

    case PKC_FMTJSON:
      {
        const PKChar *comma = last ? "" : ",";
        pkcStrFmt (buf, PK_CHARSMAX (buf),
                   "  {\"id\":\"%s\",\"type\":\"%s\",\"taken\":\"%s\","
                   "\"uploaded\":\"%s\",\"owner\":%s,\"favorite\":%s,"
                   "\"favorites\":%d,\"approved\":%s}%s",
                   id, type, taken, uploaded, owner ? "true" : "false",
                   favorite ? "true" : "false", favorites,
                   approved ? "true" : "false", comma);
        buf[PK_CHARSMAX (buf)] = '\0';
        PKC_INFO ("%s", buf);
        break;
      }

    default:
      {
        PKChar ttype[TYPE_WIDTH + 1];
        pkcStrFmt (ttype, sizeof (ttype), "%s", type);

        PKChar ttaken[DATE_WIDTH + 1];
        pkcStrFmt (ttaken, sizeof (ttaken), "%s", taken);

        PKChar tuploaded[DATE_WIDTH + 1];
        pkcStrFmt (tuploaded, sizeof (tuploaded), "%s", uploaded);

        pkcStrFmt (buf, PK_CHARSMAX (buf),
                   "%-36s  %-*s  %-10s  %-10s  %3s %3s  %4d  %3s", id,
                   TYPE_WIDTH, ttype, ttaken, tuploaded, owner ? "yes" : "no",
                   favorite ? "yes" : "no", favorites,
                   approved ? "yes" : "no");
        buf[PK_CHARSMAX (buf)] = '\0';
        PKC_INFO ("%s", buf);
        break;
      }
    }
}

static PKVoid
pkcPrintMediaFooter (PKCArgFmt fmt)
{
  switch (fmt)
    {
    case PKC_FMTJSON:
      PKC_INFO ("]");
      break;
    default:
      break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKInt
pkcMediaList (PKClient *client, const PKChar *event, PKBool owner,
              PKBool favorite, PKCArgFmt fmt)
{
  if (!client || !event)
    {
      PKC_ERROR ("Client, and/or event are invalid");
      return PK_EARG;
    }

  PKMediaList list;
  if (pkMediaList (client, event, &list) != PK_OK)
    {
      pkcReport (client, "Failed to fetch media list");
      return pkError (client)->status;
    }

  PKSize filtered = 0;
  for (PKSize i = 0; i < list.count; ++i)
    {
      PKMedia *m = &list.media[i];
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

  pkcPrintMediaHeader (fmt);

  PKSize printed = 0;
  for (PKSize i = 0; i < list.count; ++i)
    {
      PKMedia *m = &list.media[i];
      if (owner && !m->owner)
        {
          continue;
        }
      if (favorite && !m->favorite)
        {
          continue;
        }
      pkcPrintMediaRow (m->id, m->type, m->taken, m->uploaded, m->owner,
                        m->favorite, m->favorites, m->approved, fmt,
                        printed == filtered - 1);
      ++printed;
    }

  pkcPrintMediaFooter (fmt);
  if (fmt == PKC_FMTPLAIN)
    {
      PKC_INFO ("");
      PKC_INFO ("%u media item(s)", (PKDword)printed);
    }

  pkMediaListFree (&list);

  return PK_OK;
}
