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
#include "partoska.h"
#include "pktypes.h"
#include "test_runner.h"
#include <bikini.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Helpers
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Write string content to a temp file and return its path in buf. */
static PKVoid
writeTmpfile (PKChar *buf, PKSize size, const PKChar *content)
{
#ifdef _WIN32
  PKChar *tmp = _tempnam (NULL, "p6a");
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
  const PKChar *tmp = getenv ("TMPDIR");
  if (tmp == NULL || tmp[0] == '\0')
    {
      tmp = "/tmp";
    }
  snprintf (buf, size, "%s/p6a_ini_test_XXXXXX", tmp);
  PKInt fd = mkstemp (buf);
  if (fd < 0)
    {
      return;
    }
  PKSize len = strlen (content);
  PK_ASSERT_EQ ((PKLong)write (fd, content, len), (PKLong)len);
  close (fd);
#endif
}

/* Create an empty temp file and return its path in buf. */
static PKVoid
makeTmpfile (PKChar *buf, PKSize size)
{
#ifdef _WIN32
  PKChar *tmp = _tempnam (NULL, "p6a");
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
  const PKChar *tmp = getenv ("TMPDIR");
  if (tmp == NULL || tmp[0] == '\0')
    {
      tmp = "/tmp";
    }
  snprintf (buf, size, "%s/p6a_ini_tmp_XXXXXX", tmp);
  PKInt fd = mkstemp (buf);
  if (fd >= 0)
    {
      close (fd);
    }
#endif
}

/* Number of keys in a section of a document. */
static PKSize
keyCount (const PKCIni *ini, const PKChar *section)
{
  return bkKeyCount (bkSectionFind (ini, section));
}

/* Read a whole text file into buf, without carriage returns. */
static PKVoid
readFile (const PKChar *path, PKChar *buf, PKSize size)
{
  PKSize len = 0;
  FILE *f = fopen (path, "rb");
  if (f)
    {
      PKInt c;
      while (len + 1 < size && (c = fgetc (f)) != EOF)
        {
          if (c != '\r')
            {
              buf[len++] = (PKChar)c;
            }
        }
      fclose (f);
    }
  buf[len] = '\0';
}

/* Temporary directory of the platform. */
static const PKChar *
tmpDir (void)
{
#ifdef _WIN32
  const PKChar *tmp = getenv ("TEMP");
#else
  const PKChar *tmp = getenv ("TMPDIR");
#endif
  if (tmp == NULL || tmp[0] == '\0')
    {
      return "/tmp";
    }

  return tmp;
}

/* Remove a file given by a UTF-8 path. */
static PKVoid
rmFileUtf8 (const PKChar *path)
{
#ifdef _WIN32
  wchar_t wpath[512];
  if (MultiByteToWideChar (CP_UTF8, 0, path, -1, wpath, 512) > 0)
    {
      _wremove (wpath);
    }
#else
  remove (path);
#endif
}

/* Remove a regular file. */
static PKVoid
rmFile (const PKChar *path)
{
  unlink (path);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Init / Destroy
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testInitEmpty (void)
{
  // A freshly allocated document has no entries.
  PKCIni *ini = pkcIniInit ();
  PK_ASSERT (ini != NULL);
  PK_ASSERT_EQ (bkSectionCount (ini), (PKSize)0);
  pkcIniDestroy (ini);
}

static PKVoid
testDestroyNull (void)
{
  // pkcIniDestroy(NULL) must not crash.
  pkcIniDestroy (NULL);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Set / Get
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testSetAndGet (void)
{
  PKCIni *ini = pkcIniInit ();

  PK_ASSERT_EQ (pkcIniSet (ini, "auth", "token", "abc123"), PK_OK);
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "auth", "token"), "abc123");

  pkcIniDestroy (ini);
}

static PKVoid
testGetMissingKey (void)
{
  // A key that was never set returns NULL.
  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniSet (ini, "auth", "token", "abc123"), PK_OK);
  PK_ASSERT (pkcIniGet (ini, "auth", "missing") == NULL);
  PK_ASSERT (pkcIniGet (ini, "nosection", "token") == NULL);
  pkcIniDestroy (ini);
}

