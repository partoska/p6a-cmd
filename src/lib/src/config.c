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

#include "config.h"
#include "client.h"
#include "plat.h"
#include <bikini.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define ENDPOINT "https://api.partoska.com/rest/v1"

#define CLIENT_ID "b6ac7100-0000-4000-8000-000000000000"
#define DEVICE_URL "https://app.partoska.com/auth/v1/oauth/device"
#define TOKEN_URL "https://app.partoska.com/auth/v1/oauth/token"
#define SCOPE "event media rest"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Keys of the INI file, in the order they are written. */
static const struct
{
  const PKChar *section;
  const PKChar *key;
} pkCfgKeys[] = {
  { "Global", "Version" }, { "Global", "Endpoint" }, { "OAuth", "Client" },
  { "OAuth", "Device" },   { "OAuth", "Token" },     { "OAuth", "Scope" },
  { "Login", "Access" },   { "Login", "Refresh" },   { "Login", "Expires" },
};

/* pkCfgField(), pkCfgLoad() and pkCfgSave() handle exactly these keys. */
PK_COMPILE_ASSERT (PK_COUNTOF (pkCfgKeys) == 9);

/* The string setting behind pkCfgKeys[index], or NULL for Expires. */
static PKChar **
pkCfgField (PKCfg *cfg, PKSize index)
{
  PKChar **fields[]
      = { &cfg->glob->version, &cfg->glob->endpoint, &cfg->oauth->client,
          &cfg->oauth->device, &cfg->oauth->token,   &cfg->oauth->scope,
          &cfg->login->access, &cfg->login->refresh };

  return index < PK_COUNTOF (fields) ? fields[index] : NULL;
}

/* Copies a string; release it with pkMemFree(). */
static PKChar *
pkCfgCopy (const PKChar *str)
{
  return pkStrDup (str, pkStrLen (str));
}

/* Replaces a string setting with a copy of value. */
static PKStatus
pkCfgSet (PKChar **field, const PKChar *value)
{
  PKChar *copy = pkCfgCopy (value);
  if (!copy)
    {
      return PK_EMEM;
    }

  pkMemFree (*field);
  *field = copy;

  return PK_OK;
}

/* Writes text to a file; failures show in pkFileFailed(). */
static PKVoid
pkCfgWrite (PKFile *stream, const PKChar *text)
{
  pkFileWrite (stream, text, pkStrLen (text));
}

/* Feeds bkRead() from a file opened by pkFileOpen(), which takes UTF-8
 * paths on every platform. */
