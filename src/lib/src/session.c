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

static const PKChar P6A_DIR[] = "/.p6a";

/* Copies a path, refusing to truncate it. */
static PKStatus
pkPathCopy (PKClient *client, PKChar *out, PKSize size, const PKChar *a,
            const PKChar *b)
{
  PKInt length = pkStrFmt (out, size, "%s%s", a, b);
  if (length < 0 || (PKSize)length >= size)
    {
      return pkFail (client, PK_EARG, 0, "Path too long: %s%s", a, b);
    }

  return PK_OK;
}

/* Requires an existing directory. */
static PKStatus
pkExistingDir (PKClient *client, const PKChar *path, const PKChar *origin)
{
  if (!pkIsDir (path))
    {
      return pkFail (client, PK_EFS, 0, "%s is not a directory: %s", origin,
                     path);
    }

  return PK_OK;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Internal
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKStatus
pkWorkdir (PKClient *client, const PKChar *dir, PKChar *out, PKSize size)
{
  // A directory given by the caller has to exist.
  if (dir && *dir)
    {
      PKStatus status = pkPathCopy (client, out, size, dir, "");
      return status != PK_OK
                 ? status
                 : pkExistingDir (client, out, "Working directory");
    }

  // Then the P6A_HOME environment variable, which has to exist too.
  const PKChar *env = pkEnvGet ("P6A_HOME");
  if (env)
    {
      PKStatus status = pkPathCopy (client, out, size, env, "");
      return status != PK_OK ? status
                             : pkExistingDir (client, out, "P6A_HOME");
    }

  // Then ~/.p6a, created when missing.
  env = pkEnvGet ("HOME");
#ifdef _WIN32
  if (!env)
    {
      env = pkEnvGet ("USERPROFILE");
    }
#endif
  if (env)
    {
      PKStatus status = pkPathCopy (client, out, size, env, P6A_DIR);
      if (status != PK_OK)
        {
          return status;
        }

      PKInt created = pkCreateDir (out);
      if (created < 0)
        {
          return pkFail (client, PK_EFS, pkSysError (), "Cannot create %s: %s",
                         out, pkSysErrorText (pkSysError ()));
        }
      pkLog (client, PK_LOG_DEBUG, "%s workdir: %s",
             created == 0 ? "Created" : "Using", out);

      return PK_OK;
    }

  // Without a home, the current directory.
  if (!pkCwd (out, size))
    {
      return pkFail (client, PK_EFS, pkSysError (),
                     "Cannot get the current directory: %s",
                     pkSysErrorText (pkSysError ()));
    }

  return PK_OK;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKStatus
pkOpen (PKClient *client, const PKChar *dir)
{
  PKChar workdir[PK_PATH_MAX];
  PKChar ini[PK_PATH_MAX];
  PKStatus status = pkWorkdir (client, dir, workdir, sizeof (workdir));
  if (status == PK_OK)
    {
      status = pkPathCopy (client, ini, sizeof (ini), workdir, PK_CONFIG_INI);
    }
  if (status != PK_OK)
    {
      return status;
    }

  // A missing configuration means the defaults, not logged in.
  PKCfg *cfg = pkCfgInit ();
  if (!cfg)
    {
      return pkFailMem (client);
    }
  if (pkIsFile (ini))
    {
      status = pkCfgLoad (client, cfg, ini);
      if (status != PK_OK)
        {
          pkCfgDestroy (cfg);
          return status;
        }
    }

  pkCfgDestroy (client->cfg);
  client->cfg = cfg;
  pkMemCopy (client->dir, workdir, sizeof (workdir));
  pkMemCopy (client->ini, ini, sizeof (ini));

  return PK_OK;
}

const PKChar *
pkDir (const PKClient *client)
{
  return client->dir;
}

PKBool
pkLoginValid (const PKClient *client)
{
  const PKCfgLogin *login = client->cfg->login;
  return (login->access && *login->access)
         || (login->refresh && *login->refresh);
}

PKStatus
pkImport (PKClient *client, const PKChar *path)
{
  if (!*client->ini)
    {
      return pkFail (client, PK_EARG, 0, "No working directory, open first");
    }

  PKCfg *cfg = pkCfgInit ();
  if (!cfg)
    {
      return pkFailMem (client);
    }

  PKStatus status = pkCfgLoad (client, cfg, path);
  if (status == PK_OK)
    {
      status = pkCfgSave (client, cfg, client->ini);
    }
  if (status != PK_OK)
    {
      pkCfgDestroy (cfg);
      return pkErrorContext (client, "Cannot import %s", path);
    }

  pkCfgDestroy (client->cfg);
  client->cfg = cfg;

  return PK_OK;
}

PKStatus
pkLogout (PKClient *client)
{
  if (!*client->ini)
    {
      return pkFail (client, PK_EARG, 0, "No working directory, open first");
    }

  pkCfgUnsetLogin (client->cfg);

  return pkCfgSave (client, client->cfg, client->ini);
}
