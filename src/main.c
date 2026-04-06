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

#include "arg.h"
#include "config.h"
#include "create.h"
#include "download.h"
#include "fs.h"
#include "link.h"
#include "list.h"
#include "logger.h"
#include "media.h"
#include "oauth.h"
#include "qr.h"
#include "sync.h"
#include "types.h"
#include "update.h"
#include "wdir.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define WORKDIR_MAX (1024)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
plPrintLogo (void)
{
  PL_INFO ("   ___           __           __");
  PL_INFO ("  / _ \\___ _____/ /____  ___ / /_____ _");
  PL_INFO (" / ___/ _ `/ __/ __/ _ \\(_-</  '_/ _ `/");
  PL_INFO ("/_/   \\_,_/_/  \\__/\\___/___/_/\\_\\\\_,_/");
  PL_INFO ("");
  PL_INFO ("Partoska Command-Line Interface (p6a) v" PL_VERSION_STRING);
  PL_INFO ("(C) 2026 Fabrika Charvat s.r.o., <https://lab.partoska.com/p6a>");
  PL_INFO ("");
  PL_INFO ("Licensed under the MIT License.");
  PL_INFO ("");
}

static PLInt
plDoLogin (const PLChar *dir, const PLChar *import)
{
  plPrintLogo ();

  PLChar workdir[WORKDIR_MAX];
  if (dir != NULL)
    {
      strncpy (workdir, dir, PL_CHARSMAX (workdir));
      workdir[PL_CHARSMAX (workdir)] = '\0';
    }
  else
    {
      workdir[0] = '\0';
    }

  PLInt result = plPrepareWorkdir (workdir, sizeof (workdir));
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to prepare working directory");
      return result;
    }

  PLChar ini[sizeof (workdir) + sizeof (PL_CONFIG_INI)];
  plGetIniPath (ini, sizeof (ini), workdir);

  PLCfg *config = plCfgInit ();
  if (!config)
    {
      PL_ERROR ("Failed to initialize configuration");
      return PL_EMEM;
    }

  if (import != NULL)
    {
      PL_DEBUG ("Importing: %s", import);
      result = plCfgLoad (config, import);
      if (result != PL_EOK && result != PL_EFS)
        {
          PL_ERROR ("Failed to import configuration");
          plCfgDestroy (config);
          return result;
        }

      if (!plCfgCheck (config))
        {
          PL_ERROR ("Invalid configuration detected, import canceled ...");
          plCfgDestroy (config);
          return PL_EARG;
        }

      result = plCfgSave (config, ini);
      if (result != PL_EOK)
        {
          PL_ERROR ("Failed to save configuration");
          plCfgDestroy (config);
          return result;
        }
    }
  else
    {
      result = plCfgLoad (config, ini);
      if (result != PL_EOK && result != PL_EFS)
        {
          PL_ERROR ("Failed to load configuration");
          plCfgDestroy (config);
          return result;
        }

      if (!plCfgCheck (config))
        {
          PL_WARN ("Invalid configuration detected, using defaults ...");
          plCfgDestroy (config);

          config = plCfgInit ();
          if (!config)
            {
              PL_ERROR ("Failed to initialize configuration");
              return PL_EMEM;
            }
        }
    }

  curl_global_init (CURL_GLOBAL_DEFAULT);

  result = plOAuthLogin (config, ini);
  if (result != PL_EOK)
    {
      curl_global_cleanup ();
      plCfgDestroy (config);
      return result;
    }

  curl_global_cleanup ();
  plCfgDestroy (config);

  return PL_EOK;
}

