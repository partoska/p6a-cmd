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

#include "sync.h"
#include "ini.h"
#include "log.h"
#include "partoska.h"
#include "pktypes.h"
#include "plat.h"
#include "report.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define EVENT_INI_FILE ".event.p6a.ini"
#define MAX_PATH (1024)
#define MAX_DIRNAME (128)
#define MAX_TIME (32)
#define MAX_FILENAME (64)
#define MAX_SECTION (128)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKBool
pkcIsCharValid (PKByte c)
{
  /* Reject characters forbidden in Windows directory names: \ / : * ? " < > |
   * Also reject control characters (0x00-0x1F). Allow all other printable
   * ASCII and high bytes (UTF-8 multibyte sequences). */
  if (c < 0x20)
    {
      return 0;
    }

  if (c & 0x80)
    {
      return 1;
    }

  switch (c)
    {
    case '\\':
    case '/':
    case ':':
    case '*':
    case '?':
    case '"':
    case '<':
    case '>':
    case '|':
      return 0;

    default:
      return 1;
    }
}

static PKVoid
pkcSanitizeDirName (PKChar *dst, PKSize size, const PKChar *src)
{
  PKSize j = 0;
  PKBool space = PK_FALSE;
  for (PKSize i = 0; src[i] != '\0' && j < size - 1; ++i)
    {
      PKByte c = (PKByte)src[i];

      if (!pkcIsCharValid (c))
        {
          continue;
        }

      if (c == ' ')
        {
          if (!space && j > 0)
            {
              dst[j++] = ' ';
              space = PK_TRUE;
            }
        }
      else
        {
          dst[j++] = src[i];
          space = PK_FALSE;
        }
    }

  dst[j] = '\0';

  while (j > 0 && dst[j - 1] == ' ')
    {
      --j;
      dst[j] = '\0';
    }

  if (j == 0)
    {
      pkcStrFmt (dst, size, "%s", "event");
    }
}

static PKVoid
pkcFormatDirName (PKChar *dst, PKSize size, const PKEvent *event)
{
  // Without a start date, as before: "1900-01-00".
  PKInt year, month, day;
  if (!pkcDateParse (event->from, &year, &month, &day))
    {
      year = 1900;
      month = 1;
      day = 0;
    }

  PKChar sanitized[64];
  pkcSanitizeDirName (sanitized, sizeof (sanitized), event->name);

  pkcStrFmt (dst, size - 1, "%04d-%02d-%02d %s", year, month, day, sanitized);
  dst[size - 1] = '\0';
}

static PKChar *
pkcIdentifyDir (const PKChar *path)
{
  PKChar ipath[MAX_PATH + 64];
  pkcStrFmt (ipath, PK_CHARSMAX (ipath), "%s/%s", path, EVENT_INI_FILE);
  ipath[PK_CHARSMAX (ipath)] = '\0';
  if (!pkcIsFile (ipath))
    {
      return NULL;
    }

  PKCIni *ini = pkcIniInit ();
  if (!ini)
    {
      return NULL;
    }

  if (pkcIniLoad (ini, ipath) < 0)
    {
      pkcIniDestroy (ini);
      return NULL;
    }

  const PKChar *uuid = pkcIniGet (ini, "global", "UUID");
  PKChar *result = NULL;
  if (uuid)
    {
      result = pkcStrDup (uuid, pkcStrLen (uuid));
    }

  pkcIniDestroy (ini);

  return result;
}

