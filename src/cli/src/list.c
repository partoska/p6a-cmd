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

#include "list.h"
#include "log.h"
#include "partoska.h"
#include "pktypes.h"
#include "plat.h"
#include "report.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define DATE_WIDTH (10)
#define NAME_WIDTH (32)
#define ROW_MAX (256)
#define JSON_MAX (1024)
#define NAME_ESC_MAX (512)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKSize
pkcExtraBytes (const PKChar *s)
{
  PKSize extra = 0;
  while (*s)
    {
      if (((PKByte)*s & 0xC0) == 0x80)
        {
          extra++;
        }
      s++;
    }

  return extra;
}

PKVoid
pkcJsonEsc (PKChar *dst, PKSize size, const PKChar *src)
{
  PKSize j = 0;
  for (PKSize i = 0; src[i] && j + 2 < size; ++i)
    {
      if ((src[i] == '"' || src[i] == '\\') && j + 3 < size)
        {
          dst[j++] = '\\';
        }
      dst[j++] = src[i];
    }
  dst[j] = '\0';
}

PKVoid
pkcCsvQuote (PKChar *dst, PKSize size, const PKChar *src)
{
  PKSize j = 0;
  if (j + 1 < size)
    {
      dst[j++] = '"';
    }
  for (PKSize i = 0; src[i] && j + 2 < size; ++i)
    {
      if (src[i] == '"' && j + 3 < size)
        {
          dst[j++] = '"';
        }
      dst[j++] = src[i];
    }
  if (j + 1 < size)
    {
      dst[j++] = '"';
    }
  dst[j] = '\0';
}

