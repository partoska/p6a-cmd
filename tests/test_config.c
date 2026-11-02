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

#include "client.h"
#include "config.h"
#include "pktypes.h"
#include "plat.h"
#include "test_runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Variables
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Records the failures of the calls under test. */
static PKClient *client;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Helpers
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
  snprintf (buf, size, "%s/p6a_cfg_test_XXXXXX", tmp);
  PKInt fd = mkstemp (buf);
  if (fd >= 0)
    {
      close (fd);
    }
#endif
}

static PKVoid
rmFile (const PKChar *path)
{
  unlink (path);
}

/* Write text to a fresh temp file and return its path in buf. */
static PKVoid
writeTmpfile (PKChar *buf, PKSize size, const PKChar *text)
{
  makeTmpfile (buf, size);
  FILE *f = fopen (buf, "wb");
  if (f)
    {
      fputs (text, f);
      fclose (f);
    }
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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Init / Destroy / Check
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testInitNotNull (void)
{
  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT (cfg != NULL);
  PK_ASSERT (cfg->glob != NULL);
  PK_ASSERT (cfg->oauth != NULL);
  PK_ASSERT (cfg->login != NULL);
  pkCfgDestroy (cfg);
}

static PKVoid
testInitDefaultsPopulated (void)
{
  // After init, global and OAuth fields are filled with built-in defaults.
  PKCfg *cfg = pkCfgInit ();

  PK_ASSERT (cfg->glob->version != NULL);
  PK_ASSERT (cfg->glob->endpoint != NULL);

  PK_ASSERT (cfg->oauth->client != NULL);
  PK_ASSERT (cfg->oauth->device != NULL);
  PK_ASSERT (cfg->oauth->token != NULL);
  PK_ASSERT (cfg->oauth->scope != NULL);

  pkCfgDestroy (cfg);
}

static PKVoid
testInitLoginEmpty (void)
{
  // Login fields are NULL / zero before any credentials are stored.
  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT (cfg->login->access == NULL);
  PK_ASSERT (cfg->login->refresh == NULL);
  PK_ASSERT_EQ (cfg->login->expires, (PKTime)0);
  pkCfgDestroy (cfg);
}

static PKVoid
testCheckFreshCfg (void)
{
  // A freshly initialized config passes the completeness check.
  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgCheck (cfg), PK_TRUE);
  pkCfgDestroy (cfg);
}

static PKVoid
testCheckNull (void)
{
  PK_ASSERT_EQ (pkCfgCheck (NULL), PK_FALSE);
}

static PKVoid
testDestroyNull (void)
{
  // pkCfgDestroy(NULL) must not crash.
  pkCfgDestroy (NULL);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - SetLogin / UnsetLogin
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testSetLogin (void)
{
  PKCfg *cfg = pkCfgInit ();

  PK_ASSERT_EQ (pkCfgSetLogin (cfg, "access_tok", "refresh_tok", 9999999),
                PK_OK);
  PK_ASSERT_STR_EQ (cfg->login->access, "access_tok");
  PK_ASSERT_STR_EQ (cfg->login->refresh, "refresh_tok");
  PK_ASSERT_EQ (cfg->login->expires, (PKTime)9999999);

  pkCfgDestroy (cfg);
}

static PKVoid
testSetLoginReplacesExisting (void)
{
  // Calling SetLogin twice replaces the previous tokens.
  PKCfg *cfg = pkCfgInit ();

  PK_ASSERT_EQ (pkCfgSetLogin (cfg, "old_access", "old_refresh", 1111), PK_OK);
  PK_ASSERT_EQ (pkCfgSetLogin (cfg, "new_access", "new_refresh", 2222), PK_OK);

  PK_ASSERT_STR_EQ (cfg->login->access, "new_access");
  PK_ASSERT_STR_EQ (cfg->login->refresh, "new_refresh");
  PK_ASSERT_EQ (cfg->login->expires, (PKTime)2222);

  pkCfgDestroy (cfg);
}

static PKVoid
testSetLoginNullAccess (void)
{
  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgSetLogin (cfg, NULL, "refresh_tok", 0), PK_EARG);
  pkCfgDestroy (cfg);
}

static PKVoid
testSetLoginNullRefresh (void)
{
  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgSetLogin (cfg, "access_tok", NULL, 0), PK_EARG);
  pkCfgDestroy (cfg);
}

static PKVoid
testUnsetLogin (void)
{
  PKCfg *cfg = pkCfgInit ();

  PK_ASSERT_EQ (pkCfgSetLogin (cfg, "access_tok", "refresh_tok", 9999), PK_OK);
  pkCfgUnsetLogin (cfg);

  PK_ASSERT (cfg->login->access == NULL);
  PK_ASSERT (cfg->login->refresh == NULL);
  PK_ASSERT_EQ (cfg->login->expires, (PKTime)0);

  pkCfgDestroy (cfg);
}

