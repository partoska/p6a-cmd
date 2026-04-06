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
#include "api.h"
#include "fs.h"
#include "ini.h"
#include "logger.h"
#include "types.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

static PLBool
plIsCharValid (PLByte c)
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

static void
plSanitizeDirName (PLChar *dst, PLSize size, const PLChar *src)
{
  PLSize j = 0;
  PLBool space = PL_FALSE;
  for (PLSize i = 0; src[i] != '\0' && j < size - 1; ++i)
    {
      PLByte c = (PLByte)src[i];

      if (!plIsCharValid (c))
        {
          continue;
        }

      if (c == ' ')
        {
          if (!space && j > 0)
            {
              dst[j++] = ' ';
              space = PL_TRUE;
            }
        }
      else
        {
          dst[j++] = src[i];
          space = PL_FALSE;
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
      strncpy (dst, "event", size - 1);
      dst[size - 1] = '\0';
    }
}

static void
plFormatDirName (PLChar *dst, PLSize size, const PLEvent *event)
{
  struct tm tm;
  memset (&tm, 0, sizeof (tm));
  if (sscanf (event->from, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday)
      == 3)
    {
      tm.tm_year -= 1900;
      tm.tm_mon -= 1;
    }

  PLChar sanitized[64];
  plSanitizeDirName (sanitized, sizeof (sanitized), event->name);

  snprintf (dst, size - 1, "%04d-%02d-%02d %s", tm.tm_year + 1900,
            tm.tm_mon + 1, tm.tm_mday, sanitized);
  dst[size - 1] = '\0';
}

static PLChar *
plIdentifyDir (const PLChar *path)
{
  PLChar ipath[MAX_PATH + 64];
  snprintf (ipath, PL_CHARSMAX (ipath), "%s/%s", path, EVENT_INI_FILE);
  ipath[PL_CHARSMAX (ipath)] = '\0';
  if (!plIsFile (ipath))
    {
      return NULL;
    }

  PLIni *ini = plIniInit ();
  if (!ini)
    {
      return NULL;
    }

  if (plIniLoad (ini, ipath) < 0)
    {
      plIniDestroy (ini);
      return NULL;
    }

  const PLChar *uuid = plIniGet (ini, "global", "UUID");
  PLChar *result = NULL;
  if (uuid)
    {
      result = strdup (uuid);
    }

  plIniDestroy (ini);

  return result;
}

static PLInt
plSyncMedia (PLIni *ini, const PLChar *dir, const PLChar *base,
             const PLChar *bearer, const PLChar *event, const PLMedia *media)
{
  PLChar section[MAX_SECTION];
  snprintf (section, PL_CHARSMAX (section), "media:%s", media->id);
  section[PL_CHARSMAX (section)] = '\0';
  const PLChar *name = plIniGet (ini, section, "NAME");
  if (name != NULL)
    {
      PL_DSLOW ("Media found in INI: %s (%s)", media->id, name);

      PLChar path[MAX_PATH];
      snprintf (path, PL_CHARSMAX (path), "%s/%s", dir, name);
      path[PL_CHARSMAX (path)] = '\0';
      if (plIsFile (path))
        {
          PL_DSLOW ("Media file already exists: %s", path);
          return PL_EOK;
        }
    }

  const PLChar *pre = "";
  const PLChar *ext = "";
  if (strstr (media->type, "image/jpeg"))
    {
      pre = "IMG";
      ext = ".jpg";
    }
  else if (strstr (media->type, "video/mp4"))
    {
      pre = "MOV";
      ext = ".mp4";
    }
  else if (strstr (media->type, "video/quicktime"))
    {
      pre = "MOV";
      ext = ".mov";
    }
  else
    {
      PL_ERROR ("Unsupported media type: %s", media->type);
      return PL_EARG;
    }

  PLChar candidate[MAX_FILENAME];
  PLBool found = 0;
  for (PLInt n = 1; n < 10000; ++n)
    {
      snprintf (candidate, PL_CHARSMAX (candidate), "%s_%04d%s", pre, n, ext);
      candidate[PL_CHARSMAX (candidate)] = '\0';

      PLBool occupied = 0;
      for (PLSize i = 0; i < ini->count; ++i)
        {
          if (ini->entries[i].value
              && strcmp (ini->entries[i].key, "NAME") == 0
              && strcmp (ini->entries[i].value, candidate) == 0)
            {
              occupied = 1;
              break;
            }
        }

      if (!occupied)
        {
          found = 1;
          break;
        }
    }

  if (!found)
    {
      PL_ERROR ("Could not find a new name for: %s", media->id);
      return PL_EFS;
    }

  if (name == NULL)
    {
      name = candidate;
    }

  PLChar path[MAX_PATH];
  snprintf (path, PL_CHARSMAX (path), "%s/%s", dir, name);
  path[PL_CHARSMAX (path)] = '\0';

  PL_DSLOW ("Fetching media file: %s", name);
  PLInt result = plApiMediaFetch (base, bearer, event, media->id, path);
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to fetch media: %s", media->id);
      return result;
    }

  result = plIniSet (ini, section, "NAME", name);
  if (result != PL_EOK)
    {
      return result;
    }

  result = plIniSet (ini, section, "TYPE", media->type);
  if (result != PL_EOK)
    {
      return result;
    }

  return PL_EOK;
}

