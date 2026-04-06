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

#include "ini.h"
#include "test_runner.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Helpers
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Write string content to a temp file and return its path in buf. */
static void
writeTmpfile (PLChar *buf, PLSize size, const PLChar *content)
{
#ifdef _WIN32
  PLChar *tmp = _tempnam (NULL, "p6a");
  if (tmp)
    {
      strncpy (buf, tmp, size - 1);
      buf[size - 1] = '\0';
      free (tmp);
    }
  FILE *f = fopen (buf, "wb");
  if (!f)
    {
      return;
    }
  fwrite (content, 1, strlen (content), f);
  fclose (f);
#else
  const PLChar *tmp = getenv ("TMPDIR");
  if (tmp == NULL || tmp[0] == '\0')
    {
      tmp = "/tmp";
    }
  snprintf (buf, size, "%s/p6a_ini_test_XXXXXX", tmp);
  PLInt fd = mkstemp (buf);
  if (fd < 0)
    {
      return;
    }
  write (fd, content, strlen (content));
  close (fd);
#endif
}

/* Create an empty temp file and return its path in buf. */
static void
makeTmpfile (PLChar *buf, PLSize size)
{
#ifdef _WIN32
  PLChar *tmp = _tempnam (NULL, "p6a");
  if (tmp)
    {
      strncpy (buf, tmp, size - 1);
      buf[size - 1] = '\0';
      free (tmp);
    }
  FILE *f = fopen (buf, "wb");
  if (f)
    {
      fclose (f);
    }
#else
  const PLChar *tmp = getenv ("TMPDIR");
  if (tmp == NULL || tmp[0] == '\0')
    {
      tmp = "/tmp";
    }
  snprintf (buf, size, "%s/p6a_ini_tmp_XXXXXX", tmp);
  PLInt fd = mkstemp (buf);
  if (fd >= 0)
    {
      close (fd);
    }
#endif
}