static PKVoid
testUpdateExistingKey (void)
{
  // Setting the same section+key again overwrites the value.
  PKCIni *ini = pkcIniInit ();

  PK_ASSERT_EQ (pkcIniSet (ini, "auth", "token", "old"), PK_OK);
  PK_ASSERT_EQ (pkcIniSet (ini, "auth", "token", "new"), PK_OK);

  PK_ASSERT_STR_EQ (pkcIniGet (ini, "auth", "token"), "new");
  PK_ASSERT_EQ (keyCount (ini, "auth"), (PKSize)1);

  pkcIniDestroy (ini);
}

static PKVoid
testMultipleSections (void)
{
  // Keys in different sections are independent.
  PKCIni *ini = pkcIniInit ();

  PK_ASSERT_EQ (pkcIniSet (ini, "section_a", "k", "va"), PK_OK);
  PK_ASSERT_EQ (pkcIniSet (ini, "section_b", "k", "vb"), PK_OK);

  PK_ASSERT_STR_EQ (pkcIniGet (ini, "section_a", "k"), "va");
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "section_b", "k"), "vb");

  pkcIniDestroy (ini);
}

static PKVoid
testManyEntriesGrow (void)
{
  // Insert more than the initial capacity (16) to exercise realloc.
  PKCIni *ini = pkcIniInit ();
  PKChar key[32];
  PKChar val[32];

  for (PKInt i = 0; i < 32; ++i)
    {
      snprintf (key, sizeof (key), "key%d", i);
      snprintf (val, sizeof (val), "val%d", i);
      PK_ASSERT_EQ (pkcIniSet (ini, "s", key, val), PK_OK);
    }

  PK_ASSERT_EQ (keyCount (ini, "s"), (PKSize)32);

  for (PKInt i = 0; i < 32; ++i)
    {
      snprintf (key, sizeof (key), "key%d", i);
      snprintf (val, sizeof (val), "val%d", i);
      PK_ASSERT_STR_EQ (pkcIniGet (ini, "s", key), val);
    }

  pkcIniDestroy (ini);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Load
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testLoadBasic (void)
{
  // Parse a minimal INI file with one section and two keys.
  const PKChar *content = "[auth]\ntoken=abc\nexpiry=9999\n";
  PKChar path[256];
  writeTmpfile (path, sizeof (path), content);

  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniLoad (ini, path), PK_OK);
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "auth", "token"), "abc");
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "auth", "expiry"), "9999");
  pkcIniDestroy (ini);
  rmFile (path);
}

static PKVoid
testLoadCommentsIgnored (void)
{
  // Lines starting with ';' and '#' are ignored.
  const PKChar *content = "; this is a comment\n"
                          "# another comment\n"
                          "[s]\n"
                          "; inline section comment\n"
                          "key=value\n"
                          "# another inline comment\n";
  PKChar path[256];
  writeTmpfile (path, sizeof (path), content);

  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniLoad (ini, path), PK_OK);
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "s", "key"), "value");
  PK_ASSERT_EQ (keyCount (ini, "s"), (PKSize)1);
  pkcIniDestroy (ini);
  rmFile (path);
}

static PKVoid
testLoadWhitespaceTrimmed (void)
{
  // Leading and trailing whitespace is stripped from keys and values.
  // (Section names are stored as-is between '[' and ']'.)
  const PKChar *content = "[s]\n  key  =  value  \n";
  PKChar path[256];
  writeTmpfile (path, sizeof (path), content);

  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniLoad (ini, path), PK_OK);
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "s", "key"), "value");
  pkcIniDestroy (ini);
  rmFile (path);
}

static PKVoid
testLoadEmptyLinesIgnored (void)
{
  // Blank lines between entries do not cause errors.
  const PKChar *content = "\n[s]\n\nk=v\n\n";
  PKChar path[256];
  writeTmpfile (path, sizeof (path), content);

  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniLoad (ini, path), PK_OK);
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "s", "k"), "v");
  pkcIniDestroy (ini);
  rmFile (path);
}