static PKInt
pkcSyncMedia (PKCIni *ini, const PKChar *dir, PKClient *client,
              const PKChar *event, const PKMedia *media)
{
  PKChar section[MAX_SECTION];
  pkcStrFmt (section, PK_CHARSMAX (section), "media:%s", media->id);
  section[PK_CHARSMAX (section)] = '\0';
  const PKChar *name = pkcIniGet (ini, section, "NAME");
  if (name != NULL)
    {
      PKC_DSLOW ("Media found in INI: %s (%s)", media->id, name);

      PKChar path[MAX_PATH];
      pkcStrFmt (path, PK_CHARSMAX (path), "%s/%s", dir, name);
      path[PK_CHARSMAX (path)] = '\0';
      if (pkcIsFile (path))
        {
          PKC_DSLOW ("Media file already exists: %s", path);
          return PK_OK;
        }
    }

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
  PKBool found = 0;
  for (PKInt n = 1; n < 10000; ++n)
    {
      pkcStrFmt (candidate, PK_CHARSMAX (candidate), "%s_%04d%s", pre, n, ext);
      candidate[PK_CHARSMAX (candidate)] = '\0';

      if (!pkcIniHasValue (ini, "NAME", candidate))
        {
          found = 1;
          break;
        }
    }

  if (!found)
    {
      PKC_ERROR ("Could not find a new name for: %s", media->id);
      return PK_EFS;
    }

  if (name == NULL)
    {
      name = candidate;
    }

  PKChar path[MAX_PATH];
  pkcStrFmt (path, PK_CHARSMAX (path), "%s/%s", dir, name);
  path[PK_CHARSMAX (path)] = '\0';

  PKC_DSLOW ("Fetching media file: %s", name);
  PKInt result = pkMediaDownload (client, event, media->id, path);
  if (result != PK_OK)
    {
      pkcReport (client, "Failed to fetch media");
      return result;
    }

  result = pkcIniSet (ini, section, "NAME", name);
  if (result != PK_OK)
    {
      return result;
    }

  result = pkcIniSet (ini, section, "TYPE", media->type);
  if (result != PK_OK)
    {
      return result;
    }

  return PK_OK;
}

static PKInt
pkcResolveDirName (PKChar *path, PKSize pathsize, const PKChar *root,
                   const PKChar *name)
{
  pkcStrFmt (path, pathsize - 1, "%s/%s", root, name);
  path[pathsize - 1] = '\0';
  if (!pkcIsDir (path))
    {
      return PK_OK;
    }

  for (PKInt n = 1; n < 10000; ++n)
    {
      pkcStrFmt (path, pathsize - 1, "%s/%s %d", root, name, n);
      path[pathsize - 1] = '\0';
      if (!pkcIsDir (path))
        {
          return PK_OK;
        }
    }

  PKC_ERROR ("Could not resolve unique directory name for: %s", name);
  return PK_EFS;
}

static PKInt
pkcSyncEventCreate (const PKChar *root, PKClient *client, const PKEvent *event,
                    const PKMediaList *media)
{
  if (!root || !event || !client || !event || !media)
    {
      PKC_DEBUG ("Invalid arguments");
      return PK_EARG;
    }

  PKChar name[MAX_DIRNAME];
  pkcFormatDirName (name, PK_CHARSMAX (name), event);
  name[PK_CHARSMAX (name)] = '\0';

  PKChar path[MAX_PATH];
  PKInt result = pkcResolveDirName (path, sizeof (path), root, name);
  if (result != PK_OK)
    {
      return result;
    }
  result = pkcCreateDir (path);
  if (result < 0)
    {
      PKC_ERROR ("Could not create directory: %s", path);
      return result;
    }

  PKCIni *ini = pkcIniInit ();
  if (!ini)
    {
      return PK_EMEM;
    }

  result = pkcIniSet (ini, "global", "UUID", event->id);
  if (result != PK_OK)
    {
      pkcIniDestroy (ini);
      return result;
    }

  result = pkcIniSet (ini, "global", "NAME", event->name);
  if (result != PK_OK)
    {
      pkcIniDestroy (ini);
      return result;
    }

  PKChar tsync[MAX_TIME];
  pkcStrFmt (tsync, sizeof (tsync), "%ld", (PKLong)pkcTimeNow ());
  tsync[PK_CHARSMAX (tsync)] = '\0';
  result = pkcIniSet (ini, "global", "SYNC", tsync);
  if (result != PK_OK)
    {
      pkcIniDestroy (ini);
      return result;
    }

  PKChar ipath[MAX_PATH + 64];
  pkcStrFmt (ipath, PK_CHARSMAX (ipath), "%s/%s", path, EVENT_INI_FILE);
  ipath[PK_CHARSMAX (ipath)] = '\0';
  // Save INI immediately after we have fetched event data.
  result = pkcIniSave (ini, ipath);
  if (result != PK_OK)
    {
      pkcIniDestroy (ini);
      return result;
    }

  for (PKSize i = 0; i < media->count; ++i)
    {
      PKMedia *m = &media->media[i];
      result = pkcSyncMedia (ini, path, client, event->id, m);
      if (result != PK_OK)
        {
          pkcIniDestroy (ini);
          return result;
        }
    }

  // Save INI once again after fetching the media.
  result = pkcIniSave (ini, ipath);
  if (result != PK_OK)
    {
      pkcIniDestroy (ini);
      return result;
    }

  pkcIniDestroy (ini);
  return PK_OK;
}