static PLInt
plDoLogout (const PLChar *dir)
{
  PLChar workdir[WORKDIR_MAX];
  if (dir != NULL)
    {
      strncpy (workdir, dir, PL_CHARSMAX (workdir));
      workdir[PL_CHARSMAX (workdir)] = '\0';
    }
  else
    {
      workdir[0] = '\0';
    }

  PLInt result = plPrepareWorkdir (workdir, sizeof (workdir));
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to prepare working directory");
      return result;
    }

  PLChar ini[sizeof (workdir) + sizeof (PL_CONFIG_INI)];
  plGetIniPath (ini, sizeof (ini), workdir);

  PLCfg *config = plCfgInit ();
  if (!config)
    {
      PL_ERROR ("Failed to initialize configuration");
      return PL_EMEM;
    }

  result = plCfgLoad (config, ini);
  if (result != PL_EOK && result != PL_EFS)
    {
      PL_ERROR ("Failed to load configuration");
      plCfgDestroy (config);
      return result;
    }

  if (!plCfgCheck (config))
    {
      PL_WARN ("Invalid configuration detected, using defaults ...");
      plCfgDestroy (config);

      config = plCfgInit ();
      if (!config)
        {
          PL_ERROR ("Failed to initialize configuration");
          return PL_EMEM;
        }
    }

  result = plCfgUnsetLogin (config);
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to logout");
      plCfgDestroy (config);
      return result;
    }

  result = plCfgSave (config, ini);
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to save configuration");
      plCfgDestroy (config);
      return result;
    }

  return PL_EOK;
}

static PLInt
plDoSync (const PLChar *dir, const PLChar *target, PLBool owner,
          PLBool favorite)
{
  if (!target)
    {
      PL_ERROR ("Invalid target directory");
      return PL_EARG;
    }

  PLChar workdir[WORKDIR_MAX];
  if (dir != NULL)
    {
      strncpy (workdir, dir, PL_CHARSMAX (workdir));
      workdir[PL_CHARSMAX (workdir)] = '\0';
    }
  else
    {
      workdir[0] = '\0';
    }

  PLInt result = plPrepareWorkdir (workdir, sizeof (workdir));
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to prepare working directory");
      return result;
    }

  PLChar ini[sizeof (workdir) + sizeof (PL_CONFIG_INI)];
  plGetIniPath (ini, sizeof (ini), workdir);

  PLCfg *config = plCfgInit ();
  if (!config)
    {
      PL_ERROR ("Failed to initialize configuration");
      return PL_EMEM;
    }

  result = plCfgLoad (config, ini);
  if (result != PL_EOK)
    {
      switch (result)
        {
        case PL_EFS:
          PL_ERROR ("Missing configuration, please login first");
          break;
        default:
          PL_ERROR ("Failed to load configuration");
          break;
        }

      plCfgDestroy (config);
      return result;
    }

  if (!plCfgCheck (config))
    {
      PL_ERROR ("Invalid configuration, please login again");
      plCfgDestroy (config);
      return PL_EARG;
    }

  curl_global_init (CURL_GLOBAL_DEFAULT);

  PLChar *bearer = plOAuthGet (config, ini);
  if (!bearer)
    {
      curl_global_cleanup ();
      plCfgDestroy (config);
      return PL_ENET;
    }

  result = plSync (config->glob->endpoint, bearer, target, owner, favorite);

  free (bearer);
  curl_global_cleanup ();
  plCfgDestroy (config);

  return result;
}

static PLInt
plDoList (const PLChar *dir, const PLChar *query, PLBool owner,
          PLBool favorite, PLArgFmt fmt)
{
  PLChar workdir[WORKDIR_MAX];
  if (dir != NULL)
    {
      strncpy (workdir, dir, PL_CHARSMAX (workdir));
      workdir[PL_CHARSMAX (workdir)] = '\0';
    }
  else
    {
      workdir[0] = '\0';
    }

  PLInt result = plPrepareWorkdir (workdir, sizeof (workdir));
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to prepare working directory");
      return result;
    }

  PLChar ini[sizeof (workdir) + sizeof (PL_CONFIG_INI)];
  plGetIniPath (ini, sizeof (ini), workdir);

  PLCfg *config = plCfgInit ();
  if (!config)
    {
      PL_ERROR ("Failed to initialize configuration");
      return PL_EMEM;
    }

  result = plCfgLoad (config, ini);
  if (result != PL_EOK)
    {
      switch (result)
        {
        case PL_EFS:
          PL_ERROR ("Missing configuration, please login first");
          break;
        default:
          PL_ERROR ("Failed to load configuration");
          break;
        }

      plCfgDestroy (config);
      return result;
    }

  if (!plCfgCheck (config))
    {
      PL_ERROR ("Invalid configuration, please login again");
      plCfgDestroy (config);
      return PL_EARG;
    }

  curl_global_init (CURL_GLOBAL_DEFAULT);

  PLChar *bearer = plOAuthGet (config, ini);
  if (!bearer)
    {
      curl_global_cleanup ();
      plCfgDestroy (config);
      return PL_ENET;
    }

  result
      = plList (config->glob->endpoint, bearer, query, owner, favorite, fmt);

  free (bearer);
  curl_global_cleanup ();
  plCfgDestroy (config);

  return result;
}