static PKVoid
testLoadMultipleSections (void)
{
  // Keys in different sections are stored independently.
  const PKChar *content = "[global]\nendpoint=https://api.example.com\n\n"
                          "[oauth]\nclient_id=myid\n\n"
                          "[login]\naccess_token=tok\n";
  PKChar path[256];
  writeTmpfile (path, sizeof (path), content);

  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniLoad (ini, path), PK_OK);
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "global", "endpoint"),
                    "https://api.example.com");
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "oauth", "client_id"), "myid");
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "login", "access_token"), "tok");
  pkcIniDestroy (ini);
  rmFile (path);
}

static PKVoid
testLoadNonexistentFile (void)
{
  // Loading a path that does not exist must return PK_EFS.
  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniLoad (ini, "/tmp/p6a_no_such_ini_xyz987654.ini"),
                PK_EFS);
  pkcIniDestroy (ini);
}

static PKVoid
testLoadValuesKept (void)
{
  // Values keep ';', '#', quotes and '=' as written.
  const PKChar *content = "[global]\n"
                          "NAME=Party; fun # yes\n"
                          "QUOTED=\"quoted\"\n"
                          "EQUALS=a=b==\n";
  PKChar path[256];
  writeTmpfile (path, sizeof (path), content);

  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniLoad (ini, path), PK_OK);
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "global", "NAME"), "Party; fun # yes");
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "global", "QUOTED"), "\"quoted\"");
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "global", "EQUALS"), "a=b==");
  pkcIniDestroy (ini);
  rmFile (path);
}

static PKVoid
testLoadCrlf (void)
{
  // Windows line endings are not part of the values.
  const PKChar *content = "[s]\r\nk=v\r\n";
  PKChar path[256];
  writeTmpfile (path, sizeof (path), content);

  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniLoad (ini, path), PK_OK);
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "s", "k"), "v");
  pkcIniDestroy (ini);
  rmFile (path);
}

static PKVoid
testLoadMalformed (void)
{
  // Malformed files are errors, apart from a missing file.
  const PKChar *contents[] = {
    "[s]\nnoequals\n", // line without '='
    "k=v\n[s]\n",      // key outside of any section
    "[s]\nk=1\nk=2\n", // repeated key
    "[s\nk=v\n",       // unterminated section header
  };

  for (PKSize i = 0; i < sizeof (contents) / sizeof (contents[0]); ++i)
    {
      PKChar path[256];
      writeTmpfile (path, sizeof (path), contents[i]);

      PKCIni *ini = pkcIniInit ();
      PK_ASSERT_EQ (pkcIniLoad (ini, path), PK_EDATA);
      pkcIniDestroy (ini);
      rmFile (path);
    }
}

static PKVoid
testLoadUtf8Path (void)
{
  // Paths are UTF-8 on every platform, Windows included.
  PKChar path[512];
  snprintf (path, sizeof (path),
            "%s/p6a_ini_\xc5\xbelu\xc5\xa5ou\xc4\x8dk%d.ini", tmpDir (),
            (PKInt)getpid ());

  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniSet (ini, "s", "k", "v"), PK_OK);
  PK_ASSERT_EQ (pkcIniSave (ini, path), PK_OK);
  pkcIniDestroy (ini);

  PKCIni *loaded = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniLoad (loaded, path), PK_OK);
  PK_ASSERT_STR_EQ (pkcIniGet (loaded, "s", "k"), "v");
  pkcIniDestroy (loaded);

  rmFileUtf8 (path);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Case-insensitive lookup
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testCaseInsensitiveLookup (void)
{
  // Keys set in one case must be retrievable in any other case.
  PKCIni *ini = pkcIniInit ();

  PK_ASSERT_EQ (pkcIniSet (ini, "Auth", "Token", "abc123"), PK_OK);
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "AUTH", "TOKEN"), "abc123");
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "auth", "token"), "abc123");
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "Auth", "Token"), "abc123");

  pkcIniDestroy (ini);
}

