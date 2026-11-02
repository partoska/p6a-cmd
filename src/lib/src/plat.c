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

#include "plat.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Memory
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKVoid *
pkMemZalloc (PKSize size)
{
  return calloc (1, size ? size : 1);
}

PKVoid *
pkMemZallocArray (PKSize count, PKSize size)
{
  return calloc (count ? count : 1, size ? size : 1);
}

PKVoid *
pkMemRealloc (PKVoid *ptr, PKSize size)
{
  return realloc (ptr, size ? size : 1);
}

PKVoid
pkMemFree (PKVoid *ptr)
{
  free (ptr);
}

PKVoid
pkMemCopy (PKVoid *dst, const PKVoid *src, PKSize size)
{
  memcpy (dst, src, size);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Strings
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKInt
pkLower (PKChar c)
{
  return c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : (PKByte)c;
}

PKSize
pkStrLen (const PKChar *str)
{
  return strlen (str);
}

PKInt
pkStrCmp (const PKChar *a, const PKChar *b, PKBool icase)
{
  if (!icase)
    {
      return strcmp (a, b);
    }

  // Portable ASCII folding: no strcasecmp() on Windows, no locale surprises.
  while (*a && pkLower (*a) == pkLower (*b))
    {
      ++a;
      ++b;
    }

  return pkLower (*a) - pkLower (*b);
}

PKInt
pkStrCmpN (const PKChar *a, const PKChar *b, PKSize n)
{
  return strncmp (a, b, n);
}

PKChar *
pkStrFind (const PKChar *str, PKChar c)
{
  return strchr (str, c);
}

PKChar *
pkStrFindLast (const PKChar *str, PKChar c)
{
  return strrchr (str, c);
}

PKChar *
pkStrFindAny (const PKChar *str, const PKChar *set)
{
  return strpbrk (str, set);
}

PKChar *
pkStrDup (const PKChar *str, PKSize length)
{
  PKChar *copy = malloc (length + 1);
  if (copy)
    {
      memcpy (copy, str, length);
      copy[length] = '\0';
    }

  return copy;
}

PKInt
pkStrFmt (PKChar *dst, PKSize size, const PKChar *fmt, ...)
{
  PKVaList args;
  PK_VA_START (args, fmt);
  PKInt n = vsnprintf (dst, size, fmt, args);
  PK_VA_END (args);

  return n;
}

PKInt
pkStrFmtV (PKChar *dst, PKSize size, const PKChar *fmt, PKVaList args)
{
  return vsnprintf (dst, size, fmt, args);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef _WIN32
typedef struct _stat PKStat;
#define pkIsDirStat(m) (((m) & _S_IFMT) == _S_IFDIR)
#define pkIsRegStat(m) (((m) & _S_IFMT) == _S_IFREG)

static wchar_t *
pkToWide (const PKChar *utf8)
{
  PKInt len = MultiByteToWideChar (CP_UTF8, 0, utf8, -1, NULL, 0);
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

static PKInt
pkStat (const PKChar *path, PKStat *st)
{
  wchar_t *wide = pkToWide (path);
  if (!wide)
    {
      return PK_EFS;
    }

  PKInt r = _wstat (wide, st);
  free (wide);
  return r;
}

static PKInt
pkMkdir (const PKChar *path)
{
  wchar_t *wide = pkToWide (path);
  if (!wide)
    {
      return PK_EFS;
    }

  PKInt r = _wmkdir (wide);
  free (wide);

  return r;
}

#else
typedef struct stat PKStat;
#define pkIsDirStat(m) S_ISDIR (m)
#define pkIsRegStat(m) S_ISREG (m)
#define pkStat(path, st) stat (path, st)
#define pkMkdir(path) mkdir (path, 0755)

#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKBool
pkIsDir (const PKChar *path)
{
  PKStat st;
  if (pkStat (path, &st) != 0)
    {
      return PK_FALSE;
    }

  return pkIsDirStat (st.st_mode) ? PK_TRUE : PK_FALSE;
}

PKBool
pkIsFile (const PKChar *path)
{
  PKStat st;
  if (pkStat (path, &st) != 0)
    {
      return PK_FALSE;
    }

  return pkIsRegStat (st.st_mode) ? PK_TRUE : PK_FALSE;
}

PKFile *
pkFileOpen (const PKChar *path, PKBool write)
{
#ifdef _WIN32
  wchar_t *wide = pkToWide (path);
  if (!wide)
    {
      errno = EINVAL;
      return NULL;
    }

  FILE *file = _wfopen (wide, write ? L"wb" : L"rb");
  free (wide);

  return (PKFile *)file;
#else
  return (PKFile *)fopen (path, write ? "wb" : "rb");
#endif
}

PKSize
pkFileRead (PKFile *file, PKVoid *data, PKSize size)
{
  return fread (data, 1, size, (FILE *)file);
}

PKSize
pkFileWrite (PKFile *file, const PKVoid *data, PKSize size)
{
  return fwrite (data, 1, size, (FILE *)file);
}

PKBool
pkFileFailed (PKFile *file)
{
  return ferror ((FILE *)file) != 0;
}

PKBool
pkFileSize (PKFile *file, PKLong *size)
{
  if (fseek ((FILE *)file, 0, SEEK_END) != 0)
    {
      return PK_FALSE;
    }

  *size = ftell ((FILE *)file);

  return *size >= 0 && fseek ((FILE *)file, 0, SEEK_SET) == 0;
}

PKBool
pkFileClose (PKFile *file)
{
  return fclose ((FILE *)file) == 0;
}

PKInt
pkChmod600 (const PKChar *path)
{
#ifdef _WIN32
  if (_chmod (path, _S_IREAD | _S_IWRITE) != 0)
    {
      return PK_EFS;
    }
#else
  if (chmod (path, 0600) != 0)
    {
      return PK_EFS;
    }
#endif
  return PK_OK;
}

PKInt
pkCreateDir (const PKChar *path)
{
  if (pkIsDir (path))
    {
      // Indicates that directory already exists.
      return 1;
    }
  if (pkMkdir (path) < 0)
    {
      return PK_EFS;
    }

  return PK_OK;
}

PKBool
pkFileRemove (const PKChar *path)
{
#ifdef _WIN32
  wchar_t *wide = pkToWide (path);
  if (!wide)
    {
      return PK_FALSE;
    }

  PKInt r = _wremove (wide);
  free (wide);

  return r == 0 ? PK_TRUE : PK_FALSE;
#else
  return remove (path) == 0 ? PK_TRUE : PK_FALSE;
#endif
}

PKVoid
pkSleep (PKLong ms)
{
#ifdef _WIN32
  Sleep ((DWORD)ms);
#else
  struct timespec ts;
  ts.tv_sec = (time_t)(ms / 1000);
  ts.tv_nsec = (long)(ms % 1000) * 1000000L;
  nanosleep (&ts, NULL);
#endif
}

PKInt
pkSysError (void)
{
  return errno;
}

const PKChar *
pkSysErrorText (PKInt code)
{
  return strerror (code);
}

const PKChar *
pkEnvGet (const PKChar *name)
{
  return getenv (name);
}

PKBool
pkCwd (PKChar *buf, PKSize size)
{
  return getcwd (buf, size) ? PK_TRUE : PK_FALSE;
}

PKTime
pkTimeNow (void)
{
  return time (NULL);
}

PKDword
pkClockMs (void)
{
#ifdef _WIN32
  return (PKDword)GetTickCount64 ();
#else
  struct timespec ts;
  clock_gettime (CLOCK_MONOTONIC, &ts);
  return (PKDword)((unsigned long long)ts.tv_sec * 1000u
                   + (unsigned long long)ts.tv_nsec / 1000000u);
#endif
}

PKInt
pkStrFmtTime (PKChar *dst, PKSize size, PKTime time)
{
  return snprintf (dst, size, "%lld", (long long)time);
}

PKTime
pkStrToTime (const PKChar *str)
{
  return (PKTime)strtoll (str, NULL, 10);
}
