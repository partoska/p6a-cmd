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

#include "config.h"
#include "test_runner.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Helpers
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
  snprintf (buf, size, "%s/p6a_cfg_test_XXXXXX", tmp);
  PLInt fd = mkstemp (buf);
  if (fd >= 0)
    {
      close (fd);
    }
#endif
}

static void
rmFile (const PLChar *path)
{
  unlink (path);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Init / Destroy / Check
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testInitNotNull (void)
{
  PLCfg *cfg = plCfgInit ();
  PL_ASSERT (cfg != NULL);
  PL_ASSERT (cfg->glob != NULL);
  PL_ASSERT (cfg->oauth != NULL);
  PL_ASSERT (cfg->login != NULL);
  plCfgDestroy (cfg);
}

static void
testInitDefaultsPopulated (void)
{
  // After init, global and OAuth fields are filled with built-in defaults.
  PLCfg *cfg = plCfgInit ();

  PL_ASSERT (cfg->glob->version != NULL);
  PL_ASSERT (cfg->glob->endpoint != NULL);

  PL_ASSERT (cfg->oauth->client != NULL);
  PL_ASSERT (cfg->oauth->redirect != NULL);
  PL_ASSERT (cfg->oauth->authorize != NULL);
  PL_ASSERT (cfg->oauth->token != NULL);
  PL_ASSERT (cfg->oauth->scope != NULL);

  plCfgDestroy (cfg);
}

static void
testInitLoginEmpty (void)
{
  // Login fields are NULL / zero before any credentials are stored.
  PLCfg *cfg = plCfgInit ();
  PL_ASSERT (cfg->login->access == NULL);
  PL_ASSERT (cfg->login->refresh == NULL);
  PL_ASSERT_EQ (cfg->login->expires, (PLTime)0);
  plCfgDestroy (cfg);
}

static void
testCheckFreshCfg (void)
{
  // A freshly initialized config passes the completeness check.
  PLCfg *cfg = plCfgInit ();
  PL_ASSERT_EQ (plCfgCheck (cfg), PL_TRUE);
  plCfgDestroy (cfg);
}

static void
testCheckNull (void)
{
  PL_ASSERT_EQ (plCfgCheck (NULL), PL_FALSE);
}

static void
testDestroyNull (void)
{
  // plCfgDestroy(NULL) must not crash.
  plCfgDestroy (NULL);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - SetLogin / UnsetLogin
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testSetLogin (void)
{
  PLCfg *cfg = plCfgInit ();

  PL_ASSERT_EQ (plCfgSetLogin (cfg, "access_tok", "refresh_tok", 9999999),
                PL_EOK);
  PL_ASSERT_STR_EQ (cfg->login->access, "access_tok");
  PL_ASSERT_STR_EQ (cfg->login->refresh, "refresh_tok");
  PL_ASSERT_EQ (cfg->login->expires, (PLTime)9999999);

  plCfgDestroy (cfg);
}

static void
testSetLoginReplacesExisting (void)
{
  // Calling SetLogin twice replaces the previous tokens.
  PLCfg *cfg = plCfgInit ();

  PL_ASSERT_EQ (plCfgSetLogin (cfg, "old_access", "old_refresh", 1111),
                PL_EOK);
  PL_ASSERT_EQ (plCfgSetLogin (cfg, "new_access", "new_refresh", 2222),
                PL_EOK);

  PL_ASSERT_STR_EQ (cfg->login->access, "new_access");
  PL_ASSERT_STR_EQ (cfg->login->refresh, "new_refresh");
  PL_ASSERT_EQ (cfg->login->expires, (PLTime)2222);

  plCfgDestroy (cfg);
}

static void
testSetLoginNullAccess (void)
{
  PLCfg *cfg = plCfgInit ();
  PL_ASSERT_EQ (plCfgSetLogin (cfg, NULL, "refresh_tok", 0), PL_EARG);
  plCfgDestroy (cfg);
}

static void
testSetLoginNullRefresh (void)
{
  PLCfg *cfg = plCfgInit ();
  PL_ASSERT_EQ (plCfgSetLogin (cfg, "access_tok", NULL, 0), PL_EARG);
  plCfgDestroy (cfg);
}

static void
testUnsetLogin (void)
{
  PLCfg *cfg = plCfgInit ();

  PL_ASSERT_EQ (plCfgSetLogin (cfg, "access_tok", "refresh_tok", 9999),
                PL_EOK);
  PL_ASSERT_EQ (plCfgUnsetLogin (cfg), PL_EOK);

  PL_ASSERT (cfg->login->access == NULL);
  PL_ASSERT (cfg->login->refresh == NULL);
  PL_ASSERT_EQ (cfg->login->expires, (PLTime)0);

  plCfgDestroy (cfg);
}

static void
testUnsetLoginIdempotent (void)
{
  // Calling UnsetLogin on a config with no login set must not crash.
  PLCfg *cfg = plCfgInit ();
  PL_ASSERT_EQ (plCfgUnsetLogin (cfg), PL_EOK);
  PL_ASSERT_EQ (plCfgUnsetLogin (cfg), PL_EOK);
  plCfgDestroy (cfg);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Load
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testLoadNullFile (void)
{
  PLCfg *cfg = plCfgInit ();
  PL_ASSERT_EQ (plCfgLoad (cfg, NULL), PL_EARG);
  plCfgDestroy (cfg);
}

static void
testLoadNonexistentFile (void)
{
  PLCfg *cfg = plCfgInit ();
  PL_ASSERT_EQ (plCfgLoad (cfg, "/tmp/p6a_no_such_config_xyz987654.ini"),
                PL_EFS);
  plCfgDestroy (cfg);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Save / Load round-trip
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testRoundtripWithLogin (void)
{
  // Save a config with tokens; load it into a fresh config and verify.
  PLChar path[256];
  makeTmpfile (path, sizeof (path));

  PLCfg *cfg = plCfgInit ();
  PL_ASSERT_EQ (plCfgSetLogin (cfg, "my_access", "my_refresh", 1234567890),
                PL_EOK);
  PL_ASSERT_EQ (plCfgSave (cfg, path), PL_EOK);
  plCfgDestroy (cfg);

  PLCfg *loaded = plCfgInit ();
  PL_ASSERT_EQ (plCfgLoad (loaded, path), PL_EOK);

  PL_ASSERT_STR_EQ (loaded->login->access, "my_access");
  PL_ASSERT_STR_EQ (loaded->login->refresh, "my_refresh");
  PL_ASSERT_EQ (loaded->login->expires, (PLTime)1234567890);

  plCfgDestroy (loaded);
  rmFile (path);
}

static void
testRoundtripPreservesGlob (void)
{
  // Global fields (version, endpoint) survive a save/load cycle.
  PLChar path[256];
  makeTmpfile (path, sizeof (path));

  PLCfg *cfg = plCfgInit ();
  PLChar savedVer[64];
  PLChar savedEp[128];
  strncpy (savedVer, cfg->glob->version, sizeof (savedVer) - 1);
  savedVer[sizeof (savedVer) - 1] = '\0';
  strncpy (savedEp, cfg->glob->endpoint, sizeof (savedEp) - 1);
  savedEp[sizeof (savedEp) - 1] = '\0';

  PL_ASSERT_EQ (plCfgSave (cfg, path), PL_EOK);
  plCfgDestroy (cfg);

  PLCfg *loaded = plCfgInit ();
  PL_ASSERT_EQ (plCfgLoad (loaded, path), PL_EOK);

  PL_ASSERT_STR_EQ (loaded->glob->version, savedVer);
  PL_ASSERT_STR_EQ (loaded->glob->endpoint, savedEp);
  PL_ASSERT_EQ (plCfgCheck (loaded), PL_TRUE);

  plCfgDestroy (loaded);
  rmFile (path);
}

static void
testRoundtripNoLogin (void)
{
  // Save with no login tokens; loaded config still passes plCfgCheck.
  PLChar path[256];
  makeTmpfile (path, sizeof (path));

  PLCfg *cfg = plCfgInit ();
  PL_ASSERT_EQ (plCfgSave (cfg, path), PL_EOK);
  plCfgDestroy (cfg);

  PLCfg *loaded = plCfgInit ();
  PL_ASSERT_EQ (plCfgLoad (loaded, path), PL_EOK);
  PL_ASSERT_EQ (plCfgCheck (loaded), PL_TRUE);
  PL_ASSERT_EQ (loaded->login->expires, (PLTime)0);

  plCfgDestroy (loaded);
  rmFile (path);
}

static void
testRoundtripUnsetAfterSet (void)
{
  // Set tokens, save, load, unset -> check fields are cleared.
  PLChar path[256];
  makeTmpfile (path, sizeof (path));

  PLCfg *cfg = plCfgInit ();
  PL_ASSERT_EQ (plCfgSetLogin (cfg, "tok_a", "tok_r", 555), PL_EOK);
  PL_ASSERT_EQ (plCfgSave (cfg, path), PL_EOK);
  plCfgDestroy (cfg);

  PLCfg *loaded = plCfgInit ();
  PL_ASSERT_EQ (plCfgLoad (loaded, path), PL_EOK);
  PL_ASSERT_STR_EQ (loaded->login->access, "tok_a");

  PL_ASSERT_EQ (plCfgUnsetLogin (loaded), PL_EOK);
  PL_ASSERT (loaded->login->access == NULL);
  PL_ASSERT (loaded->login->refresh == NULL);
  PL_ASSERT_EQ (loaded->login->expires, (PLTime)0);

  plCfgDestroy (loaded);
  rmFile (path);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Main
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
main (void)
{
  PL_SUITE ("config - init / check / destroy");
  PL_RUN (testInitNotNull);
  PL_RUN (testInitDefaultsPopulated);
  PL_RUN (testInitLoginEmpty);
  PL_RUN (testCheckFreshCfg);
  PL_RUN (testCheckNull);
  PL_RUN (testDestroyNull);

  PL_SUITE ("config - SetLogin / UnsetLogin");
  PL_RUN (testSetLogin);
  PL_RUN (testSetLoginReplacesExisting);
  PL_RUN (testSetLoginNullAccess);
  PL_RUN (testSetLoginNullRefresh);
  PL_RUN (testUnsetLogin);
  PL_RUN (testUnsetLoginIdempotent);

  PL_SUITE ("config - load");
  PL_RUN (testLoadNullFile);
  PL_RUN (testLoadNonexistentFile);

  PL_SUITE ("config - save / round-trip");
  PL_RUN (testRoundtripWithLogin);
  PL_RUN (testRoundtripPreservesGlob);
  PL_RUN (testRoundtripNoLogin);
  PL_RUN (testRoundtripUnsetAfterSet);

  PL_SUMMARY ();
}