static PKVoid
testCaseInsensitiveUpdate (void)
{
  // Updating a key via a differently-cased name overwrites in-place.
  PKCIni *ini = pkcIniInit ();

  PK_ASSERT_EQ (pkcIniSet (ini, "Auth", "Token", "old"), PK_OK);
  PK_ASSERT_EQ (pkcIniSet (ini, "AUTH", "TOKEN", "new"), PK_OK);

  PK_ASSERT_STR_EQ (pkcIniGet (ini, "auth", "token"), "new");
  PK_ASSERT_EQ (keyCount (ini, "auth"), (PKSize)1);

  pkcIniDestroy (ini);
}

static PKVoid
testCaseInsensitiveLoad (void)
{
  // An INI file with uppercase keys/sections is readable with CamelCase
  // lookups.
  const PKChar *content = "[GLOBAL]\nVERSION=1.0.0\nENDPOINT=https://x\n";
  PKChar path[256];
  writeTmpfile (path, sizeof (path), content);

  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniLoad (ini, path), PK_OK);
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "Global", "Version"), "1.0.0");
  PK_ASSERT_STR_EQ (pkcIniGet (ini, "Global", "Endpoint"), "https://x");
  pkcIniDestroy (ini);
  rmFile (path);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Save / Round-trip
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testSaveAndReload (void)
{
  // Build a document in memory, save it, reload it, verify values.
  PKChar path[256];
  makeTmpfile (path, sizeof (path));

  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniSet (ini, "s1", "a", "1"), PK_OK);
  PK_ASSERT_EQ (pkcIniSet (ini, "s1", "b", "2"), PK_OK);
  PK_ASSERT_EQ (pkcIniSet (ini, "s2", "c", "3"), PK_OK);
  PK_ASSERT_EQ (pkcIniSave (ini, path), PK_OK);
  pkcIniDestroy (ini);

  PKCIni *loaded = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniLoad (loaded, path), PK_OK);
  PK_ASSERT_STR_EQ (pkcIniGet (loaded, "s1", "a"), "1");
  PK_ASSERT_STR_EQ (pkcIniGet (loaded, "s1", "b"), "2");
  PK_ASSERT_STR_EQ (pkcIniGet (loaded, "s2", "c"), "3");
  pkcIniDestroy (loaded);

  rmFile (path);
}

static PKVoid
testRoundtripUpdate (void)
{
  // Set, update a key, save, reload — reloaded value must be updated.
  PKChar path[256];
  makeTmpfile (path, sizeof (path));

  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniSet (ini, "auth", "token", "old"), PK_OK);
  PK_ASSERT_EQ (pkcIniSet (ini, "auth", "token", "new"), PK_OK);
  PK_ASSERT_EQ (pkcIniSave (ini, path), PK_OK);
  pkcIniDestroy (ini);

  PKCIni *loaded = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniLoad (loaded, path), PK_OK);
  PK_ASSERT_STR_EQ (pkcIniGet (loaded, "auth", "token"), "new");
  PK_ASSERT_EQ (keyCount (loaded, "auth"), (PKSize)1);
  pkcIniDestroy (loaded);

  rmFile (path);
}

static PKVoid
testSaveFormat (void)
{
  // Header comment, sections in order, key=value lines.
  PKChar path[256];
  makeTmpfile (path, sizeof (path));

  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniSet (ini, "Global", "Version", "1.0.0"), PK_OK);
  PK_ASSERT_EQ (pkcIniSet (ini, "Login", "Access", ""), PK_OK);
  PK_ASSERT_EQ (pkcIniSet (ini, "global", "Endpoint", "https://x/y?a=b"),
                PK_OK);
  PK_ASSERT_EQ (pkcIniSave (ini, path), PK_OK);
  pkcIniDestroy (ini);

  PKChar text[1024];
  readFile (path, text, sizeof (text));
  PK_ASSERT_STR_EQ (
      text, "; Partoska.com Configuration File\n"
            "; Auto-generated by p6a utility v" PK_VERSION_STRING ".\n"
            "; Do not modify. More info at <https://www.partoska.com/p6a>\n"
            "\n"
            "[Global]\n"
            "Version=1.0.0\n"
            "Endpoint=https://x/y?a=b\n"
            "\n"
            "[Login]\n"
            "Access=\n");

  rmFile (path);
}