static PLInt
plDoCreate (const PLChar *dir, const PLChar *name, PLArgFmt fmt)
{
  if (!name)
    {
      PL_ERROR ("Event name is required");
      return PL_EARG;
    }

  PLChar workdir[WORKDIR_MAX];
  if (dir != NULL)
    {
      strncpy (workdir, dir, PL_CHARSMAX (workdir));
      workdir[PL_CHARSMAX (workdir)] = '\0';
    }
  else
    {
      workdir[0] = '\0';
    }

  PLInt result = plPrepareWorkdir (workdir, sizeof (workdir));
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to prepare working directory");
      return result;
    }

  PLChar ini[sizeof (workdir) + sizeof (PL_CONFIG_INI)];
  plGetIniPath (ini, sizeof (ini), workdir);

  PLCfg *config = plCfgInit ();
  if (!config)
    {
      PL_ERROR ("Failed to initialize configuration");
      return PL_EMEM;
    }

  result = plCfgLoad (config, ini);
  if (result != PL_EOK)
    {
      switch (result)
        {
        case PL_EFS:
          PL_ERROR ("Missing configuration, please login first");
          break;
        default:
          PL_ERROR ("Failed to load configuration");
          break;
        }

      plCfgDestroy (config);
      return result;
    }

  if (!plCfgCheck (config))
    {
      PL_ERROR ("Invalid configuration, please login again");
      plCfgDestroy (config);
      return PL_EARG;
    }

  curl_global_init (CURL_GLOBAL_DEFAULT);

  PLChar *bearer = plOAuthGet (config, ini);
  if (!bearer)
    {
      curl_global_cleanup ();
      plCfgDestroy (config);
      return PL_ENET;
    }

  result = plCreate (config->glob->endpoint, bearer, name, fmt);

  free (bearer);
  curl_global_cleanup ();
  plCfgDestroy (config);

  return result;
}

static PLInt
plDoUpdate (const PLChar *dir, const PLChar *id, const PLChar *name,
            const PLChar *from, const PLChar *to, PLInt pub, PLInt fav)
{
  if (!id)
    {
      PL_ERROR ("Event ID is required");
      return PL_EARG;
    }

  PLChar workdir[WORKDIR_MAX];
  if (dir != NULL)
    {
      strncpy (workdir, dir, PL_CHARSMAX (workdir));
      workdir[PL_CHARSMAX (workdir)] = '\0';
    }
  else
    {
      workdir[0] = '\0';
    }

  PLInt result = plPrepareWorkdir (workdir, sizeof (workdir));
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to prepare working directory");
      return result;
    }

  PLChar ini[sizeof (workdir) + sizeof (PL_CONFIG_INI)];
  plGetIniPath (ini, sizeof (ini), workdir);

  PLCfg *config = plCfgInit ();
  if (!config)
    {
      PL_ERROR ("Failed to initialize configuration");
      return PL_EMEM;
    }

  result = plCfgLoad (config, ini);
  if (result != PL_EOK)
    {
      switch (result)
        {
        case PL_EFS:
          PL_ERROR ("Missing configuration, please login first");
          break;
        default:
          PL_ERROR ("Failed to load configuration");
          break;
        }

      plCfgDestroy (config);
      return result;
    }

  if (!plCfgCheck (config))
    {
      PL_ERROR ("Invalid configuration, please login again");
      plCfgDestroy (config);
      return PL_EARG;
    }

  curl_global_init (CURL_GLOBAL_DEFAULT);

  PLChar *bearer = plOAuthGet (config, ini);
  if (!bearer)
    {
      curl_global_cleanup ();
      plCfgDestroy (config);
      return PL_ENET;
    }

  result = plUpdate (config->glob->endpoint, bearer, id, name, from, to, pub,
                     fav);

  free (bearer);
  curl_global_cleanup ();
  plCfgDestroy (config);

  return result;
}

