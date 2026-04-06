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
#include "ini.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define ENDPOINT "https://api.partoska.com/rest/v1"

#define REDIRECT_URL "https://app.partoska.com/callback/p6a"
#define CLIENT_ID "b6ac7100-0000-4000-8000-000000000000"
#define AUTHORIZE_URL "https://app.partoska.com/auth/v1/oauth/authorize"
#define TOKEN_URL "https://app.partoska.com/auth/v1/oauth/token"
#define SCOPE "event media rest"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PLChar *
plStrDup (const PLChar *str)
{
  if (!str)
    {
      return NULL;
    }

  return strdup (str);
}

static void
plCfgGlobDestroy (PLCfgGlob *glob)
{
  if (!glob)
    {
      return;
    }

  free (glob->version);
  free (glob->endpoint);
  free (glob);
}

static PLCfgGlob *
plCfgGlobInit (void)
{
  PLCfgGlob *glob = (PLCfgGlob *)malloc (sizeof (PLCfgGlob));
  if (!glob)
    {
      return NULL;
    }

  glob->version = plStrDup (PL_VERSION_STRING);
  glob->endpoint = plStrDup (ENDPOINT);
  if (!glob->version || !glob->endpoint)
    {
      plCfgGlobDestroy (glob);
      return NULL;
    }

  return glob;
}

static void
plCfgAuthDestroy (PLCfgOAuth *auth)
{
  if (!auth)
    {
      return;
    }

  free (auth->client);
  free (auth->redirect);
  free (auth->authorize);
  free (auth->token);
  free (auth->scope);
  free (auth);
}

static PLCfgOAuth *
plCfgAuthInit (void)
{
  PLCfgOAuth *auth = (PLCfgOAuth *)malloc (sizeof (PLCfgOAuth));
  if (!auth)
    {
      return NULL;
    }

  auth->client = plStrDup (CLIENT_ID);
  auth->redirect = plStrDup (REDIRECT_URL);
  auth->authorize = plStrDup (AUTHORIZE_URL);
  auth->token = plStrDup (TOKEN_URL);
  auth->scope = plStrDup (SCOPE);
  if (!auth->client || !auth->redirect || !auth->authorize || !auth->token
      || !auth->scope)
    {
      plCfgAuthDestroy (auth);
      return NULL;
    }

  return auth;
}

static void
plCfgLoginDestroy (PLCfgLogin *login)
{
  if (!login)
    {
      return;
    }

  free (login->access);
  free (login->refresh);
  free (login);
}

