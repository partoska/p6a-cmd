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

#include "plat.h"
#include <getopt.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <dirent.h>
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Strings
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKSize
pkcStrLen (const PKChar *str)
{
  return strlen (str);
}

PKBool
pkcStrEq (const PKChar *a, const PKChar *b)
{
  return strcmp (a, b) == 0;
}

PKBool
pkcStrEqN (const PKChar *a, const PKChar *b, PKSize n)
{
  return strncmp (a, b, n) == 0;
}

PKChar *
pkcStrFindAny (const PKChar *str, const PKChar *set)
{
  return strpbrk (str, set);
}

PKChar *
pkcStrFindStr (const PKChar *str, const PKChar *sub)
{
  return strstr (str, sub);
}

PKChar *
pkcStrDup (const PKChar *str, PKSize length)
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
pkcStrFmt (PKChar *dst, PKSize size, const PKChar *fmt, ...)
{
  PKVaList args;
  PK_VA_START (args, fmt);
  PKInt n = vsnprintf (dst, size, fmt, args);
  PK_VA_END (args);

  return n;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Memory
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKVoid *
pkcMemAlloc (PKSize size)
{
  return malloc (size ? size : 1);
}

PKVoid *
pkcMemZalloc (PKSize size)
{
  return calloc (1, size ? size : 1);
}

PKVoid
pkcMemFree (PKVoid *ptr)
{
  free (ptr);
}

PKVoid
pkcMemZero (PKVoid *ptr, PKSize size)
{
  memset (ptr, 0, size);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef _WIN32
typedef struct _stat PKCStat;
#define pkcIsDirStat(m) (((m) & _S_IFMT) == _S_IFDIR)
#define pkcIsRegStat(m) (((m) & _S_IFMT) == _S_IFREG)

static wchar_t *
pkcToWide (const PKChar *utf8)
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
pkcStat (const PKChar *path, PKCStat *st)
{
  wchar_t *wide = pkcToWide (path);
  if (!wide)
    {
      return PK_EFS;
    }

  PKInt r = _wstat (wide, st);
  free (wide);
  return r;
}

static PKInt
pkcMkdir (const PKChar *path)
{
  wchar_t *wide = pkcToWide (path);
  if (!wide)
    {
      return PK_EFS;
    }

  PKInt r = _wmkdir (wide);
  free (wide);

  return r;
}

static PKChar *
pkcFromWide (const wchar_t *wide, PKChar *buf, PKSize size)
{
  if (size > 0x7FFFFFFF)
    {
      return NULL;
    }

  PKInt len
      = WideCharToMultiByte (CP_UTF8, 0, wide, -1, buf, (INT)size, NULL, NULL);
  if (len <= 0)
    {
      return NULL;
    }

  return buf;
}

struct PKCDir
{
  HANDLE handle;
  WIN32_FIND_DATAW ffd;
  PKBool first;
  PKChar name[1024];
};
#else
typedef struct stat PKCStat;
#define pkcIsDirStat(m) S_ISDIR (m)
#define pkcIsRegStat(m) S_ISREG (m)
#define pkcStat(path, st) stat (path, st)
#define pkcMkdir(path) mkdir (path, 0755)

struct PKCDir
{
  DIR *d;
};
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKBool
pkcIsDir (const PKChar *path)
{
  PKCStat st;
  if (pkcStat (path, &st) != 0)
    {
      return PK_FALSE;
    }

  return pkcIsDirStat (st.st_mode) ? PK_TRUE : PK_FALSE;
}

PKBool
pkcIsFile (const PKChar *path)
{
  PKCStat st;
  if (pkcStat (path, &st) != 0)
    {
      return PK_FALSE;
    }

  return pkcIsRegStat (st.st_mode) ? PK_TRUE : PK_FALSE;
}

PKCFile *
pkcFileOpen (const PKChar *path, PKBool write)
{
#ifdef _WIN32
  wchar_t *wide = pkcToWide (path);
  if (!wide)
    {
      return NULL;
    }

  FILE *file = _wfopen (wide, write ? L"wb" : L"rb");
  free (wide);

  return (PKCFile *)file;
#else
  return (PKCFile *)fopen (path, write ? "wb" : "rb");
#endif
}

PKSize
pkcFileRead (PKCFile *file, PKVoid *data, PKSize size)
{
  return fread (data, 1, size, (FILE *)file);
}

PKSize
pkcFileWrite (PKCFile *file, const PKVoid *data, PKSize size)
{
  return fwrite (data, 1, size, (FILE *)file);
}

PKBool
pkcFileFailed (PKCFile *file)
{
  return ferror ((FILE *)file) != 0;
}

PKBool
pkcFileClose (PKCFile *file)
{
  return fclose ((FILE *)file) == 0;
}

PKBool
pkcFileRename (const PKChar *from, const PKChar *to)
{
#ifdef _WIN32
  wchar_t *wfrom = pkcToWide (from);
  wchar_t *wto = pkcToWide (to);
  PKBool ok = wfrom && wto && _wrename (wfrom, wto) == 0;
  free (wfrom);
  free (wto);

  return ok;
#else
  return rename (from, to) == 0;
#endif
}

#ifdef _WIN32
static PKDword pkcConsolePage;

static PKVoid
pkcConsoleRestore (void)
{
  SetConsoleOutputCP (pkcConsolePage);
}
#endif

PKVoid
pkcConsoleInit (void)
{
#ifdef _WIN32
  // A console shows the UTF-8 it is given only in its UTF-8 code page; the
  // one it had comes back at exit, so the shell keeps its own. Without a
  // console the page reads as 0 and nothing changes.
  PKDword page = GetConsoleOutputCP ();
  if (page != 0 && page != CP_UTF8 && SetConsoleOutputCP (CP_UTF8))
    {
      pkcConsolePage = page;
      atexit (pkcConsoleRestore);
    }
#endif
}

PKInt
pkcCreateDir (const PKChar *path)
{
  if (pkcIsDir (path))
    {
      // Indicates that directory already exists.
      return 1;
    }
  if (pkcMkdir (path) < 0)
    {
      return PK_EFS;
    }

  return PK_OK;
}

PKCDir *
pkcOpenDir (const PKChar *path)
{
#ifdef _WIN32
  PKChar pattern[1024 + 3];
  snprintf (pattern, sizeof (pattern) - 1, "%s/*", path);
  pattern[sizeof (pattern) - 1] = '\0';

  wchar_t *wpattern = pkcToWide (pattern);
  if (!wpattern)
    {
      return NULL;
    }

  PKCDir *dir = malloc (sizeof (PKCDir));
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

  dir->first = PK_TRUE;
  return dir;
#else
  DIR *d = opendir (path);
  if (!d)
    {
      return NULL;
    }

  PKCDir *dir = malloc (sizeof (PKCDir));
  if (!dir)
    {
      closedir (d);
      return NULL;
    }

  dir->d = d;
  return dir;
#endif
}

const PKChar *
pkcReadDir (PKCDir *dir)
{
#ifdef _WIN32
  if (dir->first)
    {
      dir->first = PK_FALSE;
    }
  else
    {
      if (!FindNextFileW (dir->handle, &dir->ffd))
        {
          return NULL;
        }
    }

  if (!pkcFromWide (dir->ffd.cFileName, dir->name, sizeof (dir->name)))
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

PKVoid
pkcCloseDir (PKCDir *dir)
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

PKVoid
pkcOptReset (void)
{
#if defined(__linux__) || defined(__GLIBC__) || defined(_WIN32)
  // glibc, musl and MinGW start over at optind 0.
  optind = 0;
#else
  // BSD and macOS start over with optreset.
  optind = 1;
  optreset = 1;
#endif
}

PKInt
pkcOptNext (PKInt argc, PKChar **argv, const PKCOpt *opts,
            const PKChar **value)
{
  struct option longs[PKC_OPT_MAX + 1];
  PKChar shorts[2 * PKC_OPT_MAX + 1];
  PKSize count = 0;
  PKSize length = 0;
  *value = NULL;
  for (; opts[count].name != NULL; count++)
    {
      if (count == PKC_OPT_MAX)
        {
          return '?';
        }
      longs[count].name = opts[count].name;
      longs[count].has_arg
          = opts[count].value ? required_argument : no_argument;
      longs[count].flag = NULL;
      longs[count].val = opts[count].key;
      shorts[length++] = (PKChar)opts[count].key;
      if (opts[count].value)
        {
          shorts[length++] = ':';
        }
    }
  memset (&longs[count], 0, sizeof (longs[count]));
  shorts[length] = '\0';

  // getopt_long sets optarg only for options with a value.
  opterr = 0;
  optarg = NULL;
  PKInt key = getopt_long (argc, argv, shorts, longs, NULL);
  if (key != -1 && key != '?')
    {
      *value = optarg;
    }
  return key;
}

PKInt
pkcOptIndex (void)
{
  return optind;
}

PKInt
pkcPrintV (PKBool error, const PKChar *fmt, PKVaList args)
{
  return vfprintf (error ? stderr : stdout, fmt, args);
}

PKVoid
pkcFlush (void)
{
  fflush (stdout);
}

PKVoid
pkcSignals (PKCSignalFn handler)
{
  signal (SIGINT, handler ? handler : SIG_DFL);
  signal (SIGTERM, handler ? handler : SIG_DFL);
}

PKTime
pkcTimeNow (void)
{
  return time (NULL);
}

PKBool
pkcDateParse (const PKChar *text, PKInt *year, PKInt *month, PKInt *day)
{
  return sscanf (text, "%d-%d-%d", year, month, day) == 3;
}
