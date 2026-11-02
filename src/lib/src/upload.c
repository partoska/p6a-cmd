/*
 * Partoska Library - Client library for Partoska.com media sharing service.
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

#include "client.h"
#include "plat.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ASCII case-insensitive comparison, independent of the locale. */
static PKBool
pkExtIs (const PKChar *ext, const PKChar *expected)
{
  for (; *ext && *expected; ++ext, ++expected)
    {
      PKChar c
          = *ext >= 'A' && *ext <= 'Z' ? (PKChar)(*ext - 'A' + 'a') : *ext;
      if (c != *expected)
        {
          return PK_FALSE;
        }
    }

  return *ext == *expected;
}

/* The media type of a file, from its extension. */
static const PKChar *
pkMediaType (const PKChar *path)
{
  const PKChar *ext = pkStrFindLast (path, '.');
  if (!ext)
    {
      return NULL;
    }
  if (pkExtIs (ext, ".jpg") || pkExtIs (ext, ".jpeg"))
    {
      return "image/jpeg";
    }
  if (pkExtIs (ext, ".mp4"))
    {
      return "video/mp4";
    }
  if (pkExtIs (ext, ".mov"))
    {
      return "video/quicktime";
    }

  return NULL;
}

/* Sends every chunk of an open file. */
static PKStatus
pkUploadChunks (PKClient *client, const PKChar *event, const PKChar *session,
                PKFile *file, const PKChar *path, PKInt chunk, PKInt total)
{
  PKByte *buffer = pkMemZalloc ((PKSize)chunk);
  if (!buffer)
    {
      return pkFailMem (client);
    }

  PKStatus status = PK_OK;
  for (PKInt index = 0; index < total && status == PK_OK; ++index)
    {
      PKSize length = pkFileRead (file, buffer, (PKSize)chunk);
      if (length == 0)
        {
          status
              = pkFail (client, PK_EFS, pkSysError (), "Cannot read %s", path);
          break;
        }

      status = pkUploadChunk (client, event, session, index, buffer, length);
      if (status != PK_OK)
        {
          pkErrorContext (client, "Chunk %d of %d", index + 1, total);
        }
      else
        {
          pkLog (client, PK_LOG_DEBUG, "Uploaded chunk %d of %d", index + 1,
                 total);
        }
    }

  pkMemFree (buffer);

  return status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKStatus
pkMediaUpload (PKClient *client, const PKChar *event, const PKChar *path,
               PKChar *id, PKSize size)
{
  if (!event || !path || !id || !size)
    {
      return pkFail (client, PK_EARG, 0, "No event, file or id buffer");
    }

  const PKChar *type = pkMediaType (path);
  if (!type)
    {
      return pkFail (client, PK_EARG, 0,
                     "Unsupported file type; expected .jpg, .jpeg, .mp4, or "
                     ".mov");
    }

  PKFile *file = pkFileOpen (path, PK_FALSE);
  if (!file)
    {
      return pkFail (client, PK_EFS, pkSysError (), "Cannot read %s: %s", path,
                     pkSysErrorText (pkSysError ()));
    }

  PKLong length = 0;
  if (!pkFileSize (file, &length) || length <= 0)
    {
      pkFileClose (file);
      return pkFail (client, PK_EFS, pkSysError (),
                     "Cannot determine the size of %s", path);
    }

  PKChar session[64];
  PKInt chunk = 0;
  PKInt total = 0;
  PKStatus status = pkUploadStart (client, event, type, length, session,
                                   sizeof (session), &chunk, &total);
  if (status != PK_OK)
    {
      pkFileClose (file);
      return pkErrorContext (client, "Cannot start the upload");
    }

  status = pkUploadChunks (client, event, session, file, path, chunk, total);
  pkFileClose (file);
  if (status == PK_OK)
    {
      status = pkUploadComplete (client, event, session, id, size);
      if (status != PK_OK)
        {
          pkErrorContext (client, "Cannot complete the upload");
        }
    }

  if (status != PK_OK)
    {
      pkUploadCancel (client, event, session);
    }

  return status;
}