static PKInt
pkcSyncEventRename (const PKChar *root, const PKChar *dir, PKClient *client,
                    const PKEvent *event, const PKMediaList *media)
{
  if (!root || !dir || !client || !event || !media)
    {
      PKC_DEBUG ("Invalid arguments");
      return PK_EARG;
    }

  PKChar path[MAX_PATH];
  pkcStrFmt (path, PK_CHARSMAX (path), "%s/%s", root, dir);
  path[PK_CHARSMAX (path)] = '\0';

  PKChar ndir[MAX_DIRNAME];
  pkcFormatDirName (ndir, PK_CHARSMAX (ndir), event);
  ndir[PK_CHARSMAX (ndir)] = '\0';

  PKChar npath[MAX_PATH];
  pkcStrFmt (npath, PK_CHARSMAX (npath), "%s/%s", root, ndir);
  npath[PK_CHARSMAX (npath)] = '\0';

  PKSize ndirlen = pkcStrLen (ndir);
  PKSize dirlen = pkcStrLen (dir);
  PKBool prefixed = dirlen >= ndirlen && pkcStrEqN (dir, ndir, ndirlen)
                    && (dirlen == ndirlen || dir[ndirlen] == ' ');
  PKInt renaming = !prefixed;
  if (renaming)
    {
      if (pkcIsDir (npath))
        {
          PKInt rr = pkcResolveDirName (npath, sizeof (npath), root, ndir);
          if (rr != PK_OK)
            {
              return rr;
            }
        }
      PKC_DSLOW ("Renaming %s -> %s", path, npath);
      if (!pkcFileRename (path, npath))
        {
          return PK_EFS;
        }
    }

  PKCIni *ini = pkcIniInit ();
  if (!ini)
    {
      return PK_EMEM;
    }

  PKChar ipath[MAX_PATH + 64];
  pkcStrFmt (ipath, PK_CHARSMAX (ipath), "%s/%s", renaming ? npath : path,
             EVENT_INI_FILE);
  ipath[PK_CHARSMAX (ipath)] = '\0';
  PKInt result = pkcIniLoad (ini, ipath);
  if (result != PK_OK)
    {
      pkcIniDestroy (ini);
      return result;
    }

  result = pkcIniSet (ini, "global", "NAME", event->name);
  if (result != PK_OK)
    {
      pkcIniDestroy (ini);
      return result;
    }

  PKChar tsync[MAX_TIME];
  pkcStrFmt (tsync, PK_CHARSMAX (tsync), "%ld", (PKLong)pkcTimeNow ());
  tsync[PK_CHARSMAX (tsync)] = '\0';
  result = pkcIniSet (ini, "global", "SYNC", tsync);
  if (result != PK_OK)
    {
      pkcIniDestroy (ini);
      return result;
    }

  // Save INI immediately after we have fetched event data.
  result = pkcIniSave (ini, ipath);
  if (result != PK_OK)
    {
      pkcIniDestroy (ini);
      return result;
    }

  for (PKSize i = 0; i < media->count; ++i)
    {
      PKMedia *m = &media->media[i];
      result
          = pkcSyncMedia (ini, renaming ? npath : path, client, event->id, m);
      if (result != PK_OK)
        {
          pkcIniDestroy (ini);
          return result;
        }
    }

  // Save INI once again after fetching the media.
  result = pkcIniSave (ini, ipath);
  if (result != PK_OK)
    {
      pkcIniDestroy (ini);
      return result;
    }

  pkcIniDestroy (ini);
  return PK_OK;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKInt
pkcSync (PKClient *client, const PKChar *target, PKBool owner, PKBool favorite)
{
  if (!client)
    {
      PKC_ERROR ("Invalid client");
      return PK_EARG;
    }

  if (!target)
    {
      PKC_ERROR ("Invalid target directory");
      return PK_EARG;
    }

  if (!pkcIsDir (target))
    {
      PKC_ERROR ("Target is not a directory");
      return PK_EFS;
    }

  PKEventList events;
  if (pkEventList (client, NULL, &events) != PK_OK)
    {
      pkcReport (client, "Failed to fetch events");
      return pkError (client)->status;
    }

  PKBool *matched = pkcMemZalloc (events.count * sizeof (PKBool));
  if (!matched)
    {
      PKC_ERROR ("Out of memory");
      pkEventListFree (&events);
      return PK_EMEM;
    }

  PKCDir *dir = pkcOpenDir (target);
  if (!dir)
    {
      PKC_ERROR ("Failed to open target directory");
      pkcMemFree (matched);
      pkEventListFree (&events);
      return PK_EFS;
    }

  const PKChar *dname;
  while ((dname = pkcReadDir (dir)) != NULL)
    {
      if (pkcStrEq (dname, ".") || pkcStrEq (dname, ".."))
        {
          // Skiping virtual dirs is expected ...
          continue;
        }

      PKChar subdir[MAX_PATH];
      pkcStrFmt (subdir, PK_CHARSMAX (subdir), "%s/%s", target, dname);
      subdir[PK_CHARSMAX (subdir)] = '\0';
      if (!pkcIsDir (subdir))
        {
          PKC_WARN ("Skipping dir: %s", subdir);
          continue;
        }

      PKChar *uuid = pkcIdentifyDir (subdir);
      if (!uuid)
        {
          PKC_WARN ("No UUID found in: %s", subdir);
          continue;
        }

      PKC_DEBUG ("--- Sync Existing Event ---");
      PKC_DEBUG ("Dir: %s", subdir);
      PKBool found = PK_FALSE;
      for (PKSize i = 0; i < events.count; ++i)
        {
          PKEvent *e = &events.events[i];
          if (!pkcStrEq (uuid, e->id))
            {
              continue;
            }

          PKC_DEBUG ("Event: %s", e->name);
          matched[i] = PK_TRUE;
          found = PK_TRUE;

          PKMediaList media;
          if (pkMediaList (client, e->id, &media) != PK_OK)
            {
              pkcReport (client, "Failed to fetch media");
              pkcMemFree (uuid);
              pkcCloseDir (dir);
              pkcMemFree (matched);
              pkEventListFree (&events);
              return pkError (client)->status;
            }

          PKInt result = pkcSyncEventRename (target, dname, client, e, &media);
          if (result != PK_OK)
            {
              pkMediaListFree (&media);
              pkcMemFree (uuid);
              pkcCloseDir (dir);
              pkcMemFree (matched);
              pkEventListFree (&events);
              return result;
            }

          pkMediaListFree (&media);
          break;
        }

      if (!found)
        {
          PKC_DEBUG ("No matching online event");
        }

      pkcMemFree (uuid);

      PKC_DEBUG ("--- Sync Done ---");
    }

  pkcCloseDir (dir);

  for (PKSize i = 0; i < events.count; ++i)
    {
      if (matched[i])
        {
          continue;
        }

      PKEvent *e = &events.events[i];
      PKC_DEBUG ("--- Sync New Event ---");
      PKC_DEBUG ("Event: %s", e->name);

      if (owner && !e->owner)
        {
          PKC_DEBUG ("Skipping non-owner event");
          continue;
        }

      if (favorite && !e->favorite)
        {
          PKC_DEBUG ("Skipping non-favorite event");
          continue;
        }

      PKMediaList media;
      if (pkMediaList (client, e->id, &media) != PK_OK)
        {
          pkcReport (client, "Failed to fetch media");
          pkcMemFree (matched);
          pkEventListFree (&events);
          return pkError (client)->status;
        }

      PKInt result = pkcSyncEventCreate (target, client, e, &media);
      if (result != PK_OK)
        {
          pkMediaListFree (&media);
          pkcMemFree (matched);
          pkEventListFree (&events);
          return result;
        }

      pkMediaListFree (&media);

      PKC_DEBUG ("--- Sync Done ---");
    }

  pkcMemFree (matched);
  pkEventListFree (&events);

  return PK_OK;
}