/* Remove a regular file. */
static void
rmFile (const PLChar *path)
{
  unlink (path);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Init / Destroy
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testInitEmpty (void)
{
  // A freshly allocated document has no entries.
  PLIni *ini = plIniInit ();
  PL_ASSERT (ini != NULL);
  PL_ASSERT_EQ (ini->count, (PLSize)0);
  PL_ASSERT (ini->capacity >= (PLSize)1);
  plIniDestroy (ini);
}

static void
testDestroyNull (void)
{
  // plIniDestroy(NULL) must not crash.
  plIniDestroy (NULL);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Set / Get
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testSetAndGet (void)
{
  PLIni *ini = plIniInit ();

  PL_ASSERT_EQ (plIniSet (ini, "auth", "token", "abc123"), PL_EOK);
  PL_ASSERT_STR_EQ (plIniGet (ini, "auth", "token"), "abc123");

  plIniDestroy (ini);
}

static void
testGetMissingKey (void)
{
  // A key that was never set returns NULL.
  PLIni *ini = plIniInit ();
  PL_ASSERT_EQ (plIniSet (ini, "auth", "token", "abc123"), PL_EOK);
  PL_ASSERT (plIniGet (ini, "auth", "missing") == NULL);
  PL_ASSERT (plIniGet (ini, "nosection", "token") == NULL);
  plIniDestroy (ini);
}

static void
testUpdateExistingKey (void)
{
  // Setting the same section+key again overwrites the value.
  PLIni *ini = plIniInit ();

  PL_ASSERT_EQ (plIniSet (ini, "auth", "token", "old"), PL_EOK);
  PL_ASSERT_EQ (plIniSet (ini, "auth", "token", "new"), PL_EOK);

  PL_ASSERT_STR_EQ (plIniGet (ini, "auth", "token"), "new");
  PL_ASSERT_EQ (ini->count, (PLSize)1);

  plIniDestroy (ini);
}

static void
testMultipleSections (void)
{
  // Keys in different sections are independent.
  PLIni *ini = plIniInit ();

  PL_ASSERT_EQ (plIniSet (ini, "section_a", "k", "va"), PL_EOK);
  PL_ASSERT_EQ (plIniSet (ini, "section_b", "k", "vb"), PL_EOK);

  PL_ASSERT_STR_EQ (plIniGet (ini, "section_a", "k"), "va");
  PL_ASSERT_STR_EQ (plIniGet (ini, "section_b", "k"), "vb");

  plIniDestroy (ini);
}

static void
testManyEntriesGrow (void)
{
  // Insert more than the initial capacity (16) to exercise realloc.
  PLIni *ini = plIniInit ();
  PLChar key[32];
  PLChar val[32];

  for (PLInt i = 0; i < 32; ++i)
    {
      snprintf (key, sizeof (key), "key%d", i);
      snprintf (val, sizeof (val), "val%d", i);
      PL_ASSERT_EQ (plIniSet (ini, "s", key, val), PL_EOK);
    }

  PL_ASSERT_EQ (ini->count, (PLSize)32);

  for (PLInt i = 0; i < 32; ++i)
    {
      snprintf (key, sizeof (key), "key%d", i);
      snprintf (val, sizeof (val), "val%d", i);
      PL_ASSERT_STR_EQ (plIniGet (ini, "s", key), val);
    }

  plIniDestroy (ini);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Load
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testLoadBasic (void)
{
  // Parse a minimal INI file with one section and two keys.
  const PLChar *content = "[auth]\ntoken=abc\nexpiry=9999\n";
  PLChar path[256];
  writeTmpfile (path, sizeof (path), content);

  PLIni *ini = plIniInit ();
  PL_ASSERT_EQ (plIniLoad (ini, path), PL_EOK);
  PL_ASSERT_STR_EQ (plIniGet (ini, "auth", "token"), "abc");
  PL_ASSERT_STR_EQ (plIniGet (ini, "auth", "expiry"), "9999");
  plIniDestroy (ini);
  rmFile (path);
}

static void
testLoadCommentsIgnored (void)
{
  // Lines starting with ';' and '#' are ignored.
  const PLChar *content = "; this is a comment\n"
                          "# another comment\n"
                          "[s]\n"
                          "; inline section comment\n"
                          "key=value\n"
                          "# another inline comment\n";
  PLChar path[256];
  writeTmpfile (path, sizeof (path), content);

  PLIni *ini = plIniInit ();
  PL_ASSERT_EQ (plIniLoad (ini, path), PL_EOK);
  PL_ASSERT_STR_EQ (plIniGet (ini, "s", "key"), "value");
  PL_ASSERT_EQ (ini->count, (PLSize)1);
  plIniDestroy (ini);
  rmFile (path);
}

static void
testLoadWhitespaceTrimmed (void)
{
  // Leading and trailing whitespace is stripped from keys and values.
  // (Section names are stored as-is between '[' and ']'.)
  const PLChar *content = "[s]\n  key  =  value  \n";
  PLChar path[256];
  writeTmpfile (path, sizeof (path), content);

  PLIni *ini = plIniInit ();
  PL_ASSERT_EQ (plIniLoad (ini, path), PL_EOK);
  PL_ASSERT_STR_EQ (plIniGet (ini, "s", "key"), "value");
  plIniDestroy (ini);
  rmFile (path);
}

static void
testLoadEmptyLinesIgnored (void)
{
  // Blank lines between entries do not cause errors.
  const PLChar *content = "\n[s]\n\nk=v\n\n";
  PLChar path[256];
  writeTmpfile (path, sizeof (path), content);

  PLIni *ini = plIniInit ();
  PL_ASSERT_EQ (plIniLoad (ini, path), PL_EOK);
  PL_ASSERT_STR_EQ (plIniGet (ini, "s", "k"), "v");
  plIniDestroy (ini);
  rmFile (path);
}

static void
testLoadMultipleSections (void)
{
  // Keys in different sections are stored independently.
  const PLChar *content = "[global]\nendpoint=https://api.example.com\n\n"
                          "[oauth]\nclient_id=myid\n\n"
                          "[login]\naccess_token=tok\n";
  PLChar path[256];
  writeTmpfile (path, sizeof (path), content);

  PLIni *ini = plIniInit ();
  PL_ASSERT_EQ (plIniLoad (ini, path), PL_EOK);
  PL_ASSERT_STR_EQ (plIniGet (ini, "global", "endpoint"),
                    "https://api.example.com");
  PL_ASSERT_STR_EQ (plIniGet (ini, "oauth", "client_id"), "myid");
  PL_ASSERT_STR_EQ (plIniGet (ini, "login", "access_token"), "tok");
  plIniDestroy (ini);
  rmFile (path);
}

static void
testLoadNonexistentFile (void)
{
  // Loading a path that does not exist must return PL_EFS.
  PLIni *ini = plIniInit ();
  PL_ASSERT_EQ (plIniLoad (ini, "/tmp/p6a_no_such_ini_xyz987654.ini"), PL_EFS);
  plIniDestroy (ini);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Save / Round-trip
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testSaveAndReload (void)
{
  // Build a document in memory, save it, reload it, verify values.
  PLChar path[256];
  makeTmpfile (path, sizeof (path));

  PLIni *ini = plIniInit ();
  PL_ASSERT_EQ (plIniSet (ini, "s1", "a", "1"), PL_EOK);
  PL_ASSERT_EQ (plIniSet (ini, "s1", "b", "2"), PL_EOK);
  PL_ASSERT_EQ (plIniSet (ini, "s2", "c", "3"), PL_EOK);
  PL_ASSERT_EQ (plIniSave (ini, path), PL_EOK);
  plIniDestroy (ini);

  PLIni *loaded = plIniInit ();
  PL_ASSERT_EQ (plIniLoad (loaded, path), PL_EOK);
  PL_ASSERT_STR_EQ (plIniGet (loaded, "s1", "a"), "1");
  PL_ASSERT_STR_EQ (plIniGet (loaded, "s1", "b"), "2");
  PL_ASSERT_STR_EQ (plIniGet (loaded, "s2", "c"), "3");
  plIniDestroy (loaded);

  rmFile (path);
}

static void
testRoundtripUpdate (void)
{
  // Set, update a key, save, reload — reloaded value must be updated.
  PLChar path[256];
  makeTmpfile (path, sizeof (path));

  PLIni *ini = plIniInit ();
  PL_ASSERT_EQ (plIniSet (ini, "auth", "token", "old"), PL_EOK);
  PL_ASSERT_EQ (plIniSet (ini, "auth", "token", "new"), PL_EOK);
  PL_ASSERT_EQ (plIniSave (ini, path), PL_EOK);
  plIniDestroy (ini);

  PLIni *loaded = plIniInit ();
  PL_ASSERT_EQ (plIniLoad (loaded, path), PL_EOK);
  PL_ASSERT_STR_EQ (plIniGet (loaded, "auth", "token"), "new");
  PL_ASSERT_EQ (loaded->count, (PLSize)1);
  plIniDestroy (loaded);

  rmFile (path);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Main
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
main (void)
{
  PL_SUITE ("ini - init / destroy");
  PL_RUN (testInitEmpty);
  PL_RUN (testDestroyNull);

  PL_SUITE ("ini - set / get");
  PL_RUN (testSetAndGet);
  PL_RUN (testGetMissingKey);
  PL_RUN (testUpdateExistingKey);
  PL_RUN (testMultipleSections);
  PL_RUN (testManyEntriesGrow);

  PL_SUITE ("ini - load");
  PL_RUN (testLoadBasic);
  PL_RUN (testLoadCommentsIgnored);
  PL_RUN (testLoadWhitespaceTrimmed);
  PL_RUN (testLoadEmptyLinesIgnored);
  PL_RUN (testLoadMultipleSections);
  PL_RUN (testLoadNonexistentFile);

  PL_SUITE ("ini - save / round-trip");
  PL_RUN (testSaveAndReload);
  PL_RUN (testRoundtripUpdate);

  PL_SUMMARY ();
}