static PLInt
plDoQr (const PLChar *dir, const PLChar *event, const PLChar *output,
        PLBool svg)
{
  if (!event)
    {
      PL_ERROR ("Event ID is required");
      return PL_EARG;
    }

  PLChar workdir[WORKDIR_MAX];
  if (dir != NULL)
    {
      strncpy (workdir, dir, PL_CHARSMAX (workdir));
      workdir[PL_CHARSMAX (workdir)] = '\0';
    }
  else
    {
      workdir[0] = '\0';
    }

  PLInt result = plPrepareWorkdir (workdir, sizeof (workdir));
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to prepare working directory");
      return result;
    }

  PLChar ini[sizeof (workdir) + sizeof (PL_CONFIG_INI)];
  plGetIniPath (ini, sizeof (ini), workdir);

  PLCfg *config = plCfgInit ();
  if (!config)
    {
      PL_ERROR ("Failed to initialize configuration");
      return PL_EMEM;
    }

  result = plCfgLoad (config, ini);
  if (result != PL_EOK)
    {
      switch (result)
        {
        case PL_EFS:
          PL_ERROR ("Missing configuration, please login first");
          break;
        default:
          PL_ERROR ("Failed to load configuration");
          break;
        }

      plCfgDestroy (config);
      return result;
    }

  if (!plCfgCheck (config))
    {
      PL_ERROR ("Invalid configuration, please login again");
      plCfgDestroy (config);
      return PL_EARG;
    }

  curl_global_init (CURL_GLOBAL_DEFAULT);

  PLChar *bearer = plOAuthGet (config, ini);
  if (!bearer)
    {
      curl_global_cleanup ();
      plCfgDestroy (config);
      return PL_ENET;
    }

  result = plQr (config->glob->endpoint, bearer, event, output, svg);

  free (bearer);
  curl_global_cleanup ();
  plCfgDestroy (config);

  return result;
}

static PLInt
plDoLink (const PLChar *dir, const PLChar *event)
{
  if (!event)
    {
      PL_ERROR ("Event ID is required");
      return PL_EARG;
    }

  PLChar workdir[WORKDIR_MAX];
  if (dir != NULL)
    {
      strncpy (workdir, dir, PL_CHARSMAX (workdir));
      workdir[PL_CHARSMAX (workdir)] = '\0';
    }
  else
    {
      workdir[0] = '\0';
    }

  PLInt result = plPrepareWorkdir (workdir, sizeof (workdir));
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to prepare working directory");
      return result;
    }

  PLChar ini[sizeof (workdir) + sizeof (PL_CONFIG_INI)];
  plGetIniPath (ini, sizeof (ini), workdir);

  PLCfg *config = plCfgInit ();
  if (!config)
    {
      PL_ERROR ("Failed to initialize configuration");
      return PL_EMEM;
    }

  result = plCfgLoad (config, ini);
  if (result != PL_EOK)
    {
      switch (result)
        {
        case PL_EFS:
          PL_ERROR ("Missing configuration, please login first");
          break;
        default:
          PL_ERROR ("Failed to load configuration");
          break;
        }

      plCfgDestroy (config);
      return result;
    }

  if (!plCfgCheck (config))
    {
      PL_ERROR ("Invalid configuration, please login again");
      plCfgDestroy (config);
      return PL_EARG;
    }

  curl_global_init (CURL_GLOBAL_DEFAULT);

  PLChar *bearer = plOAuthGet (config, ini);
  if (!bearer)
    {
      curl_global_cleanup ();
      plCfgDestroy (config);
      return PL_ENET;
    }

  result = plLink (config->glob->endpoint, bearer, event);

  free (bearer);
  curl_global_cleanup ();
  plCfgDestroy (config);

  return result;
}

