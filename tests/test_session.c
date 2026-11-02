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

/* macOS declares mkdtemp() only with _DARWIN_C_SOURCE. */
#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#endif

#include "partoska.h"
#include "test_runner.h"
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

static PKInt
setenv (const PKChar *name, const PKChar *value, PKInt overwrite)
{
  if (!overwrite && getenv (name) != NULL)
    {
      return 0;
    }
  return _putenv_s (name, value);
}

static PKInt
unsetenv (const PKChar *name)
{
  return _putenv_s (name, "");
}

#define mkdir(path, mode) _mkdir (path)
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Helpers
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Create a unique temp directory and return its path. */
static PKVoid
makeTmpdir (PKChar *buf, PKSize size)
{
  const PKChar *tmp = getenv ("TMPDIR");
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
  PK_ASSERT (_mktemp_s (buf, strlen (buf) + 1) == 0 && _mkdir (buf) == 0);
#else
  PK_ASSERT (mkdtemp (buf) != NULL);
#endif
}

/* Copy an environment variable into buf, as a later setenv() may free the
 * string getenv() returned; returns NULL when the variable is unset. */
static const PKChar *
saveEnv (const PKChar *name, PKChar *buf, PKSize size)
{
  const PKChar *value = getenv (name);
  if (value == NULL)
    {
      return NULL;
    }
  snprintf (buf, size, "%s", value);
  return buf;
}

/* Remove a directory (must be empty). */
static PKVoid
rmDir (const PKChar *path)
{
  rmdir (path);
}

/* Write text to dir/name and return the path in buf. */
static PKVoid
writeFile (PKChar *buf, PKSize size, const PKChar *dir, const PKChar *name,
           const PKChar *text)
{
  snprintf (buf, size, "%s/%s", dir, name);
  FILE *f = fopen (buf, "wb");
  PK_ASSERT (f != NULL);
  if (f)
    {
      fputs (text, f);
      fclose (f);
    }
}

/* Restore an environment variable saved with saveEnv(). */
static PKVoid
restoreEnv (const PKChar *name, const PKChar *value)
{
  if (value)
    {
      setenv (name, value, 1);
    }
  else
    {
      unsetenv (name);
    }
}

/* A login in the format p6a writes. */
static const PKChar *const LOGIN_INI = "[Login]\n"
                                       "Access=a1\n"
                                       "Refresh=r1\n"
                                       "Expires=4102444800\n";

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Working directory
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testOpenExplicitDir (void)
{
  // An existing directory is used as is; without p6a.ini, not logged in.
  PKChar tmpdir[512];
  makeTmpdir (tmpdir, sizeof (tmpdir));

  PKClient *client = pkInit ();
  PK_ASSERT_STR_EQ (pkDir (client), "");
  PK_ASSERT_EQ (pkOpen (client, tmpdir), PK_OK);
  PK_ASSERT_STR_EQ (pkDir (client), tmpdir);
  PK_ASSERT (!pkLoginValid (client));
  pkDestroy (client);

  rmDir (tmpdir);
}

static PKVoid
testOpenNonexistentDir (void)
{
  // A directory that does not exist is an error, recorded in the client.
  PKClient *client = pkInit ();
  PK_ASSERT_EQ (pkOpen (client, "/tmp/p6a_no_such_dir_xyz987"), PK_EFS);
  PK_ASSERT_EQ (pkError (client)->status, PK_EFS);
  PK_ASSERT (strstr (pkError (client)->message, "p6a_no_such_dir_xyz987"));
  PK_ASSERT_STR_EQ (pkDir (client), "");
  pkDestroy (client);
}

static PKVoid
testOpenP6aHome (void)
{
  // Without a directory, P6A_HOME is used, and has to exist.
  PKChar tmpdir[512];
  makeTmpdir (tmpdir, sizeof (tmpdir));
  PKChar saved[1024];
  const PKChar *prev = saveEnv ("P6A_HOME", saved, sizeof (saved));

  setenv ("P6A_HOME", tmpdir, 1);
  PKClient *client = pkInit ();
  PKStatus found = pkOpen (client, NULL);
  PKChar dir[512];
  snprintf (dir, sizeof (dir), "%s", pkDir (client));

  setenv ("P6A_HOME", "/tmp/p6a_no_such_dir_abc123", 1);
  PKStatus missing = pkOpen (client, NULL);
  pkDestroy (client);
  restoreEnv ("P6A_HOME", prev);

  PK_ASSERT_EQ (found, PK_OK);
  PK_ASSERT_STR_EQ (dir, tmpdir);
  PK_ASSERT_EQ (missing, PK_EFS);

  rmDir (tmpdir);
}