static BKSize
pkCfgRead (BKVoid *user, BKChar *data, BKSize size)
{
  PKFile *stream = (PKFile *)user;
  PKSize count = pkFileRead (stream, data, size);
  if (count < size && pkFileFailed (stream))
    {
      return BK_READ_ERROR;
    }

  return count;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const PKChar PK_CONFIG_INI[16] = "/p6a.ini";

PKCfg *
pkCfgInit (void)
{
  PKCfg *cfg = pkMemZalloc (sizeof (PKCfg));
  if (!cfg)
    {
      return NULL;
    }

  cfg->glob = pkMemZalloc (sizeof (PKCfgGlob));
  cfg->oauth = pkMemZalloc (sizeof (PKCfgOAuth));
  cfg->login = pkMemZalloc (sizeof (PKCfgLogin));
  if (!cfg->glob || !cfg->oauth || !cfg->login)
    {
      pkCfgDestroy (cfg);
      return NULL;
    }

  cfg->glob->version = pkCfgCopy (PK_VERSION_STRING);
  cfg->glob->endpoint = pkCfgCopy (ENDPOINT);
  cfg->oauth->client = pkCfgCopy (CLIENT_ID);
  cfg->oauth->device = pkCfgCopy (DEVICE_URL);
  cfg->oauth->token = pkCfgCopy (TOKEN_URL);
  cfg->oauth->scope = pkCfgCopy (SCOPE);
  if (!pkCfgCheck (cfg))
    {
      pkCfgDestroy (cfg);
      return NULL;
    }

  return cfg;
}

PKStatus
pkCfgLoad (PKClient *client, PKCfg *cfg, const PKChar *file)
{
  if (!file || !pkCfgCheck (cfg))
    {
      return pkFail (client, PK_EARG, 0, "Invalid configuration arguments");
    }

  PKFile *stream = pkFileOpen (file, PK_FALSE);
  if (!stream)
    {
      return pkFail (client, PK_EFS, pkSysError (), "Cannot read %s: %s", file,
                     pkSysErrorText (pkSysError ()));
    }

  BKIni *ini = bkInit (BK_FICASE);
  if (!ini)
    {
      pkFileClose (stream);
      return pkFailMem (client);
    }

  BKStatus status = bkRead (ini, pkCfgRead, stream);
  pkFileClose (stream);
  if (status != BK_OK)
    {
      PKStatus result
          = status == BK_EMEM ? pkFailMem (client)
            : status == BK_EIO
                ? pkFail (client, PK_EFS, 0, "Cannot read %s", file)
                : pkFail (client, PK_EDATA, 0, "%s: %s", file,
                          bkError (ini)->message);
      bkDestroy (ini);
      return result;
    }

  PKStatus result = PK_OK;
  for (PKSize i = 0; i < PK_COUNTOF (pkCfgKeys) && result == PK_OK; ++i)
    {
      const PKChar *value
          = bkGet (ini, pkCfgKeys[i].section, pkCfgKeys[i].key);
      PKChar **field = pkCfgField (cfg, i);
      if (!value)
        {
          continue;
        }
      if (!field)
        {
          cfg->login->expires = pkStrToTime (value);
        }
      else if (pkCfgSet (field, value) != PK_OK)
        {
          result = pkFailMem (client);
        }
    }

  bkDestroy (ini);

  return result;
}

PKStatus
pkCfgSave (PKClient *client, const PKCfg *cfg, const PKChar *file)
{
  if (!file || !pkCfgCheck (cfg))
    {
      return pkFail (client, PK_EARG, 0, "Invalid configuration arguments");
    }

  PKChar expires[32];
  pkStrFmtTime (expires, sizeof (expires), cfg->login->expires);

  const PKChar *values[PK_COUNTOF (pkCfgKeys)];
  for (PKSize i = 0; i < PK_COUNTOF (pkCfgKeys); ++i)
    {
      PKChar **field = pkCfgField ((PKCfg *)cfg, i);
      values[i] = !field ? expires : *field ? *field : "";

      // Every value takes one line.
      if (pkStrFindAny (values[i], "\r\n"))
        {
          return pkFail (client, PK_EARG, 0, "%s.%s spans lines",
                         pkCfgKeys[i].section, pkCfgKeys[i].key);
        }
    }
  values[0] = PK_VERSION_STRING;

  PKFile *stream = pkFileOpen (file, PK_TRUE);
  if (!stream)
    {
      return pkFail (client, PK_EFS, pkSysError (), "Cannot write %s: %s",
                     file, pkSysErrorText (pkSysError ()));
    }

  pkCfgWrite (stream, "; Partoska.com Configuration File\n");
  pkCfgWrite (stream,
              "; Auto-generated by p6a utility v" PK_VERSION_STRING ".\n");
  pkCfgWrite (
      stream,
      "; Do not modify. More info at <https://www.partoska.com/p6a>\n");

  const PKChar *section = NULL;
  for (PKSize i = 0; i < PK_COUNTOF (values); ++i)
    {
      if (!section || pkStrCmp (section, pkCfgKeys[i].section, PK_FALSE) != 0)
        {
          section = pkCfgKeys[i].section;
          pkCfgWrite (stream, "\n[");
          pkCfgWrite (stream, section);
          pkCfgWrite (stream, "]\n");
        }
      pkCfgWrite (stream, pkCfgKeys[i].key);
      pkCfgWrite (stream, "=");
      pkCfgWrite (stream, values[i]);
      pkCfgWrite (stream, "\n");
    }

  PKBool failed = pkFileFailed (stream);
  failed = !pkFileClose (stream) || failed;
  if (failed)
    {
      return pkFail (client, PK_EFS, pkSysError (), "Cannot write %s: %s",
                     file, pkSysErrorText (pkSysError ()));
    }

  if (pkChmod600 (file) != PK_OK)
    {
      return pkFail (client, PK_EFS, pkSysError (), "Cannot protect %s: %s",
                     file, pkSysErrorText (pkSysError ()));
    }

  return PK_OK;
}

PKStatus
pkCfgSetLogin (PKCfg *cfg, const PKChar *access, const PKChar *refresh,
               PKTime expires)
{
  if (!access || !refresh || !pkCfgCheck (cfg))
    {
      return PK_EARG;
    }

  PKChar *naccess = pkCfgCopy (access);
  PKChar *nrefresh = pkCfgCopy (refresh);
  if (!naccess || !nrefresh)
    {
      pkMemFree (naccess);
      pkMemFree (nrefresh);
      return PK_EMEM;
    }

  pkMemFree (cfg->login->access);
  pkMemFree (cfg->login->refresh);

  cfg->login->access = naccess;
  cfg->login->refresh = nrefresh;
  cfg->login->expires = expires;

  return PK_OK;
}

PKVoid
pkCfgUnsetLogin (PKCfg *cfg)
{
  if (!cfg || !cfg->login)
    {
      return;
    }

  pkMemFree (cfg->login->access);
  pkMemFree (cfg->login->refresh);

  cfg->login->access = NULL;
  cfg->login->refresh = NULL;
  cfg->login->expires = 0;
}

PKBool
pkCfgCheck (const PKCfg *cfg)
{
  if (!cfg || !cfg->glob || !cfg->oauth || !cfg->login)
    {
      return PK_FALSE;
    }

  return cfg->glob->version && cfg->glob->endpoint && cfg->oauth->client
         && cfg->oauth->device && cfg->oauth->token && cfg->oauth->scope;
}

PKVoid
pkCfgDestroy (PKCfg *cfg)
{
  if (!cfg)
    {
      return;
    }

  if (cfg->glob)
    {
      pkMemFree (cfg->glob->version);
      pkMemFree (cfg->glob->endpoint);
      pkMemFree (cfg->glob);
    }

  if (cfg->oauth)
    {
      pkMemFree (cfg->oauth->client);
      pkMemFree (cfg->oauth->device);
      pkMemFree (cfg->oauth->token);
      pkMemFree (cfg->oauth->scope);
      pkMemFree (cfg->oauth);
    }

  if (cfg->login)
    {
      pkMemFree (cfg->login->access);
      pkMemFree (cfg->login->refresh);
      pkMemFree (cfg->login);
    }

  pkMemFree (cfg);
}
