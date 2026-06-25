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
#include "logger.h"
#include "types.h"
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
  if (!ext
      || (plStricmp (ext, ".jpg") != 0 && plStricmp (ext, ".jpeg") != 0
          && plStricmp (ext, ".mp4") != 0 && plStricmp (ext, ".mov") != 0))
    {
      PL_ERROR ("Unsupported file type; expected .jpg, .jpeg, .mp4, or .mov");
      return PL_EARG;
    }

  PLChar id[64];
  PLInt result = plApiMediaUpload (id, sizeof (id), base, bearer, event, path);
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to upload media");
      return result;
    }

  PL_INFO ("%s", id);

  return PL_EOK;
}