static PLInt
plResolveDirName (PLChar *path, PLSize pathsize, const PLChar *root,
                  const PLChar *name)
{
  snprintf (path, pathsize - 1, "%s/%s", root, name);
  path[pathsize - 1] = '\0';
  if (!plIsDir (path))
    {
      return PL_EOK;
    }

  for (PLInt n = 1; n < 10000; ++n)
    {
      snprintf (path, pathsize - 1, "%s/%s %d", root, name, n);
      path[pathsize - 1] = '\0';
      if (!plIsDir (path))
        {
          return PL_EOK;
        }
    }

  PL_ERROR ("Could not resolve unique directory name for: %s", name);
  return PL_EFS;
}

static PLInt
plSyncEventCreate (const PLChar *root, const PLChar *base,
                   const PLChar *bearer, const PLEvent *event,
                   const PLMediaList *media)
{
  if (!root || !event || !base || !bearer || !event || !media)
    {
      PL_DEBUG ("Invalid arguments");
      return PL_EARG;
    }

  PLChar name[MAX_DIRNAME];
  plFormatDirName (name, PL_CHARSMAX (name), event);
  name[PL_CHARSMAX (name)] = '\0';

  PLChar path[MAX_PATH];
  PLInt result = plResolveDirName (path, sizeof (path), root, name);
  if (result != PL_EOK)
    {
      return result;
    }
  result = plCreateDir (path);
  if (result < 0)
    {
      PL_ERROR ("Could not create directory: %s", path);
      return result;
    }

  PLIni *ini = plIniInit ();
  if (!ini)
    {
      return PL_EMEM;
    }

  result = plIniSet (ini, "global", "UUID", event->id);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  result = plIniSet (ini, "global", "NAME", event->name);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  PLChar tsync[MAX_TIME];
  snprintf (tsync, sizeof (tsync), "%lld", (PLLong)time (NULL));
  tsync[PL_CHARSMAX (tsync)] = '\0';
  result = plIniSet (ini, "global", "SYNC", tsync);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  PLChar ipath[MAX_PATH + 64];
  snprintf (ipath, PL_CHARSMAX (ipath), "%s/%s", path, EVENT_INI_FILE);
  ipath[PL_CHARSMAX (ipath)] = '\0';
  // Save INI immediately after we have fetched event data.
  result = plIniSave (ini, ipath);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  for (PLSize i = 0; i < media->count; ++i)
    {
      PLMedia *m = &media->media[i];
      result = plSyncMedia (ini, path, base, bearer, event->id, m);
      if (result != PL_EOK)
        {
          plIniDestroy (ini);
          return result;
        }
    }

  // Save INI once again after fetching the media.
  result = plIniSave (ini, ipath);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  plIniDestroy (ini);
  return PL_EOK;
}