static PLInt
plDoMedia (const PLChar *dir, const PLChar *event, PLBool owner,
           PLBool favorite, PLArgFmt fmt)
{
  if (!event)
    {
      PL_ERROR ("Event ID is required");
      return PL_EARG;
    }

  PLChar workdir[WORKDIR_MAX];
  if (dir != NULL)
    {
      strncpy (workdir, dir, PL_CHARSMAX (workdir));
      workdir[PL_CHARSMAX (workdir)] = '\0';
    }
  else
    {
      workdir[0] = '\0';
    }

  PLInt result = plPrepareWorkdir (workdir, sizeof (workdir));
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to prepare working directory");
      return result;
    }

  PLChar ini[sizeof (workdir) + sizeof (PL_CONFIG_INI)];
  plGetIniPath (ini, sizeof (ini), workdir);

  PLCfg *config = plCfgInit ();
  if (!config)
    {
      PL_ERROR ("Failed to initialize configuration");
      return PL_EMEM;
    }

  result = plCfgLoad (config, ini);
  if (result != PL_EOK)
    {
      switch (result)
        {
        case PL_EFS:
          PL_ERROR ("Missing configuration, please login first");
          break;
        default:
          PL_ERROR ("Failed to load configuration");
          break;
        }

      plCfgDestroy (config);
      return result;
    }

  if (!plCfgCheck (config))
    {
      PL_ERROR ("Invalid configuration, please login again");
      plCfgDestroy (config);
      return PL_EARG;
    }

  curl_global_init (CURL_GLOBAL_DEFAULT);

  PLChar *bearer = plOAuthGet (config, ini);
  if (!bearer)
    {
      curl_global_cleanup ();
      plCfgDestroy (config);
      return PL_ENET;
    }

  result = plMediaList (config->glob->endpoint, bearer, event, owner, favorite,
                        fmt);

  free (bearer);
  curl_global_cleanup ();
  plCfgDestroy (config);

  return result;
}

static PLInt
plDoDownload (const PLChar *dir, const PLChar *event, const PLChar *media,
              const PLChar *target, PLBool owner, PLBool favorite)
{
  if (!event)
    {
      PL_ERROR ("Event ID is required");
      return PL_EARG;
    }

  PLChar workdir[WORKDIR_MAX];
  if (dir != NULL)
    {
      strncpy (workdir, dir, PL_CHARSMAX (workdir));
      workdir[PL_CHARSMAX (workdir)] = '\0';
    }
  else
    {
      workdir[0] = '\0';
    }

  PLInt result = plPrepareWorkdir (workdir, sizeof (workdir));
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to prepare working directory");
      return result;
    }

  PLChar ini[sizeof (workdir) + sizeof (PL_CONFIG_INI)];
  plGetIniPath (ini, sizeof (ini), workdir);

  PLCfg *config = plCfgInit ();
  if (!config)
    {
      PL_ERROR ("Failed to initialize configuration");
      return PL_EMEM;
    }

  result = plCfgLoad (config, ini);
  if (result != PL_EOK)
    {
      switch (result)
        {
        case PL_EFS:
          PL_ERROR ("Missing configuration, please login first");
          break;
        default:
          PL_ERROR ("Failed to load configuration");
          break;
        }

      plCfgDestroy (config);
      return result;
    }

  if (!plCfgCheck (config))
    {
      PL_ERROR ("Invalid configuration, please login again");
      plCfgDestroy (config);
      return PL_EARG;
    }

  curl_global_init (CURL_GLOBAL_DEFAULT);

  PLChar *bearer = plOAuthGet (config, ini);
  if (!bearer)
    {
      curl_global_cleanup ();
      plCfgDestroy (config);
      return PL_ENET;
    }

  if (media != NULL)
    {
      result = plDownloadSingle (config->glob->endpoint, bearer, event, media,
                                 target);
    }
  else
    {
      result = plDownloadAll (config->glob->endpoint, bearer, event, target,
                              owner, favorite);
    }

  free (bearer);
  curl_global_cleanup ();
  plCfgDestroy (config);

  return result;
}

static PLInt
plDoVersion (void)
{
  PL_INFO ("p6a v" PL_VERSION_STRING);
  return PL_EOK;
}

