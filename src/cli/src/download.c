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
#include "log.h"
#include "partoska.h"
#include "pktypes.h"
#include "plat.h"
#include "report.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define MAX_PATH (1024)
#define MAX_FILENAME (64)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKInt
pkcDownloadOne (PKClient *client, const PKChar *event, const PKMedia *media,
                const PKChar *dir)
{
  const PKChar *pre = "";
  const PKChar *ext = "";
  if (pkcStrFindStr (media->type, "image/jpeg"))
    {
      pre = "IMG";
      ext = ".jpg";
    }
  else if (pkcStrFindStr (media->type, "video/mp4"))
    {
      pre = "MOV";
      ext = ".mp4";
    }
  else if (pkcStrFindStr (media->type, "video/quicktime"))
    {
      pre = "MOV";
      ext = ".mov";
    }
  else
    {
      PKC_ERROR ("Unsupported media type: %s", media->type);
      return PK_EARG;
    }

  PKChar candidate[MAX_FILENAME];
  PKChar path[MAX_PATH];
  PKBool found = PK_FALSE;
  for (PKInt n = 1; n < 10000; ++n)
    {
      pkcStrFmt (candidate, PK_CHARSMAX (candidate), "%s_%04d%s", pre, n, ext);
      candidate[PK_CHARSMAX (candidate)] = '\0';
      pkcStrFmt (path, PK_CHARSMAX (path), "%s/%s", dir, candidate);
      path[PK_CHARSMAX (path)] = '\0';
      if (!pkcIsFile (path))
        {
          found = PK_TRUE;
          break;
        }
    }

  if (!found)
    {
      PKC_ERROR ("Could not find a free filename for: %s", media->id);
      return PK_EFS;
    }

  PKStatus status = pkMediaDownload (client, event, media->id, path);
  if (status != PK_OK)
    {
      pkcReport (client, "Failed to download media");
    }

  return status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKInt
pkcDownloadSingle (PKClient *client, const PKChar *event, const PKChar *media,
                   const PKChar *output)
{
  if (!client || !event || !media || !output)
    {
      PKC_ERROR ("Client, event, media, and/or output are invalid");
      return PK_EARG;
    }

  PKStatus status = pkMediaDownload (client, event, media, output);
  if (status != PK_OK)
    {
      pkcReport (client, "Failed to download media");
    }

  return status;
}

PKInt
pkcDownloadAll (PKClient *client, const PKChar *event, const PKChar *target,
                PKBool owner, PKBool favorite)
{
  if (!client || !event || !target)
    {
      PKC_ERROR ("Client, event, and/or target are invalid");
      return PK_EARG;
    }

  if (!pkcIsDir (target))
    {
      PKInt r = pkcCreateDir (target);
      if (r < 0)
        {
          PKC_ERROR ("Failed to create target directory: %s", target);
          return PK_EFS;
        }
    }

  PKMediaList list;
  if (pkMediaList (client, event, &list) != PK_OK)
    {
      pkcReport (client, "Failed to fetch media list");
      return pkError (client)->status;
    }

  PKInt result = PK_OK;
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

      result = pkcDownloadOne (client, event, m, target);
      if (result != PK_OK)
        {
          break;
        }
    }

  pkMediaListFree (&list);

  return result;
}