static PKVoid
testUnsetLoginIdempotent (void)
{
  // Calling UnsetLogin on a config with no login set must not crash.
  PKCfg *cfg = pkCfgInit ();
  pkCfgUnsetLogin (cfg);
  pkCfgUnsetLogin (cfg);
  pkCfgDestroy (cfg);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Load
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testLoadNullFile (void)
{
  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgLoad (client, cfg, NULL), PK_EARG);
  pkCfgDestroy (cfg);
}

static PKVoid
testLoadNonexistentFile (void)
{
  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT_EQ (
      pkCfgLoad (client, cfg, "/tmp/p6a_no_such_config_xyz987654.ini"),
      PK_EFS);
  pkCfgDestroy (cfg);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Save / Load round-trip
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testRoundtripWithLogin (void)
{
  // Save a config with tokens; load it into a fresh config and verify.
  PKChar path[256];
  makeTmpfile (path, sizeof (path));

  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgSetLogin (cfg, "my_access", "my_refresh", 1234567890),
                PK_OK);
  PK_ASSERT_EQ (pkCfgSave (client, cfg, path), PK_OK);
  pkCfgDestroy (cfg);

  PKCfg *loaded = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgLoad (client, loaded, path), PK_OK);

  PK_ASSERT_STR_EQ (loaded->login->access, "my_access");
  PK_ASSERT_STR_EQ (loaded->login->refresh, "my_refresh");
  PK_ASSERT_EQ (loaded->login->expires, (PKTime)1234567890);

  pkCfgDestroy (loaded);
  rmFile (path);
}

static PKVoid
testRoundtripPreservesGlob (void)
{
  // Global fields (version, endpoint) survive a save/load cycle.
  PKChar path[256];
  makeTmpfile (path, sizeof (path));

  PKCfg *cfg = pkCfgInit ();
  PKChar savedVer[64];
  PKChar savedEp[128];
  strncpy (savedVer, cfg->glob->version, sizeof (savedVer) - 1);
  savedVer[sizeof (savedVer) - 1] = '\0';
  strncpy (savedEp, cfg->glob->endpoint, sizeof (savedEp) - 1);
  savedEp[sizeof (savedEp) - 1] = '\0';

  PK_ASSERT_EQ (pkCfgSave (client, cfg, path), PK_OK);
  pkCfgDestroy (cfg);

  PKCfg *loaded = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgLoad (client, loaded, path), PK_OK);

  PK_ASSERT_STR_EQ (loaded->glob->version, savedVer);
  PK_ASSERT_STR_EQ (loaded->glob->endpoint, savedEp);
  PK_ASSERT_EQ (pkCfgCheck (loaded), PK_TRUE);

  pkCfgDestroy (loaded);
  rmFile (path);
}

static PKVoid
testRoundtripNoLogin (void)
{
  // Save with no login tokens; loaded config still passes pkCfgCheck.
  PKChar path[256];
  makeTmpfile (path, sizeof (path));

  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgSave (client, cfg, path), PK_OK);
  pkCfgDestroy (cfg);

  PKCfg *loaded = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgLoad (client, loaded, path), PK_OK);
  PK_ASSERT_EQ (pkCfgCheck (loaded), PK_TRUE);
  PK_ASSERT_EQ (loaded->login->expires, (PKTime)0);

  pkCfgDestroy (loaded);
  rmFile (path);
}

static PKVoid
testRoundtripUnsetAfterSet (void)
{
  // Set tokens, save, load, unset -> check fields are cleared.
  PKChar path[256];
  makeTmpfile (path, sizeof (path));

  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgSetLogin (cfg, "tok_a", "tok_r", 555), PK_OK);
  PK_ASSERT_EQ (pkCfgSave (client, cfg, path), PK_OK);
  pkCfgDestroy (cfg);

  PKCfg *loaded = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgLoad (client, loaded, path), PK_OK);
  PK_ASSERT_STR_EQ (loaded->login->access, "tok_a");

  pkCfgUnsetLogin (loaded);
  PK_ASSERT (loaded->login->access == NULL);
  PK_ASSERT (loaded->login->refresh == NULL);
  PK_ASSERT_EQ (loaded->login->expires, (PKTime)0);

  pkCfgDestroy (loaded);
  rmFile (path);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Device endpoint
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testDeviceRoundtrip (void)
{
  // An explicit Device endpoint survives a save/load cycle.
  PKChar path[256];
  makeTmpfile (path, sizeof (path));

  PKCfg *cfg = pkCfgInit ();
  const PKChar *device = "https://example.com/custom/device";
  pkMemFree (cfg->oauth->device);
  cfg->oauth->device = pkStrDup (device, pkStrLen (device));
  PK_ASSERT_EQ (pkCfgSave (client, cfg, path), PK_OK);
  pkCfgDestroy (cfg);

  PKCfg *loaded = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgLoad (client, loaded, path), PK_OK);
  PK_ASSERT_STR_EQ (loaded->oauth->device,
                    "https://example.com/custom/device");

  pkCfgDestroy (loaded);
  rmFile (path);
}