static PKVoid
testOpenHomeCreatesP6aDir (void)
{
  // Without P6A_HOME, ~/.p6a is created, and reused the next time.
  PKChar tmpdir[512];
  makeTmpdir (tmpdir, sizeof (tmpdir));
  PKChar expWd[600];
  snprintf (expWd, sizeof (expWd), "%s/.p6a", tmpdir);

  PKChar savedP6a[1024];
  PKChar savedHome[1024];
  const PKChar *prevP6a = saveEnv ("P6A_HOME", savedP6a, sizeof (savedP6a));
  const PKChar *prevHome = saveEnv ("HOME", savedHome, sizeof (savedHome));
  unsetenv ("P6A_HOME");
  setenv ("HOME", tmpdir, 1);

  PKClient *client = pkInit ();
  PKStatus first = pkOpen (client, NULL);
  PKChar dir[600];
  snprintf (dir, sizeof (dir), "%s", pkDir (client));
  PKStatus second = pkOpen (client, NULL);
  pkDestroy (client);

  restoreEnv ("P6A_HOME", prevP6a);
  restoreEnv ("HOME", prevHome);

  PK_ASSERT_EQ (first, PK_OK);
  PK_ASSERT_EQ (second, PK_OK);
  PK_ASSERT_STR_EQ (dir, expWd);
  struct stat st;
  PK_ASSERT_EQ (stat (expWd, &st), 0);
  PK_ASSERT (S_ISDIR (st.st_mode));

  rmDir (expWd);
  rmDir (tmpdir);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Configuration
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testOpenReadsLogin (void)
{
  // The login in p6a.ini is read when opening.
  PKChar tmpdir[512];
  makeTmpdir (tmpdir, sizeof (tmpdir));
  PKChar ini[600];
  writeFile (ini, sizeof (ini), tmpdir, "p6a.ini", LOGIN_INI);

  PKClient *client = pkInit ();
  PK_ASSERT_EQ (pkOpen (client, tmpdir), PK_OK);
  PK_ASSERT (pkLoginValid (client));
  pkDestroy (client);

  remove (ini);
  rmDir (tmpdir);
}

static PKVoid
testOpenMalformedConfig (void)
{
  // A malformed p6a.ini fails the open and names the file and line.
  PKChar tmpdir[512];
  makeTmpdir (tmpdir, sizeof (tmpdir));
  PKChar ini[600];
  writeFile (ini, sizeof (ini), tmpdir, "p6a.ini", "[Login]\nAccess\n");

  PKClient *client = pkInit ();
  PK_ASSERT_EQ (pkOpen (client, tmpdir), PK_EDATA);
  PK_ASSERT (strstr (pkError (client)->message, "p6a.ini: line 2"));
  PK_ASSERT_STR_EQ (pkDir (client), "");
  pkDestroy (client);

  remove (ini);
  rmDir (tmpdir);
}

static PKVoid
testLogoutSaves (void)
{
  // Logging out forgets the login, in the client and in p6a.ini.
  PKChar tmpdir[512];
  makeTmpdir (tmpdir, sizeof (tmpdir));
  PKChar ini[600];
  writeFile (ini, sizeof (ini), tmpdir, "p6a.ini", LOGIN_INI);

  PKClient *client = pkInit ();
  PK_ASSERT_EQ (pkOpen (client, tmpdir), PK_OK);
  PK_ASSERT_EQ (pkLogout (client), PK_OK);
  PK_ASSERT (!pkLoginValid (client));
  pkDestroy (client);

  client = pkInit ();
  PK_ASSERT_EQ (pkOpen (client, tmpdir), PK_OK);
  PK_ASSERT (!pkLoginValid (client));
  pkDestroy (client);

  remove (ini);
  rmDir (tmpdir);
}

static PKVoid
testImport (void)
{
  // An import replaces the configuration and saves it as p6a.ini.
  PKChar tmpdir[512];
  makeTmpdir (tmpdir, sizeof (tmpdir));
  PKChar source[600];
  writeFile (source, sizeof (source), tmpdir, "other.ini", LOGIN_INI);

  PKClient *client = pkInit ();
  PK_ASSERT_EQ (pkImport (client, source), PK_EARG);
  PK_ASSERT_EQ (pkOpen (client, tmpdir), PK_OK);
  PK_ASSERT_EQ (pkImport (client, source), PK_OK);
  PK_ASSERT (pkLoginValid (client));
  pkDestroy (client);

  client = pkInit ();
  PK_ASSERT_EQ (pkOpen (client, tmpdir), PK_OK);
  PK_ASSERT (pkLoginValid (client));
  pkDestroy (client);

  PKChar ini[600];
  snprintf (ini, sizeof (ini), "%s/p6a.ini", tmpdir);
  remove (ini);
  remove (source);
  rmDir (tmpdir);
}

static PKVoid
testImportMissingFile (void)
{
  // A missing import file is an error and changes nothing.
  PKChar tmpdir[512];
  makeTmpdir (tmpdir, sizeof (tmpdir));

  PKClient *client = pkInit ();
  PK_ASSERT_EQ (pkOpen (client, tmpdir), PK_OK);
  PK_ASSERT_EQ (pkImport (client, "/tmp/p6a_no_such_import.ini"), PK_EFS);
  PK_ASSERT (strstr (pkError (client)->message, "Cannot import"));
  PK_ASSERT (!pkLoginValid (client));
  pkDestroy (client);

  rmDir (tmpdir);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Client
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testStop (void)
{
  // A stop request is remembered.
  PKClient *client = pkInit ();
  PK_ASSERT (!pkStopping (client));
  pkStop (client);
  PK_ASSERT (pkStopping (client));
  pkDestroy (client);
}

static PKVoid
testStatusText (void)
{
  // Every status has a description.
  PK_ASSERT_STR_EQ (pkStatusText (PK_OK), "Success");
  PK_ASSERT (strlen (pkStatusText (PK_ESTOPPED)) > 0);
  PK_ASSERT_STR_EQ (pkStatusText ((PKStatus)-99), "Unknown error");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Main
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
main (void)
{
  PK_SUITE ("session - working directory");
  PK_RUN (testOpenExplicitDir);
  PK_RUN (testOpenNonexistentDir);
  PK_RUN (testOpenP6aHome);
  PK_RUN (testOpenHomeCreatesP6aDir);

  PK_SUITE ("session - configuration");
  PK_RUN (testOpenReadsLogin);
  PK_RUN (testOpenMalformedConfig);
  PK_RUN (testLogoutSaves);
  PK_RUN (testImport);
  PK_RUN (testImportMissingFile);

  PK_SUITE ("session - client");
  PK_RUN (testStop);
  PK_RUN (testStatusText);

  PK_SUMMARY ();
}
