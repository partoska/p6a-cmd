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

#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <dirent.h>
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef _WIN32
typedef struct _stat PLStat;
#define plIsDirStat(m) (((m) & _S_IFMT) == _S_IFDIR)
#define plIsRegStat(m) (((m) & _S_IFMT) == _S_IFREG)

static wchar_t *
plToWide (const PLChar *utf8)
{
  PLInt len = MultiByteToWideChar (CP_UTF8, 0, utf8, -1, NULL, 0);
  if (len <= 0)
    {
      return NULL;
    }

  wchar_t *wide = malloc ((size_t)len * sizeof (wchar_t));
  if (!wide)
    {
      return NULL;
    }

  MultiByteToWideChar (CP_UTF8, 0, utf8, -1, wide, len);
  return wide;
}

static PLInt
plStat (const PLChar *path, PLStat *st)
{
  wchar_t *wide = plToWide (path);
  if (!wide)
    {
      return PL_EFS;
    }

  PLInt r = _wstat (wide, st);
  free (wide);
  return r;
}

static PLInt
plMkdir (const PLChar *path)
{
  wchar_t *wide = plToWide (path);
  if (!wide)
    {
      return PL_EFS;
    }

  PLInt r = _wmkdir (wide);
  free (wide);

  return r;
}

static PLChar *
plFromWide (const wchar_t *wide, PLChar *buf, PLSize size)
{
  if (size > 0x7FFFFFFF)
    {
      return NULL;
    }

  PLInt len
      = WideCharToMultiByte (CP_UTF8, 0, wide, -1, buf, (INT)size, NULL, NULL);
  if (len <= 0)
    {
      return NULL;
    }

  return buf;
}

struct PLDir
{
  HANDLE handle;
  WIN32_FIND_DATAW ffd;
  PLBool first;
  PLChar name[1024];
};
#else
typedef struct stat PLStat;
#define plIsDirStat(m) S_ISDIR (m)
#define plIsRegStat(m) S_ISREG (m)
#define plStat(path, st) stat (path, st)
#define plMkdir(path) mkdir (path, 0755)

struct PLDir
{
  DIR *d;
};
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PLBool
plIsDir (const PLChar *path)
{
  PLStat st;
  if (plStat (path, &st) != 0)
    {
      return PL_FALSE;
    }

  return plIsDirStat (st.st_mode) ? PL_TRUE : PL_FALSE;
}

PLBool
plIsFile (const PLChar *path)
{
  PLStat st;
  if (plStat (path, &st) != 0)
    {
      return PL_FALSE;
    }

  return plIsRegStat (st.st_mode) ? PL_TRUE : PL_FALSE;
}

PLFile *
plFileOpen (const PLChar *path, const PLChar *mode)
{
#ifdef _WIN32
  wchar_t *wpath = plToWide (path);
  if (!wpath)
    {
      return NULL;
    }

  wchar_t *wmode = plToWide (mode);
  if (!wmode)
    {
      free (wpath);
      return NULL;
    }

  PLFile *f = _wfopen (wpath, wmode);
  free (wpath);
  free (wmode);

  return f;
#else
  return fopen (path, mode);
#endif
}

void
plFileClose (PLFile *file)
{
  fclose (file);
}

void
plConsoleInit (void)
{
#ifdef _WIN32
  SetConsoleOutputCP (CP_UTF8);
#endif
}

PLInt
plChmod600 (const PLChar *path)
{
#ifdef _WIN32
  if (_chmod (path, _S_IREAD | _S_IWRITE) != 0)
    {
      return PL_EFS;
    }
#else
  if (chmod (path, 0600) != 0)
    {
      return PL_EFS;
    }
#endif
  return PL_EOK;
}

PLInt
plCreateDir (const PLChar *path)
{
  if (plIsDir (path))
    {
      // Indicates that directory already exists.
      return 1;
    }
  if (plMkdir (path) < 0)
    {
      return PL_EFS;
    }

  return PL_EOK;
}

PLDir *
plOpenDir (const PLChar *path)
{
#ifdef _WIN32
  PLChar pattern[1024 + 3];
  snprintf (pattern, sizeof (pattern) - 1, "%s/*", path);
  pattern[sizeof (pattern) - 1] = '\0';

  wchar_t *wpattern = plToWide (pattern);
  if (!wpattern)
    {
      return NULL;
    }

  PLDir *dir = malloc (sizeof (PLDir));
  if (!dir)
    {
      free (wpattern);
      return NULL;
    }

  dir->handle = FindFirstFileW (wpattern, &dir->ffd);
  free (wpattern);

  if (dir->handle == INVALID_HANDLE_VALUE)
    {
      free (dir);
      return NULL;
    }

  dir->first = PL_TRUE;
  return dir;
#else
  DIR *d = opendir (path);
  if (!d)
    {
      return NULL;
    }

  PLDir *dir = malloc (sizeof (PLDir));
  if (!dir)
    {
      closedir (d);
      return NULL;
    }

  dir->d = d;
  return dir;
#endif
}

const PLChar *
plReadDir (PLDir *dir)
{
#ifdef _WIN32
  if (dir->first)
    {
      dir->first = PL_FALSE;
    }
  else
    {
      if (!FindNextFileW (dir->handle, &dir->ffd))
        {
          return NULL;
        }
    }

  if (!plFromWide (dir->ffd.cFileName, dir->name, sizeof (dir->name)))
    {
      return NULL;
    }

  return dir->name;
#else
  struct dirent *entry = readdir (dir->d);
  if (!entry)
    {
      return NULL;
    }

  return entry->d_name;
#endif
}

void
plCloseDir (PLDir *dir)
{
#ifdef _WIN32
  if (dir->handle != INVALID_HANDLE_VALUE)
    {
      FindClose (dir->handle);
    }
#else
  closedir (dir->d);
#endif
  free (dir);
}
