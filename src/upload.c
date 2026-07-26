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

#include "upload.h"
#include "api.h"
#include "fs.h"
#include "logger.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define plStricmp _stricmp
#else
#include <strings.h>
#define plStricmp strcasecmp
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PLInt
plUpload (const PLChar *base, const PLChar *bearer, const PLChar *event,
          const PLChar *path)
{
  if (!base || !bearer || !event || !path)
    {
      PL_ERROR ("Endpoint, bearer, event, and/or path are invalid");
      return PL_EARG;
    }

  const PLChar *ext = strrchr (path, '.');
  const PLChar *type;
  if (ext && (plStricmp (ext, ".jpg") == 0 || plStricmp (ext, ".jpeg") == 0))
    {
      type = "image/jpeg";
    }
  else if (ext && plStricmp (ext, ".mp4") == 0)
    {
      type = "video/mp4";
    }
  else if (ext && plStricmp (ext, ".mov") == 0)
    {
      type = "video/quicktime";
    }
  else
    {
      PL_ERROR ("Unsupported file type; expected .jpg, .jpeg, .mp4, or .mov");
      return PL_EARG;
    }

  PLFile *file = plFileOpen (path, "rb");
  if (!file)
    {
      PL_ERROR ("Failed to open file for reading: %s", path);
      return PL_EFS;
    }

  if (fseek (file, 0, SEEK_END) != 0)
    {
      PL_ERROR ("Failed to seek file: %s", path);
      plFileClose (file);
      return PL_EFS;
    }
  PLLong filesize = ftell (file);
  if (filesize <= 0 || fseek (file, 0, SEEK_SET) != 0)
    {
      PL_ERROR ("Failed to determine file size: %s", path);
      plFileClose (file);
      return PL_EFS;
    }

  PLChar session[64];
  PLInt chunksize = 0;
  PLInt total = 0;
  PLInt result
      = plApiMediaUploadStart (session, sizeof (session), &chunksize, &total,
                                base, bearer, event, type, filesize);
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to start upload session");
      plFileClose (file);
      return result;
    }

  if (chunksize <= 0 || total < 0)
    {
      PL_ERROR ("Invalid upload session parameters");
      plFileClose (file);
      plApiMediaUploadCancel (base, bearer, event, session);
      return PL_ENET;
    }

  PLByte *buffer = malloc ((PLSize)chunksize);
  if (!buffer)
    {
      PL_ERROR ("Out of memory");
      plFileClose (file);
      plApiMediaUploadCancel (base, bearer, event, session);
      return PL_EMEM;
    }

  for (PLInt index = 0; index < total; ++index)
    {
      PLSize len = fread (buffer, 1, (PLSize)chunksize, file);
      if (len == 0)
        {
          PL_ERROR ("Failed to read chunk %d from file: %s", index, path);
          result = PL_EFS;
          break;
        }

      result = plApiMediaUploadChunk (base, bearer, event, session, index,
                                       buffer, len);
      if (result != PL_EOK)
        {
          PL_ERROR ("Failed to upload chunk %d of %d", index + 1, total);
          break;
        }

      PL_DEBUG ("Uploaded chunk %d of %d", index + 1, total);
    }

  free (buffer);
  plFileClose (file);

  if (result != PL_EOK)
    {
      plApiMediaUploadCancel (base, bearer, event, session);
      return result;
    }

  PLChar id[64];
  result
      = plApiMediaUploadComplete (id, sizeof (id), base, bearer, event, session);
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to complete upload");
      plApiMediaUploadCancel (base, bearer, event, session);
      return result;
    }

  PL_INFO ("%s", id);

  return PL_EOK;
}
