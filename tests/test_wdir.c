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

/* macOS declares mkdtemp() only with _DARWIN_C_SOURCE. */
#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#endif

#include "config.h"
#include "test_runner.h"
#include "wdir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Platform compatibility (Windows / MinGW)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef _WIN32
#include <direct.h>
#include <io.h>

static PLInt
setenv (const PLChar *name, const PLChar *value, PLInt overwrite)
{
  if (!overwrite && getenv (name) != NULL)
    {
      return 0;
    }
  return _putenv_s (name, value);
}

static PLInt
unsetenv (const PLChar *name)
{
  return _putenv_s (name, "");
}

#define mkdir(path, mode) _mkdir (path)
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Helpers
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Create a unique temp directory and return its path. */
static void
makeTmpdir (PLChar *buf, PLSize size)
{
  const PLChar *tmp = getenv ("TMPDIR");
#ifdef _WIN32
  if (tmp == NULL)
    {
      tmp = getenv ("TEMP");
    }
  if (tmp == NULL)
    {
      tmp = getenv ("TMP");
    }
#endif
  if (tmp == NULL)
    {
      tmp = "/tmp";
    }
  snprintf (buf, size, "%s/p6a_test_XXXXXX", tmp);
#ifdef _WIN32
  _mktemp_s (buf, strlen (buf) + 1);
  _mkdir (buf);
#else
  mkdtemp (buf);
#endif
}

/* Copy an environment variable into buf, as a later setenv() may free the
 * string getenv() returned; returns NULL when the variable is unset. */
static const PLChar *
saveEnv (const PLChar *name, PLChar *buf, PLSize size)
{
  const PLChar *value = getenv (name);
  if (value == NULL)
    {
      return NULL;
    }
  snprintf (buf, size, "%s", value);
  return buf;
}

