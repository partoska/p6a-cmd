
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

#include "config.h"
#include "fs.h"
#include "logger.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static const PLChar P6A_DIR[] = "/.p6a";

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PLInt
plPrepareWorkdir (PLChar *workdir, PLSize size)
{
  if (workdir != NULL && strnlen (workdir, size) > 0)
    {
      PLBool result = plIsDir (workdir);
      if (!result)
        {
          PL_DEBUG ("Workdir in arg is not directory");
          return PL_EFS;
        }

      return PL_EOK;
    }

  // No path using '-D' arg specified. Try P6A_HOME env variable first.
  const PLChar *env = getenv ("P6A_HOME");
  if (env != NULL)
    {
      PLSize envlen = strnlen (env, size);
      if (envlen >= size)
        {
          return PL_EARG;
        }
      strncat (workdir, env, size - 1);
      PLBool result = plIsDir (workdir);
      if (!result)
        {
          PL_DEBUG ("Workdir in p6a env is not directory");
          return PL_EFS;
        }

      return PL_EOK;
    }

  // Fallback to HOME directory of the current user.
  env = getenv ("HOME");
#ifdef _WIN32
  if (env == NULL)
    {
      env = getenv ("USERPROFILE");
    }
#endif
  if (env != NULL)
    {
      PLSize envlen = strnlen (env, size);
      if (envlen >= size - sizeof (P6A_DIR))
        {
          return PL_EARG;
        }
      strncpy (workdir, env, size - 1);
      workdir[size - 1] = '\0';
      strncat (workdir, P6A_DIR, size - envlen - 1);
      workdir[size - 1] = '\0';
      PLInt result = plCreateDir (workdir);
      if (result < 0)
        {
          PL_DEBUG ("Workdir in home env cannot be created");
          return result;
        }
      else if (result == 0)
        {
          PL_DEBUG ("Created workdir: %s", workdir);
        }
      else
        {
          PL_DEBUG ("Using workdir: %s", workdir);
        }

      return PL_EOK;
    }

  // No HOME directory found, use the current working directory.
  if (getcwd (workdir, size) == NULL)
    {
      PL_DEBUG ("Could not get current workdir");
      return PL_EFS;
    }

  return PL_EOK;
}

void
plGetIniPath (PLChar *path, PLSize size, const PLChar *workdir)
{
  strncpy (path, workdir, size - 1);
  path[size - 1] = '\0';
  strncat (path, PL_CONFIG_INI, size - 1);
  path[size - 1] = '\0';
}