static PLInt
plSyncEventRename (const PLChar *root, const PLChar *dir, const PLChar *base,
                   const PLChar *bearer, const PLEvent *event,
                   const PLMediaList *media)
{
  if (!root || !dir || !base || !bearer || !event || !media)
    {
      PL_DEBUG ("Invalid arguments");
      return PL_EARG;
    }

  PLChar path[MAX_PATH];
  snprintf (path, PL_CHARSMAX (path), "%s/%s", root, dir);
  path[PL_CHARSMAX (path)] = '\0';

  PLChar ndir[MAX_DIRNAME];
  plFormatDirName (ndir, PL_CHARSMAX (ndir), event);
  ndir[PL_CHARSMAX (ndir)] = '\0';

  PLChar npath[MAX_PATH];
  snprintf (npath, PL_CHARSMAX (npath), "%s/%s", root, ndir);
  npath[PL_CHARSMAX (npath)] = '\0';

  PLSize ndirlen = strlen (ndir);
  PLSize dirlen = strlen (dir);
  PLBool prefixed = dirlen >= ndirlen && strncmp (dir, ndir, ndirlen) == 0
                    && (dirlen == ndirlen || dir[ndirlen] == ' ');
  PLInt renaming = !prefixed;
  if (renaming)
    {
      if (plIsDir (npath))
        {
          PLInt rr = plResolveDirName (npath, sizeof (npath), root, ndir);
          if (rr != PL_EOK)
            {
              return rr;
            }
        }
      PL_DSLOW ("Renaming %s -> %s", path, npath);
      if (rename (path, npath) < 0)
        {
          return PL_EFS;
        }
    }

  PLIni *ini = plIniInit ();
  if (!ini)
    {
      return PL_EMEM;
    }

  PLChar ipath[MAX_PATH + 64];
  snprintf (ipath, PL_CHARSMAX (ipath), "%s/%s", renaming ? npath : path,
            EVENT_INI_FILE);
  ipath[PL_CHARSMAX (ipath)] = '\0';
  PLInt result = plIniLoad (ini, ipath);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  result = plIniSet (ini, "global", "NAME", event->name);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  PLChar tsync[MAX_TIME];
  snprintf (tsync, PL_CHARSMAX (tsync), "%lld", (PLLong)time (NULL));
  tsync[PL_CHARSMAX (tsync)] = '\0';
  // Save INI immediately after we have fetched event data.
  result = plIniSave (ini, ipath);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  for (PLSize i = 0; i < media->count; ++i)
    {
      PLMedia *m = &media->media[i];
      result = plSyncMedia (ini, renaming ? npath : path, base, bearer,
                            event->id, m);
      if (result != PL_EOK)
        {
          plIniDestroy (ini);
          return result;
        }
    }

  // Save INI once again after fetching the media.
  result = plIniSave (ini, ipath);
  if (result != PL_EOK)
    {
      plIniDestroy (ini);
      return result;
    }

  plIniDestroy (ini);
  return PL_EOK;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PLInt
plSync (const PLChar *base, const PLChar *bearer, const PLChar *target,
        PLBool owner, PLBool favorite)
{
  if (!base)
    {
      PL_ERROR ("Invalid endpoint");
      return PL_EARG;
    }

  if (!bearer)
    {
      PL_ERROR ("Invalid authorization");
      return PL_EARG;
    }

  if (!target)
    {
      PL_ERROR ("Invalid target directory");
      return PL_EARG;
    }

  if (!plIsDir (target))
    {
      PL_ERROR ("Target is not a directory");
      return PL_EFS;
    }

  PLEventList *events = plApiEventList (base, bearer, NULL);
  if (!events)
    {
      PL_ERROR ("Failed to fetch events");
      return PL_ENET;
    }

  PLBool *matched = (PLBool *)calloc (events->count, sizeof (PLBool));
  if (!matched)
    {
      PL_ERROR ("Out of memory");
      plFreeEventList (events);
      return PL_EMEM;
    }

  PLDir *dir = plOpenDir (target);
  if (!dir)
    {
      PL_ERROR ("Failed to open target directory");
      free (matched);
      plFreeEventList (events);
      return PL_EFS;
    }

  const PLChar *dname;
  while ((dname = plReadDir (dir)) != NULL)
    {
      if (strcmp (dname, ".") == 0 || strcmp (dname, "..") == 0)
        {
          // Skiping virtual dirs is expected ...
          continue;
        }

      PLChar subdir[MAX_PATH];
      snprintf (subdir, PL_CHARSMAX (subdir), "%s/%s", target, dname);
      subdir[PL_CHARSMAX (subdir)] = '\0';
      if (!plIsDir (subdir))
        {
          PL_WARN ("Skipping dir: %s", subdir);
          continue;
        }

      PLChar *uuid = plIdentifyDir (subdir);
      if (!uuid)
        {
          PL_WARN ("No UUID found in: %s", subdir);
          continue;
        }

      PL_DEBUG ("--- Sync Existing Event ---");
      PL_DEBUG ("Dir: %s", subdir);
      PLBool found = PL_FALSE;
      for (PLSize i = 0; i < events->count; ++i)
        {
          PLEvent *e = &events->events[i];
          if (strcmp (uuid, e->id) != 0)
            {
              continue;
            }

          PL_DEBUG ("Event: %s", e->name);
          matched[i] = PL_TRUE;
          found = PL_TRUE;

          PLMediaList *media = plApiMediaList (base, bearer, e->id);
          if (!media)
            {
              PL_ERROR ("Failed to fetch media for: %s", e->name);
              free (uuid);
              plCloseDir (dir);
              free (matched);
              plFreeEventList (events);
              return PL_ENET;
            }

          PLInt result
              = plSyncEventRename (target, dname, base, bearer, e, media);
          if (result != PL_EOK)
            {
              plFreeMediaList (media);
              free (uuid);
              plCloseDir (dir);
              free (matched);
              plFreeEventList (events);
              return result;
            }

          plFreeMediaList (media);
          break;
        }

      if (!found)
        {
          PL_DEBUG ("No matching online event");
        }

      free (uuid);

      PL_DEBUG ("--- Sync Done ---");
    }

  plCloseDir (dir);

  for (PLSize i = 0; i < events->count; ++i)
    {
      if (matched[i])
        {
          continue;
        }

      PLEvent *e = &events->events[i];
      PL_DEBUG ("--- Sync New Event ---");
      PL_DEBUG ("Event: %s", e->name);

      if (owner && !e->owner)
        {
          PL_DEBUG ("Skipping non-owner event");
          continue;
        }

      if (favorite && !e->favorite)
        {
          PL_DEBUG ("Skipping non-favorite event");
          continue;
        }

      PLMediaList *media = plApiMediaList (base, bearer, e->id);
      if (!media)
        {
          PL_ERROR ("Failed to fetch media for: %s", e->name);
          free (matched);
          plFreeEventList (events);
          return PL_ENET;
        }

      PLInt result = plSyncEventCreate (target, base, bearer, e, media);
      if (result != PL_EOK)
        {
          plFreeMediaList (media);
          free (matched);
          plFreeEventList (events);
          return result;
        }

      plFreeMediaList (media);

      PL_DEBUG ("--- Sync Done ---");
    }

  free (matched);
  plFreeEventList (events);

  return PL_EOK;
}