static PLCfgLogin *
plCfgLoginInit (void)
{
  PLCfgLogin *login = (PLCfgLogin *)malloc (sizeof (PLCfgLogin));
  if (!login)
    {
      return NULL;
    }

  login->access = NULL;
  login->refresh = NULL;
  login->expires = 0;

  return login;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const PLChar PL_CONFIG_INI[16] = "/p6a.ini";

PLCfg *
plCfgInit (void)
{
  PLCfg *cfg = (PLCfg *)malloc (sizeof (PLCfg));
  if (!cfg)
    {
      return NULL;
    }

  cfg->glob = plCfgGlobInit ();
  cfg->oauth = plCfgAuthInit ();
  cfg->login = plCfgLoginInit ();
  if (!cfg->glob || !cfg->oauth || !cfg->login)
    {
      plCfgDestroy (cfg);
      return NULL;
    }

  return cfg;
}

PLInt
plCfgLoad (PLCfg *cfg, const PLChar *file)
{
  if (!file)
    {
      return PL_EARG;
    }

  if (!plCfgCheck (cfg))
    {
      return PL_EARG;
    }

  PLIni *ini = plIniInit ();
  if (!ini)
    {
      return PL_EMEM;
    }

  PLInt result = plIniLoad (ini, file);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  const PLChar *value = plIniGet (ini, "global", "VERSION");
  if (value)
    {
      free (cfg->glob->version);
      cfg->glob->version = plStrDup (value);
    }

  value = plIniGet (ini, "global", "ENDPOINT");
  if (value)
    {
      free (cfg->glob->endpoint);
      cfg->glob->endpoint = plStrDup (value);
    }

  value = plIniGet (ini, "oauth", "CLIENT");
  if (value)
    {
      free (cfg->oauth->client);
      cfg->oauth->client = plStrDup (value);
    }

  value = plIniGet (ini, "oauth", "REDIRECT");
  if (value)
    {
      free (cfg->oauth->redirect);
      cfg->oauth->redirect = plStrDup (value);
    }

  value = plIniGet (ini, "oauth", "AUTHORIZE");
  if (value)
    {
      free (cfg->oauth->authorize);
      cfg->oauth->authorize = plStrDup (value);
    }

  value = plIniGet (ini, "oauth", "TOKEN");
  if (value)
    {
      free (cfg->oauth->token);
      cfg->oauth->token = plStrDup (value);
    }

  value = plIniGet (ini, "oauth", "SCOPE");
  if (value)
    {
      free (cfg->oauth->scope);
      cfg->oauth->scope = plStrDup (value);
    }

  value = plIniGet (ini, "login", "ACCESS");
  if (value)
    {
      free (cfg->login->access);
      cfg->login->access = plStrDup (value);
    }

  value = plIniGet (ini, "login", "REFRESH");
  if (value)
    {
      free (cfg->login->refresh);
      cfg->login->refresh = plStrDup (value);
    }

  value = plIniGet (ini, "login", "EXPIRES");
  if (value)
    {
      cfg->login->expires = atol (value);
    }

  plIniDestroy (ini);

  return PL_EOK;
}

PLInt
plCfgSave (const PLCfg *cfg, const PLChar *file)
{
  if (!file)
    {
      return PL_EARG;
    }

  if (!plCfgCheck (cfg))
    {
      return PL_EARG;
    }

  PLIni *ini = plIniInit ();
  if (!ini)
    {
      return PL_EMEM;
    }

  PLInt result = plIniSet (ini, "global", "VERSION", cfg->glob->version);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  result = plIniSet (ini, "global", "ENDPOINT", cfg->glob->endpoint);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  result = plIniSet (ini, "oauth", "CLIENT", cfg->oauth->client);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  result = plIniSet (ini, "oauth", "REDIRECT", cfg->oauth->redirect);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  result = plIniSet (ini, "oauth", "AUTHORIZE", cfg->oauth->authorize);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  result = plIniSet (ini, "oauth", "TOKEN", cfg->oauth->token);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  result = plIniSet (ini, "oauth", "SCOPE", cfg->oauth->scope);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  result = plIniSet (ini, "login", "ACCESS",
                     cfg->login->access ? cfg->login->access : "");
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  result = plIniSet (ini, "login", "REFRESH",
                     cfg->login->refresh ? cfg->login->refresh : "");
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  PLChar expires[32];
  snprintf (expires, PL_CHARSMAX (expires), "%lld",
            (PLLong)cfg->login->expires);
  expires[PL_CHARSMAX (expires)] = '\0';
  result = plIniSet (ini, "login", "EXPIRES", expires);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  result = plIniSave (ini, file);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  result = plChmod600 (file);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  plIniDestroy (ini);

  return PL_EOK;
}

PLInt
plCfgSetLogin (PLCfg *cfg, const PLChar *access, const PLChar *refresh,
               PLLong expires)
{
  if (!access || !refresh)
    {
      return PL_EARG;
    }

  if (!plCfgCheck (cfg))
    {
      return PL_EARG;
    }

  PLChar *naccess = plStrDup (access);
  PLChar *nrefresh = plStrDup (refresh);
  if (!naccess || !nrefresh)
    {
      free (naccess);
      free (nrefresh);
      return PL_EMEM;
    }

  free (cfg->login->access);
  free (cfg->login->refresh);

  cfg->login->access = naccess;
  cfg->login->refresh = nrefresh;
  cfg->login->expires = expires;

  return PL_EOK;
}

PLInt
plCfgUnsetLogin (PLCfg *cfg)
{
  if (!plCfgCheck (cfg))
    {
      return PL_EARG;
    }

  free (cfg->login->access);
  free (cfg->login->refresh);

  cfg->login->access = NULL;
  cfg->login->refresh = NULL;
  cfg->login->expires = 0;

  return PL_EOK;
}

PLBool
plCfgCheck (const PLCfg *cfg)
{
  if (!cfg || !cfg->glob || !cfg->oauth || !cfg->login)
    {
      return PL_FALSE;
    }

  const PLCfgGlob *glob = cfg->glob;
  if (!glob->endpoint || !glob->version)
    {
      return PL_FALSE;
    }

  const PLCfgOAuth *auth = cfg->oauth;
  if (!auth->client || !auth->redirect || !auth->authorize || !auth->token
      || !auth->scope)
    {
      return PL_FALSE;
    }

  return PL_TRUE;
}

void
plCfgDestroy (PLCfg *cfg)
{
  if (!cfg)
    {
      return;
    }

  plCfgGlobDestroy (cfg->glob);
  plCfgAuthDestroy (cfg->oauth);
  plCfgLoginDestroy (cfg->login);
  free (cfg);
}