static PKVoid
testRoundtripLiteral (void)
{
  // Values with comment characters survive a save and a load.
  PKChar path[256];
  makeTmpfile (path, sizeof (path));

  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniSet (ini, "global", "NAME", "Party; fun # yes"), PK_OK);
  PK_ASSERT_EQ (pkcIniSet (ini, "media:a1", "NAME", "IMG_0001.jpg"), PK_OK);
  PK_ASSERT_EQ (pkcIniSave (ini, path), PK_OK);
  pkcIniDestroy (ini);

  PKCIni *loaded = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniLoad (loaded, path), PK_OK);
  PK_ASSERT_STR_EQ (pkcIniGet (loaded, "global", "NAME"), "Party; fun # yes");
  PK_ASSERT_STR_EQ (pkcIniGet (loaded, "media:a1", "NAME"), "IMG_0001.jpg");
  pkcIniDestroy (loaded);

  rmFile (path);
}

static PKVoid
testSetLineBreakRejected (void)
{
  // Values cannot span lines; keys must be valid.
  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniSet (ini, "s", "k", "a\nb"), PK_EARG);
  PK_ASSERT_EQ (pkcIniSet (ini, "s", "k", "a\rb"), PK_EARG);
  PK_ASSERT_EQ (pkcIniSet (ini, "s", "a=b", "v"), PK_EARG);
  PK_ASSERT_EQ (pkcIniSet (ini, "s", "", "v"), PK_EARG);
  PK_ASSERT (pkcIniGet (ini, "s", "k") == NULL);
  pkcIniDestroy (ini);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Search
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testHasValue (void)
{
  // Keys match in any case and in any section, values exactly.
  PKCIni *ini = pkcIniInit ();
  PK_ASSERT_EQ (pkcIniSet (ini, "global", "NAME", "Party"), PK_OK);
  PK_ASSERT_EQ (pkcIniSet (ini, "media:a1", "NAME", "IMG_0001.jpg"), PK_OK);
  PK_ASSERT_EQ (pkcIniSet (ini, "media:a1", "TYPE", "IMG_0002.jpg"), PK_OK);

  PK_ASSERT (pkcIniHasValue (ini, "NAME", "IMG_0001.jpg"));
  PK_ASSERT (pkcIniHasValue (ini, "name", "Party"));
  PK_ASSERT (!pkcIniHasValue (ini, "NAME", "img_0001.jpg"));
  PK_ASSERT (!pkcIniHasValue (ini, "NAME", "IMG_0002.jpg"));
  pkcIniDestroy (ini);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Main
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
main (void)
{
  PK_SUITE ("ini - init / destroy");
  PK_RUN (testInitEmpty);
  PK_RUN (testDestroyNull);

  PK_SUITE ("ini - set / get");
  PK_RUN (testSetAndGet);
  PK_RUN (testGetMissingKey);
  PK_RUN (testUpdateExistingKey);
  PK_RUN (testMultipleSections);
  PK_RUN (testManyEntriesGrow);

  PK_SUITE ("ini - load");
  PK_RUN (testLoadBasic);
  PK_RUN (testLoadCommentsIgnored);
  PK_RUN (testLoadWhitespaceTrimmed);
  PK_RUN (testLoadEmptyLinesIgnored);
  PK_RUN (testLoadMultipleSections);
  PK_RUN (testLoadNonexistentFile);
  PK_RUN (testLoadValuesKept);
  PK_RUN (testLoadCrlf);
  PK_RUN (testLoadMalformed);
  PK_RUN (testLoadUtf8Path);

  PK_SUITE ("ini - case-insensitive lookup");
  PK_RUN (testCaseInsensitiveLookup);
  PK_RUN (testCaseInsensitiveUpdate);
  PK_RUN (testCaseInsensitiveLoad);

  PK_SUITE ("ini - save / round-trip");
  PK_RUN (testSaveAndReload);
  PK_RUN (testRoundtripUpdate);
  PK_RUN (testSaveFormat);
  PK_RUN (testRoundtripLiteral);
  PK_RUN (testSetLineBreakRejected);

  PK_SUITE ("ini - search");
  PK_RUN (testHasValue);

  PK_SUMMARY ();
}
