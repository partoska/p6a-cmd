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

#include "download.h"
#include "api.h"
#include "fs.h"
#include "logger.h"
#include "types.h"

#include <stdio.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define MAX_PATH (1024)
#define MAX_FILENAME (64)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PLInt
plDownloadOne (const PLChar *base, const PLChar *bearer, const PLChar *event,
               const PLMedia *media, const PLChar *dir)
{
  const PLChar *pre = "";
  const PLChar *ext = "";
  if (strstr (media->type, "image/jpeg"))
    {
      pre = "IMG";
      ext = ".jpg";
    }
  else if (strstr (media->type, "video/mp4"))
    {
      pre = "MOV";
      ext = ".mp4";
    }
  else if (strstr (media->type, "video/quicktime"))
    {
      pre = "MOV";
      ext = ".mov";
    }
  else
    {
      PL_ERROR ("Unsupported media type: %s", media->type);
      return PL_EARG;
    }

  PLChar candidate[MAX_FILENAME];
  PLChar path[MAX_PATH];
  PLBool found = PL_FALSE;
  for (PLInt n = 1; n < 10000; ++n)
    {
      snprintf (candidate, PL_CHARSMAX (candidate), "%s_%04d%s", pre, n, ext);
      candidate[PL_CHARSMAX (candidate)] = '\0';
      snprintf (path, PL_CHARSMAX (path), "%s/%s", dir, candidate);
      path[PL_CHARSMAX (path)] = '\0';
      if (!plIsFile (path))
        {
          found = PL_TRUE;
          break;
        }
    }

  if (!found)
    {
      PL_ERROR ("Could not find a free filename for: %s", media->id);
      return PL_EFS;
    }

  return plApiMediaFetch (base, bearer, event, media->id, path);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PLInt
plDownloadSingle (const PLChar *base, const PLChar *bearer,
                  const PLChar *event, const PLChar *media,
                  const PLChar *output)
{
  if (!base || !bearer || !event || !media || !output)
    {
      PL_ERROR ("Endpoint, bearer, event, media, and/or output are invalid");
      return PL_EARG;
    }

  return plApiMediaFetch (base, bearer, event, media, output);
}

PLInt
plDownloadAll (const PLChar *base, const PLChar *bearer, const PLChar *event,
               const PLChar *target, PLBool owner, PLBool favorite)
{
  if (!base || !bearer || !event || !target)
    {
      PL_ERROR ("Endpoint, bearer, event, and/or target are invalid");
      return PL_EARG;
    }

  if (!plIsDir (target))
    {
      PLInt r = plCreateDir (target);
      if (r < 0)
        {
          PL_ERROR ("Failed to create target directory: %s", target);
          return PL_EFS;
        }
    }

  PLMediaList *list = plApiMediaList (base, bearer, event);
  if (!list)
    {
      PL_ERROR ("Failed to fetch media list");
      return PL_ENET;
    }

  PLInt result = PL_EOK;
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

      result = plDownloadOne (base, bearer, event, m, target);
      if (result != PL_EOK)
        {
          break;
        }
    }

  plFreeMediaList (list);

  return result;
}