static PKVoid
testDeviceDefaultWhenMissing (void)
{
  // A config written before the device flow has no Device key and gets the
  // built-in default.
  PKChar path[256];
  writeTmpfile (path, sizeof (path),
                "[OAuth]\n"
                "Token=https://example.com/auth/v1/oauth/token\n");

  PKCfg *def = pkCfgInit ();
  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgLoad (client, cfg, path), PK_OK);
  PK_ASSERT_STR_EQ (cfg->oauth->device, def->oauth->device);
  PK_ASSERT_EQ (pkCfgCheck (cfg), PK_TRUE);

  pkCfgDestroy (cfg);
  pkCfgDestroy (def);
  rmFile (path);
}

static PKVoid
testLoadCaseInsensitive (void)
{
  // Section and key names match in any case.
  PKChar path[256];
  writeTmpfile (path, sizeof (path),
                "[LOGIN]\naccess=a1\nREFRESH=r1\nexpires=42\n");

  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgLoad (client, cfg, path), PK_OK);
  PK_ASSERT_STR_EQ (cfg->login->access, "a1");
  PK_ASSERT_STR_EQ (cfg->login->refresh, "r1");
  PK_ASSERT_EQ (cfg->login->expires, (PKTime)42);

  pkCfgDestroy (cfg);
  rmFile (path);
}

static PKVoid
testLoadMalformed (void)
{
  // A malformed file is an error, not a missing one, and changes nothing.
  PKChar path[256];
  writeTmpfile (path, sizeof (path), "[Login]\nAccess=a1\nAccess=a2\n");

  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgLoad (client, cfg, path), PK_EDATA);
  PK_ASSERT (cfg->login->access == NULL);

  pkCfgDestroy (cfg);
  rmFile (path);
}

static PKVoid
testSaveFormat (void)
{
  // Header comment, then every key in a fixed order.
  PKChar path[256];
  makeTmpfile (path, sizeof (path));

  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgSetLogin (cfg, "a1", "r1", 42), PK_OK);
  PK_ASSERT_EQ (pkCfgSave (client, cfg, path), PK_OK);

  PKChar expected[2048];
  snprintf (expected, sizeof (expected),
            "; Partoska.com Configuration File\n"
            "; Auto-generated by p6a utility v" PK_VERSION_STRING ".\n"
            "; Do not modify. More info at <https://www.partoska.com/p6a>\n"
            "\n"
            "[Global]\n"
            "Version=" PK_VERSION_STRING "\n"
            "Endpoint=%s\n"
            "\n"
            "[OAuth]\n"
            "Client=%s\n"
            "Device=%s\n"
            "Token=%s\n"
            "Scope=%s\n"
            "\n"
            "[Login]\n"
            "Access=a1\n"
            "Refresh=r1\n"
            "Expires=42\n",
            cfg->glob->endpoint, cfg->oauth->client, cfg->oauth->device,
            cfg->oauth->token, cfg->oauth->scope);

  PKChar text[2048];
  readFile (path, text, sizeof (text));
  PK_ASSERT_STR_EQ (text, expected);

  pkCfgDestroy (cfg);
  rmFile (path);
}

static PKVoid
testSaveLineBreakRejected (void)
{
  // A value spanning lines would corrupt the file.
  PKChar path[256];
  makeTmpfile (path, sizeof (path));

  PKCfg *cfg = pkCfgInit ();
  PK_ASSERT_EQ (pkCfgSetLogin (cfg, "a1\nAccess=evil", "r1", 42), PK_OK);
  PK_ASSERT_EQ (pkCfgSave (client, cfg, path), PK_EARG);

  pkCfgDestroy (cfg);
  rmFile (path);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Main
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
main (void)
{
  client = pkInit ();
  PK_SUITE ("config - init / check / destroy");
  PK_RUN (testInitNotNull);
  PK_RUN (testInitDefaultsPopulated);
  PK_RUN (testInitLoginEmpty);
  PK_RUN (testCheckFreshCfg);
  PK_RUN (testCheckNull);
  PK_RUN (testDestroyNull);

  PK_SUITE ("config - SetLogin / UnsetLogin");
  PK_RUN (testSetLogin);
  PK_RUN (testSetLoginReplacesExisting);
  PK_RUN (testSetLoginNullAccess);
  PK_RUN (testSetLoginNullRefresh);
  PK_RUN (testUnsetLogin);
  PK_RUN (testUnsetLoginIdempotent);

  PK_SUITE ("config - load");
  PK_RUN (testLoadNullFile);
  PK_RUN (testLoadNonexistentFile);
  PK_RUN (testLoadCaseInsensitive);
  PK_RUN (testLoadMalformed);

  PK_SUITE ("config - save / round-trip");
  PK_RUN (testRoundtripWithLogin);
  PK_RUN (testRoundtripPreservesGlob);
  PK_RUN (testRoundtripNoLogin);
  PK_RUN (testRoundtripUnsetAfterSet);
  PK_RUN (testSaveFormat);
  PK_RUN (testSaveLineBreakRejected);

  PK_SUITE ("config - device endpoint");
  PK_RUN (testDeviceRoundtrip);
  PK_RUN (testDeviceDefaultWhenMissing);

  pkDestroy (client);
  PK_SUMMARY ();
}