/* Tells whether an event passes the filters of list. */
static PKBool
pkcListKeeps (const PKEvent *event, PKBool owner, PKBool favorite,
              PKBool contributor)
{
  return (!owner || event->owner) && (!favorite || event->favorite)
         && (!contributor || event->contributor);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKVoid
pkcPrintEventHeader (PKCArgFmt fmt)
{
  PKChar buf[ROW_MAX];
  switch (fmt)
    {
    case PKC_FMTCSV:
      PKC_INFO ("id,name,from,to,owner,favorite,public,moderated,guests,media,"
                "contributor");
      break;
    case PKC_FMTJSON:
      PKC_INFO ("[");
      break;
    case PKC_FMTONE:
      break;
    default:
      pkcStrFmt (buf, PK_CHARSMAX (buf),
                 "%-*s  %-36s  %-10s  %-10s  %3s %3s %3s %3s %3s  %6s  %5s",
                 NAME_WIDTH, "NAME", "ID", "FROM", "TO", "OWN", "CON", "FAV",
                 "PUB", "MOD", "GUESTS", "MEDIA");
      buf[PK_CHARSMAX (buf)] = '\0';
      PKC_INFO ("%s", buf);
      break;
    }
}

PKVoid
pkcPrintEventRow (const PKEvent *event, PKCArgFmt fmt, PKBool last)
{
  const PKChar *name = event->name;
  const PKChar *id = event->id;
  const PKChar *from = event->from;
  const PKChar *to = event->to;
  const PKChar *owner = event->owner ? "true" : "false";
  const PKChar *contributor = event->contributor ? "true" : "false";
  const PKChar *favorite = event->favorite ? "true" : "false";
  const PKChar *pub = event->pub ? "true" : "false";
  const PKChar *moderated = event->moderated ? "true" : "false";
  PKInt guests = event->guests;
  PKInt media = event->media;
  PKChar buf[JSON_MAX];

  switch (fmt)
    {
    case PKC_FMTCSV:
      {
        PKChar qname[ROW_MAX];
        PKChar qfrom[ROW_MAX];
        PKChar qto[ROW_MAX];
        pkcCsvQuote (qname, sizeof (qname), name);
        pkcCsvQuote (qfrom, sizeof (qfrom), from);
        pkcCsvQuote (qto, sizeof (qto), to);
        pkcStrFmt (buf, PK_CHARSMAX (buf), "%s,%s,%s,%s,%s,%s,%s,%s,%d,%d,%s",
                   id, qname, qfrom, qto, owner, favorite, pub, moderated,
                   guests, media, contributor);
        buf[PK_CHARSMAX (buf)] = '\0';
        PKC_INFO ("%s", buf);
        break;
      }
    case PKC_FMTONE:
      PKC_INFO ("%s", id);
      break;
    case PKC_FMTJSON:
      {
        PKChar ename[NAME_ESC_MAX];
        pkcJsonEsc (ename, sizeof (ename), name);
        const PKChar *comma = last ? "" : ",";
        pkcStrFmt (buf, PK_CHARSMAX (buf),
                   "  {\"id\":\"%s\",\"name\":\"%s\",\"from\":\"%s\","
                   "\"to\":\"%s\",\"owner\":%s,\"contributor\":%s,"
                   "\"favorite\":%s,\"public\":%s,\"moderated\":%s,"
                   "\"guests\":%d,\"media\":%d}%s",
                   id, ename, from, to, owner, contributor, favorite, pub,
                   moderated, guests, media, comma);
        buf[PK_CHARSMAX (buf)] = '\0';
        PKC_INFO ("%s", buf);
        break;
      }
    default:
      {
        PKChar tname[NAME_WIDTH + 1];
        pkcStrFmt (tname, sizeof (tname), "%s", name);

        PKChar tfrom[DATE_WIDTH + 1];
        pkcStrFmt (tfrom, sizeof (tfrom), "%s", from);

        PKChar tto[DATE_WIDTH + 1];
        pkcStrFmt (tto, sizeof (tto), "%s", to);

        PKInt width = NAME_WIDTH + (PKInt)pkcExtraBytes (tname);
        pkcStrFmt (buf, PK_CHARSMAX (buf),
                   "%-*s  %-36s  %-10s  %-10s  %3s %3s %3s %3s %3s  %6d  %5d",
                   width, tname, id, tfrom, tto, event->owner ? "yes" : "no",
                   event->contributor ? "yes" : "no",
                   event->favorite ? "yes" : "no", event->pub ? "yes" : "no",
                   event->moderated ? "yes" : "no", guests, media);
        buf[PK_CHARSMAX (buf)] = '\0';
        PKC_INFO ("%s", buf);
        break;
      }
    }
}

PKVoid
pkcPrintEventFooter (PKCArgFmt fmt)
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

PKInt
pkcList (PKClient *client, const PKChar *query, PKBool owner, PKBool favorite,
         PKBool contributor, PKCArgFmt fmt)
{
  if (!client)
    {
      PKC_ERROR ("Client is invalid");
      return PK_EARG;
    }

  PKEventList list;
  if (pkEventList (client, query, &list) != PK_OK)
    {
      pkcReport (client, "Failed to fetch event list");
      return pkError (client)->status;
    }

  /* Collect filtered events for JSON (need total count for commas). */
  PKSize filtered = 0;
  for (PKSize i = 0; i < list.count; ++i)
    {
      if (pkcListKeeps (&list.events[i], owner, favorite, contributor))
        {
          ++filtered;
        }
    }

  pkcPrintEventHeader (fmt);

  PKSize printed = 0;
  for (PKSize i = 0; i < list.count; ++i)
    {
      PKEvent *e = &list.events[i];
      if (!pkcListKeeps (e, owner, favorite, contributor))
        {
          continue;
        }

      pkcPrintEventRow (e, fmt, printed == filtered - 1);

      ++printed;
    }

  pkcPrintEventFooter (fmt);
  if (fmt == PKC_FMTPLAIN)
    {
      PKC_INFO ("");
      PKC_INFO ("%u event(s)", (PKDword)printed);
    }

  pkEventListFree (&list);

  return PK_OK;
}