/* Remove a directory (must be empty). */
static void
rmDir (const PLChar *path)
{
  rmdir (path);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - plGetIniPath
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testIniPath (void)
{
  PLChar path[256];
  PLChar expected[256];

  plGetIniPath (path, sizeof (path), "/home/user/.p6a");
  snprintf (expected, sizeof (expected), "/home/user/.p6a%s", PL_CONFIG_INI);
  PL_ASSERT_STR_EQ (path, expected);

  plGetIniPath (path, sizeof (path), "/custom/dir");
  snprintf (expected, sizeof (expected), "/custom/dir%s", PL_CONFIG_INI);
  PL_ASSERT_STR_EQ (path, expected);

  // Buffer is always NUL-terminated regardless of input length.
  PLChar small[16];
  plGetIniPath (small, sizeof (small), "/a");
  PL_ASSERT_EQ (small[sizeof (small) - 1], '\0');
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - plPrepareWorkdir: explicit path via -D / workdir argument
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testExplicitExistingDir (void)
{
  // When workdir is pre-filled with an existing directory path, it is used
  // as-is without consulting any environment variables.
  PLChar tmpdir[256];
  makeTmpdir (tmpdir, sizeof (tmpdir));

  PLChar workdir[256];
  strncpy (workdir, tmpdir, sizeof (workdir) - 1);
  workdir[sizeof (workdir) - 1] = '\0';

  PL_ASSERT_EQ (plPrepareWorkdir (workdir, sizeof (workdir)), PL_EOK);
  PL_ASSERT_STR_EQ (workdir, tmpdir);

  rmDir (tmpdir);
}

static void
testExplicitNonexistentDir (void)
{
  // A path that does not exist must fail.
  PLChar workdir[256] = "/tmp/p6a_this_dir_must_not_exist_xyz987654";
  PL_ASSERT_EQ (plPrepareWorkdir (workdir, sizeof (workdir)), PL_EFS);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - plPrepareWorkdir: P6A_HOME environment variable
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testP6aHomeExisting (void)
{
  // P6A_HOME pointing at an existing directory is used directly.
  PLChar tmpdir[256];
  makeTmpdir (tmpdir, sizeof (tmpdir));

  PLChar prevBuf[1024];
  const PLChar *prevHome = saveEnv ("P6A_HOME", prevBuf, sizeof (prevBuf));
  setenv ("P6A_HOME", tmpdir, 1);

  PLChar workdir[256] = "";
  PLInt result = plPrepareWorkdir (workdir, sizeof (workdir));

  // Restore before asserting so cleanup always runs.
  if (prevHome)
    {
      setenv ("P6A_HOME", prevHome, 1);
    }
  else
    {
      unsetenv ("P6A_HOME");
    }

  PL_ASSERT_EQ (result, PL_EOK);
  PL_ASSERT_STR_EQ (workdir, tmpdir);

  rmDir (tmpdir);
}

static void
testP6aHomeNonexistent (void)
{
  // P6A_HOME pointing at a non-existent directory must fail.
  PLChar prevBuf[1024];
  const PLChar *prevHome = saveEnv ("P6A_HOME", prevBuf, sizeof (prevBuf));
  setenv ("P6A_HOME", "/tmp/p6a_no_such_dir_abc123", 1);

  PLChar workdir[256] = "";
  PLInt result = plPrepareWorkdir (workdir, sizeof (workdir));

  if (prevHome)
    {
      setenv ("P6A_HOME", prevHome, 1);
    }
  else
    {
      unsetenv ("P6A_HOME");
    }

  PL_ASSERT_EQ (result, PL_EFS);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - plPrepareWorkdir: HOME environment variable
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testHomeCreatesP6aSubdir (void)
{
  // When HOME is set and P6A_HOME is absent, the function should resolve to
  // $HOME/.p6a and create it if it does not exist.
  PLChar tmpdir[256];
  makeTmpdir (tmpdir, sizeof (tmpdir));

  PLChar expWd[512];
  snprintf (expWd, sizeof (expWd), "%s/.p6a", tmpdir);

  PLChar prevP6aBuf[1024];
  PLChar prevHomeBuf[1024];
  const PLChar *prevP6a
      = saveEnv ("P6A_HOME", prevP6aBuf, sizeof (prevP6aBuf));
  const PLChar *prevHome = saveEnv ("HOME", prevHomeBuf, sizeof (prevHomeBuf));

  unsetenv ("P6A_HOME");
  setenv ("HOME", tmpdir, 1);

  PLChar workdir[512] = "";
  PLInt result = plPrepareWorkdir (workdir, sizeof (workdir));

  // Restore env before asserting.
  if (prevP6a)
    {
      setenv ("P6A_HOME", prevP6a, 1);
    }
  if (prevHome)
    {
      setenv ("HOME", prevHome, 1);
    }

  PL_ASSERT_EQ (result, PL_EOK);
  PL_ASSERT_STR_EQ (workdir, expWd);

  // Verify the directory was actually created on disk.
  struct stat st;
  PL_ASSERT_EQ (stat (expWd, &st), 0);
  PL_ASSERT (S_ISDIR (st.st_mode));

  // Cleanup.
  rmDir (expWd);
  rmDir (tmpdir);
}

static void
testHomeReusesExistingP6aSubdir (void)
{
  // Calling plPrepareWorkdir twice with the same HOME is idempotent.
  PLChar tmpdir[256];
  makeTmpdir (tmpdir, sizeof (tmpdir));

  PLChar p6aDir[512];
  snprintf (p6aDir, sizeof (p6aDir), "%s/.p6a", tmpdir);
  // Pre-create the .p6a subdir.
  mkdir (p6aDir, 0700);

  PLChar prevP6aBuf[1024];
  PLChar prevHomeBuf[1024];
  const PLChar *prevP6a
      = saveEnv ("P6A_HOME", prevP6aBuf, sizeof (prevP6aBuf));
  const PLChar *prevHome = saveEnv ("HOME", prevHomeBuf, sizeof (prevHomeBuf));

  unsetenv ("P6A_HOME");
  setenv ("HOME", tmpdir, 1);

  PLChar workdir[512] = "";
  PLInt result = plPrepareWorkdir (workdir, sizeof (workdir));

  if (prevP6a)
    {
      setenv ("P6A_HOME", prevP6a, 1);
    }
  if (prevHome)
    {
      setenv ("HOME", prevHome, 1);
    }

  PL_ASSERT_EQ (result, PL_EOK);
  PL_ASSERT_STR_EQ (workdir, p6aDir);

  rmDir (p6aDir);
  rmDir (tmpdir);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - plGetIniPath round-trip with plPrepareWorkdir
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testIniPathFromWorkdir (void)
{
  // E2E: Prepare a workdir then build the INI path from it.
  PLChar tmpdir[256];
  makeTmpdir (tmpdir, sizeof (tmpdir));

  PLChar workdir[256];
  strncpy (workdir, tmpdir, sizeof (workdir) - 1);
  workdir[sizeof (workdir) - 1] = '\0';

  PL_ASSERT_EQ (plPrepareWorkdir (workdir, sizeof (workdir)), PL_EOK);

  PLChar ini[512];
  plGetIniPath (ini, sizeof (ini), workdir);

  PLChar expected[512];
  snprintf (expected, sizeof (expected), "%s%s", tmpdir, PL_CONFIG_INI);
  PL_ASSERT_STR_EQ (ini, expected);

  rmDir (tmpdir);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Main
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
main (void)
{
  PL_SUITE ("wdir - plGetIniPath");
  PL_RUN (testIniPath);

  PL_SUITE ("wdir - explicit path (-D argument)");
  PL_RUN (testExplicitExistingDir);
  PL_RUN (testExplicitNonexistentDir);

  PL_SUITE ("wdir - P6A_HOME environment variable");
  PL_RUN (testP6aHomeExisting);
  PL_RUN (testP6aHomeNonexistent);

  PL_SUITE ("wdir - HOME environment variable");
  PL_RUN (testHomeCreatesP6aSubdir);
  PL_RUN (testHomeReusesExistingP6aSubdir);

  PL_SUITE ("wdir - round-trip");
  PL_RUN (testIniPathFromWorkdir);

  PL_SUMMARY ();
}