static PLInt
plDoHelp (void)
{
  plPrintLogo ();

  PL_INFO ("Usage: p6a <command> [options]");
  PL_INFO ("");
  PL_INFO ("Commands:");
  PL_INFO ("  login               Setup connection with Partoska");
  PL_INFO ("  logout              Remove authentication credentials");
  PL_INFO ("  sync                Synchronize photos from Partoska");
  PL_INFO ("  list                List events");
  PL_INFO ("  create              Create a new event");
  PL_INFO ("  update              Update an existing event");
  PL_INFO ("  qr                  Download QR code for an event");
  PL_INFO ("  link                Get invite link for an event");
  PL_INFO ("  media               List media items for an event");
  PL_INFO ("  download            Download media for an event");
  PL_INFO ("  help                Display this help message");
  PL_INFO ("  version             Print version information");
  PL_INFO ("");
  PL_INFO ("Login options:");
  PL_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PL_INFO ("  -i, --import <file> Import configuration from file");
  PL_INFO ("");
  PL_INFO ("Synchronize options:");
  PL_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PL_INFO ("  -t, --target <path> Target directory for sync (required)");
  PL_INFO ("  -o, --owner-only    Synchronize only events owned by user");
  PL_INFO ("  -f, --favorite-only Synchronize only favorite events");
  PL_INFO ("");
  PL_INFO ("List events options:");
  PL_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PL_INFO ("  -q, --query <text>  Filter events by name");
  PL_INFO ("  -o, --owner-only    List only events owned by user");
  PL_INFO ("  -f, --favorite-only List only favorite events");
  PL_INFO (
      "  -1                  Print IDs only, one per line (same as -F one)");
  PL_INFO (
      "  -F, --format <fmt>  Output format: plain (default), json, csv, one");
  PL_INFO ("");
  PL_INFO ("Create event options:");
  PL_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PL_INFO ("  -n, --name <name>   Event name, 3-48 characters (required)");
  PL_INFO ("  -1                  Print ID only (same as -F one)");
  PL_INFO (
      "  -F, --format <fmt>  Output format: plain (default), json, csv, one");
  PL_INFO ("");
  PL_INFO ("Update event options:");
  PL_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PL_INFO ("  -e, --event <id>    Event ID (required)");
  PL_INFO ("  -n, --name <name>   New event name, 3-48 characters");
  PL_INFO ("  -S, --from <dt>     Event start date-time (ISO 8601)");
  PL_INFO ("  -E, --to <dt>       Event end date-time (ISO 8601)");
  PL_INFO ("  -p, --public        Make event public");
  PL_INFO ("  -P, --private       Make event private");
  PL_INFO ("  -f, --favorite      Mark event as favorite");
  PL_INFO ("  -F, --no-favorite   Unmark event as favorite");
  PL_INFO ("");
  PL_INFO ("QR code options:");
  PL_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PL_INFO ("  -e, --event <id>    Event ID (required)");
  PL_INFO (
      "  -t, --target <file> Output file path (default: <id>-qr.png/svg)");
  PL_INFO ("  -s, --svg           Request SVG format instead of PNG");
  PL_INFO ("");
  PL_INFO ("Invite link options:");
  PL_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PL_INFO ("  -e, --event <id>    Event ID (required)");
  PL_INFO ("");
  PL_INFO ("List media options:");
  PL_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PL_INFO ("  -e, --event <id>    Event ID (required)");
  PL_INFO ("  -o, --owner-only    List only media uploaded by user");
  PL_INFO ("  -f, --favorite-only List only favorited media");
  PL_INFO (
      "  -1                  Print IDs only, one per line (same as -F one)");
  PL_INFO (
      "  -F, --format <fmt>  Output format: plain (default), json, csv, one");
  PL_INFO ("");
  PL_INFO ("Download media options:");
  PL_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PL_INFO ("  -e, --event <id>    Event ID (required)");
  PL_INFO ("");
  PL_INFO ("  All-media mode (mutually exclusive with single-media):");
  PL_INFO ("  -t, --target <path> Target directory (required)");
  PL_INFO ("  -o, --owner-only    Download only media uploaded by user");
  PL_INFO ("  -f, --favorite-only Download only favorited media");
  PL_INFO ("");
  PL_INFO ("  Single-media mode (mutually exclusive with all-media):");
  PL_INFO ("  -m, --media <id>    Media ID (required)");
  PL_INFO ("  -t, --target <file> Output file path (required)");
  PL_INFO ("");
  PL_INFO ("Examples:");
  PL_INFO ("  p6a login");
  PL_INFO ("  p6a list");
  PL_INFO ("  p6a list -q \"Birthday\" --favorite-only");
  PL_INFO ("  p6a list -F json | jq '.[].name'");
  PL_INFO ("  p6a list -1 | xargs -I{} p6a qr -e {}");
  PL_INFO ("  p6a create -n \"Birthday Party\"");
  PL_INFO (
      "  p6a update -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -n \"New Name\"");
  PL_INFO ("  p6a sync -t ./photos -f");
  PL_INFO ("  p6a sync --target ./media --owner-only");
  PL_INFO ("  p6a qr -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7");
  PL_INFO ("  p6a qr --event 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -t qr.png");
  PL_INFO ("  p6a link -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7");
  PL_INFO ("  p6a media -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7");
  PL_INFO ("  p6a media -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -F json");
  PL_INFO (
      "  p6a download -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -t ./photos");
  PL_INFO (
      "  p6a download -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -t ./photos "
      "--owner-only");
  PL_INFO ("  p6a download -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 "
           "-m 7a8b9c0d-f00d-4a3b-8c5d-e6f700010203 -t photo.jpg");
  PL_INFO ("  p6a logout");
  PL_INFO ("");

  return PL_EOK;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PLInt
main (PLInt argc, PLChar **argv)
{
  plConsoleInit ();

  PLArg args;
  if (plArgParse (&args, argc, argv) != PL_EOK)
    {
      PL_ERROR ("Invalid program parameters, use -h for help");
      return EXIT_FAILURE;
    }

  switch (args.cmd)
    {
    case PL_CLOGIN:
      {
        if (plDoLogin (args.dir, args.c.login.path) != PL_EOK)
          {
            return EXIT_FAILURE;
          };
        break;
      }
    case PL_CLOGOUT:
      {
        if (plDoLogout (args.dir) != PL_EOK)
          {
            return EXIT_FAILURE;
          };
        break;
      }
    case PL_CSYNC:
      {
        if (plDoSync (args.dir, args.c.sync.target, args.flags & PL_FOWN,
                      args.flags & PL_FFAV)
            != PL_EOK)
          {
            return EXIT_FAILURE;
          };
        break;
      }
    case PL_CLIST:
      {
        if (plDoList (args.dir, args.c.list.query, args.flags & PL_FOWN,
                      args.flags & PL_FFAV, PL_ARGFMT (args.flags))
            != PL_EOK)
          {
            return EXIT_FAILURE;
          };
        break;
      }
    case PL_CCREATE:
      {
        if (plDoCreate (args.dir, args.c.create.name, PL_ARGFMT (args.flags))
            != PL_EOK)
          {
            return EXIT_FAILURE;
          };
        break;
      }
    case PL_CUPDATE:
      {
        if (plDoUpdate (args.dir, args.c.update.event, args.c.update.name,
                        args.c.update.from, args.c.update.to,
                        PL_ARGPUB (args.flags), PL_ARGFAV (args.flags))
            != PL_EOK)
          {
            return EXIT_FAILURE;
          };
        break;
      }
    case PL_CQR:
      {
        if (plDoQr (args.dir, args.c.qr.event, args.c.qr.out,
                    (args.flags & PL_FSVG) != 0)
            != PL_EOK)
          {
            return EXIT_FAILURE;
          };
        break;
      }
    case PL_CLINK:
      {
        if (plDoLink (args.dir, args.c.link.event) != PL_EOK)
          {
            return EXIT_FAILURE;
          };
        break;
      }
    case PL_CMEDIA:
      {
        if (plDoMedia (args.dir, args.c.media.event, args.flags & PL_FOWN,
                       args.flags & PL_FFAV, PL_ARGFMT (args.flags))
            != PL_EOK)
          {
            return EXIT_FAILURE;
          };
        break;
      }
    case PL_CDOWNLOAD:
      {
        if (plDoDownload (args.dir, args.c.download.event,
                          args.c.download.media, args.c.download.target,
                          args.flags & PL_FOWN, args.flags & PL_FFAV)
            != PL_EOK)
          {
            return EXIT_FAILURE;
          };
        break;
      }
    case PL_CHELP:
      {
        if (plDoHelp () != PL_EOK)
          {
            return EXIT_FAILURE;
          };
        break;
      }
    case PL_CVERSION:
      {
        if (plDoVersion () != PL_EOK)
          {
            return EXIT_FAILURE;
          };
        break;
      }
    default:
      PL_ERROR ("Unknown command");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
