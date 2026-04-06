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
#include "api.h"
#include "logger.h"
#include "types.h"

#include <stdio.h>
#include <string.h>

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

static PLSize
plExtraBytes (const PLChar *s)
{
  PLSize extra = 0;
  while (*s)
    {
      if (((PLByte)*s & 0xC0) == 0x80)
        {
          extra++;
        }
      s++;
    }

  return extra;
}

/* Escape a string for embedding in a JSON value (escapes " and \). */
static void
plJsonEsc (PLChar *dst, PLSize size, const PLChar *src)
{
  PLSize j = 0;
  for (PLSize i = 0; src[i] && j + 2 < size; ++i)
    {
      if ((src[i] == '"' || src[i] == '\\') && j + 3 < size)
        {
          dst[j++] = '\\';
        }
      dst[j++] = src[i];
    }
  dst[j] = '\0';
}

/* Quote a string for a CSV field (wraps in "..." and escapes " as ""). */
static void
plCsvQuote (PLChar *dst, PLSize size, const PLChar *src)
{
  PLSize j = 0;
  if (j + 1 < size)
    {
      dst[j++] = '"';
    }
  for (PLSize i = 0; src[i] && j + 2 < size; ++i)
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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
plPrintEventHeader (PLArgFmt fmt)
{
  PLChar buf[ROW_MAX];
  switch (fmt)
    {
    case PL_FMTCSV:
      PL_INFO ("id,name,from,to,owner,favorite,public,guests,media");
      break;
    case PL_FMTJSON:
      PL_INFO ("[");
      break;
    case PL_FMTONE:
      break;
    default:
      snprintf (buf, PL_CHARSMAX (buf),
                "%-*s  %-36s  %-10s  %-10s  %3s %3s %3s  %6s  %5s", NAME_WIDTH,
                "NAME", "ID", "FROM", "TO", "OWN", "FAV", "PUB", "GUESTS",
                "MEDIA");
      buf[PL_CHARSMAX (buf)] = '\0';
      PL_INFO ("%s", buf);
      break;
    }
}

void
plPrintEventRow (const PLChar *name, const PLChar *id, const PLChar *from,
                 const PLChar *to, PLBool owner, PLBool favorite, PLBool pub,
                 PLInt guests, PLInt media, PLArgFmt fmt, PLBool last)
{
  PLChar buf[JSON_MAX];

  switch (fmt)
    {
    case PL_FMTCSV:
      {
        PLChar qname[ROW_MAX];
        PLChar qfrom[ROW_MAX];
        PLChar qto[ROW_MAX];
        plCsvQuote (qname, sizeof (qname), name);
        plCsvQuote (qfrom, sizeof (qfrom), from);
        plCsvQuote (qto, sizeof (qto), to);
        snprintf (buf, PL_CHARSMAX (buf), "%s,%s,%s,%s,%s,%s,%s,%d,%d", id,
                  qname, qfrom, qto, owner ? "true" : "false",
                  favorite ? "true" : "false", pub ? "true" : "false", guests,
                  media);
        buf[PL_CHARSMAX (buf)] = '\0';
        PL_INFO ("%s", buf);
        break;
      }
    case PL_FMTONE:
      PL_INFO ("%s", id);
      break;
    case PL_FMTJSON:
      {
        PLChar ename[NAME_ESC_MAX];
        plJsonEsc (ename, sizeof (ename), name);
        const PLChar *comma = last ? "" : ",";
        snprintf (buf, PL_CHARSMAX (buf),
                  "  {\"id\":\"%s\",\"name\":\"%s\",\"from\":\"%s\","
                  "\"to\":\"%s\",\"owner\":%s,\"favorite\":%s,"
                  "\"public\":%s,\"guests\":%d,\"media\":%d}%s",
                  id, ename, from, to, owner ? "true" : "false",
                  favorite ? "true" : "false", pub ? "true" : "false", guests,
                  media, comma);
        buf[PL_CHARSMAX (buf)] = '\0';
        PL_INFO ("%s", buf);
        break;
      }
    default:
      {
        PLChar tname[NAME_WIDTH + 1];
        strncpy (tname, name, NAME_WIDTH);
        tname[NAME_WIDTH] = '\0';

        PLChar tfrom[DATE_WIDTH + 1];
        strncpy (tfrom, from, DATE_WIDTH);
        tfrom[DATE_WIDTH] = '\0';

        PLChar tto[DATE_WIDTH + 1];
        strncpy (tto, to, DATE_WIDTH);
        tto[DATE_WIDTH] = '\0';

        PLInt width = NAME_WIDTH + (PLInt)plExtraBytes (tname);
        snprintf (buf, PL_CHARSMAX (buf),
                  "%-*s  %-36s  %-10s  %-10s  %3s %3s %3s  %6d  %5d", width,
                  tname, id, tfrom, tto, owner ? "yes" : "no",
                  favorite ? "yes" : "no", pub ? "yes" : "no", guests, media);
        buf[PL_CHARSMAX (buf)] = '\0';
        PL_INFO ("%s", buf);
        break;
      }
    }
}

void
plPrintEventFooter (PLArgFmt fmt)
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

PLInt
plList (const PLChar *base, const PLChar *bearer, const PLChar *query,
        PLBool owner, PLBool favorite, PLArgFmt fmt)
{
  if (!base || !bearer)
    {
      PL_ERROR ("Endpoint and/or bearer are invalid");
      return PL_EARG;
    }

  PLEventList *list = plApiEventList (base, bearer, query);
  if (!list)
    {
      PL_ERROR ("Failed to fetch event list");
      return PL_ENET;
    }

  /* Collect filtered events for JSON (need total count for commas). */
  PLSize filtered = 0;
  for (PLSize i = 0; i < list->count; ++i)
    {
      PLEvent *e = &list->events[i];
      if (owner && !e->owner)
        {
          continue;
        }

      if (favorite && !e->favorite)
        {
          continue;
        }

      ++filtered;
    }

  plPrintEventHeader (fmt);

  PLSize printed = 0;
  for (PLSize i = 0; i < list->count; ++i)
    {
      PLEvent *e = &list->events[i];

      if (owner && !e->owner)
        {
          continue;
        }
      if (favorite && !e->favorite)
        {
          continue;
        }

      plPrintEventRow (e->name, e->id, e->from, e->to, e->owner, e->favorite,
                       e->pub, e->guests, e->media, fmt,
                       printed == filtered - 1);

      ++printed;
    }

  plPrintEventFooter (fmt);
  if (fmt == PL_FMTPLAIN)
    {
      PL_INFO ("");
      PL_INFO ("%u event(s)", (PLDword)printed);
    }

  plFreeEventList (list);

  return PL_EOK;
}
