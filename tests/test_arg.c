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

#include "arg.h"
#include "test_runner.h"
#include <stdio.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Error cases
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testNoArgs (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a" };
  PK_ASSERT_EQ (pkcArgParse (&args, 1, argv), PK_EARG);
}

static PKVoid
testUnknownCommand (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "notacommand" };
  PK_ASSERT_EQ (pkcArgParse (&args, 2, argv), PK_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - help / version
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testHelp (void)
{
  PKCArg args;
  PKChar *argvHelp[] = { "p6a", "help" };
  PKChar *argvShort[] = { "p6a", "-h" };
  PKChar *argvLong[] = { "p6a", "--help" };

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argvHelp), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CHELP);

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argvShort), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CHELP);

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argvLong), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CHELP);
}

static PKVoid
testVersion (void)
{
  PKCArg args;
  PKChar *argvVer[] = { "p6a", "version" };
  PKChar *argvShort[] = { "p6a", "-V" };
  PKChar *argvLong[] = { "p6a", "--version" };

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argvVer), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CVERSION);

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argvShort), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CVERSION);

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argvLong), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CVERSION);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - login
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testLoginBare (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "login" };

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CLOGIN);
  PK_ASSERT (args.c.login.path == NULL);
  PK_ASSERT (args.dir == NULL);
}

static PKVoid
testLoginImport (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "login", "--import", "/path/to/cfg.ini" };

  PK_ASSERT_EQ (pkcArgParse (&args, 4, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CLOGIN);
  PK_ASSERT_STR_EQ (args.c.login.path, "/path/to/cfg.ini");
}

static PKVoid
testLoginDir (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "login", "--dir", "/custom/dir" };

  PK_ASSERT_EQ (pkcArgParse (&args, 4, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CLOGIN);
  PK_ASSERT_STR_EQ (args.dir, "/custom/dir");
}

static PKVoid
testLoginFormat (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "login" };
  PKChar *argvJson[] = { "p6a", "login", "-F", "json" };
  PKChar *argvCsv[] = { "p6a", "login", "--format", "csv" };
  PKChar *argvOne[] = { "p6a", "login", "-1", "-i", "cfg.ini" };
  PKChar *argvBad[] = { "p6a", "login", "-F", "pdf" };

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argv), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTPLAIN);
  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvJson), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CLOGIN);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTJSON);
  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvCsv), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTCSV);
  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvOne), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTONE);
  PK_ASSERT_STR_EQ (args.c.login.path, "cfg.ini");
  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvBad), PK_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - logout
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testLogout (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "logout" };
  PKChar *argvExtra[] = { "p6a", "logout", "unexpected" };

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CLOGOUT);

  // Extra positional args should be rejected.
  PK_ASSERT_EQ (pkcArgParse (&args, 3, argvExtra), PK_EARG);
}

static PKVoid
testLogoutDir (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "logout", "-D", "/custom/dir" };
  PKChar *argvLong[] = { "p6a", "logout", "--dir=/custom/dir" };
  PKChar *argvBad[] = { "p6a", "logout", "-q", "x" };

  PK_ASSERT_EQ (pkcArgParse (&args, 4, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CLOGOUT);
  PK_ASSERT_STR_EQ (args.dir, "/custom/dir");

  PK_ASSERT_EQ (pkcArgParse (&args, 3, argvLong), PK_OK);
  PK_ASSERT_STR_EQ (args.dir, "/custom/dir");

  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvBad), PK_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - status
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testStatus (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "status" };
  PKChar *argvDir[] = { "p6a", "status", "--dir", "/custom/dir" };
  PKChar *argvExtra[] = { "p6a", "status", "unexpected" };
  PKChar *argvAfter[] = { "p6a", "status", "-D", "/custom/dir", "extra" };
  PKChar *argvBad[] = { "p6a", "status", "-o" };
  PKChar *argvJson[] = { "p6a", "status", "-F", "json" };
  PKChar *argvCsv[] = { "p6a", "status", "--format=csv" };
  PKChar *argvOne[] = { "p6a", "status", "-1" };
  PKChar *argvFmtBad[] = { "p6a", "status", "-F", "svg" };

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CSTATUS);
  PK_ASSERT (args.dir == NULL);

  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvDir), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CSTATUS);
  PK_ASSERT_STR_EQ (args.dir, "/custom/dir");

  // Positional arguments, before or after the options, and unknown options
  // are rejected.
  PK_ASSERT_EQ (pkcArgParse (&args, 3, argvExtra), PK_EARG);
  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvAfter), PK_EARG);
  PK_ASSERT_EQ (pkcArgParse (&args, 3, argvBad), PK_EARG);

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argv), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTPLAIN);
  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvJson), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTJSON);
  PK_ASSERT_EQ (pkcArgParse (&args, 3, argvCsv), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTCSV);
  PK_ASSERT_EQ (pkcArgParse (&args, 3, argvOne), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTONE);
  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvFmtBad), PK_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - sync
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testSync (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "sync", "-t", "/photos" };
  PKChar *argvNoTgt[] = { "p6a", "sync" };
  PKChar *argvOwn[] = { "p6a", "sync", "-t", "/photos", "--owner-only" };
  PKChar *argvFav[] = { "p6a", "sync", "-t", "/photos", "--favorite-only" };
  PKChar *argvBoth[] = { "p6a", "sync", "-t", "/photos", "-o", "-f" };

  PK_ASSERT_EQ (pkcArgParse (&args, 4, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CSYNC);
  PK_ASSERT_STR_EQ (args.c.sync.target, "/photos");
  PK_ASSERT_EQ (args.flags & PKC_FOWN, (PKDword)0);
  PK_ASSERT_EQ (args.flags & PKC_FFAV, (PKDword)0);

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argvNoTgt), PK_EARG);

  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvOwn), PK_OK);
  PK_ASSERT (args.flags & PKC_FOWN);

  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvFav), PK_OK);
  PK_ASSERT (args.flags & PKC_FFAV);

  PK_ASSERT_EQ (pkcArgParse (&args, 6, argvBoth), PK_OK);
  PK_ASSERT (args.flags & PKC_FOWN);
  PK_ASSERT (args.flags & PKC_FFAV);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - list
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testList (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "list" };
  PKChar *argvQuery[] = { "p6a", "list", "-q", "Birthday" };
  PKChar *argvOne[] = { "p6a", "list", "-1" };
  PKChar *argvJson[] = { "p6a", "list", "-F", "json" };
  PKChar *argvCsv[] = { "p6a", "list", "-F", "csv" };

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CLIST);
  PK_ASSERT (args.c.list.query == NULL);

  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvQuery), PK_OK);
  PK_ASSERT_STR_EQ (args.c.list.query, "Birthday");

  PK_ASSERT_EQ (pkcArgParse (&args, 3, argvOne), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTONE);

  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvJson), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTJSON);

  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvCsv), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTCSV);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - create
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testCreate (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "create", "-n", "My Event" };
  PKChar *argvNoName[] = { "p6a", "create" };

  PK_ASSERT_EQ (pkcArgParse (&args, 4, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CCREATE);
  PK_ASSERT_STR_EQ (args.c.create.name, "My Event");

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argvNoName), PK_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - update
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testUpdate (void)
{
  PKCArg args;
  PKChar *argvRename[]
      = { "p6a", "update", "-e", "event-uuid", "-n", "New Name" };
  PKChar *argvPub[] = { "p6a", "update", "-e", "event-uuid", "--public" };
  PKChar *argvPriv[] = { "p6a", "update", "-e", "event-uuid", "--private" };
  PKChar *argvFav[] = { "p6a", "update", "-e", "event-uuid", "--favorite" };
  PKChar *argvNofav[]
      = { "p6a", "update", "-e", "event-uuid", "--no-favorite" };
  PKChar *argvMod[] = { "p6a", "update", "-e", "event-uuid", "-m" };
  PKChar *argvNomod[] = { "p6a", "update", "-e", "event-uuid", "-M" };
  PKChar *argvLongMod[]
      = { "p6a", "update", "-e", "event-uuid", "--moderated" };
  PKChar *argvLongNomod[]
      = { "p6a", "update", "-e", "event-uuid", "--no-moderated" };

  PK_ASSERT_EQ (pkcArgParse (&args, 6, argvRename), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CUPDATE);
  PK_ASSERT_STR_EQ (args.c.update.event, "event-uuid");
  PK_ASSERT_STR_EQ (args.c.update.name, "New Name");

  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvPub), PK_OK);
  PK_ASSERT_EQ (PKC_ARGPUB (args.flags), 1);

  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvPriv), PK_OK);
  PK_ASSERT_EQ (PKC_ARGPUB (args.flags), 0);

  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvFav), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFAV (args.flags), 1);

  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvNofav), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFAV (args.flags), 0);

  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvMod), PK_OK);
  PK_ASSERT_EQ (PKC_ARGMOD (args.flags), 1);

  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvNomod), PK_OK);
  PK_ASSERT_EQ (PKC_ARGMOD (args.flags), 0);

  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvLongMod), PK_OK);
  PK_ASSERT_EQ (PKC_ARGMOD (args.flags), 1);

  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvLongNomod), PK_OK);
  PK_ASSERT_EQ (PKC_ARGMOD (args.flags), 0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - qr
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testQr (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "qr", "-e", "event-uuid" };
  PKChar *argvOut[] = { "p6a", "qr", "-e", "event-uuid", "-t", "out.png" };
  PKChar *argvSvg[] = { "p6a", "qr", "-e", "event-uuid", "-F", "svg" };
  PKChar *argvPng[] = { "p6a", "qr", "-e", "event-uuid", "--format", "png" };
  PKChar *argvBadFmt[] = { "p6a", "qr", "-e", "event-uuid", "-F", "pdf" };
  PKChar *argvOldSvg[] = { "p6a", "qr", "-e", "event-uuid", "--svg" };
  PKChar *argvNone[] = { "p6a", "qr" };

  PK_ASSERT_EQ (pkcArgParse (&args, 4, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CQR);
  PK_ASSERT_STR_EQ (args.c.qr.event, "event-uuid");
  PK_ASSERT (args.c.qr.out == NULL);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTPLAIN);

  PK_ASSERT_EQ (pkcArgParse (&args, 6, argvOut), PK_OK);
  PK_ASSERT_STR_EQ (args.c.qr.out, "out.png");

  PK_ASSERT_EQ (pkcArgParse (&args, 6, argvSvg), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTSVG);

  PK_ASSERT_EQ (pkcArgParse (&args, 6, argvPng), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTPNG);

  PK_ASSERT_EQ (pkcArgParse (&args, 6, argvBadFmt), PK_EARG);
  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvOldSvg), PK_EARG);

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argvNone), PK_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - card
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testCard (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "card", "-e", "event-uuid", "-d", "bday" };
  PKChar *argvOpts[]
      = { "p6a",      "card",           "--event",  "event-uuid", "--design",
          "forest",   "--target",       "card.jpg", "--locale",   "cs",
          "--layout", "business",       "--paper",  "letter",     "--format",
          "jpg",      "--no-background" };
  PKChar *argvPdf[]
      = { "p6a", "card", "-e", "event-uuid", "-d", "tech", "-F", "pdf" };
  PKChar *argvSilver[] = { "p6a", "card", "-e", "event-uuid", "-d", "silver" };
  PKChar *argvNineties[]
      = { "p6a", "card", "-e", "event-uuid", "-d", "nineties" };
  PKChar *argvBadFmt[]
      = { "p6a", "card", "-e", "event-uuid", "-d", "tech", "-F", "svg" };
  PKChar *argvNoEvent[] = { "p6a", "card", "-d", "bday" };
  PKChar *argvNoDesign[] = { "p6a", "card", "-e", "event-uuid" };

  PK_ASSERT_EQ (pkcArgParse (&args, 6, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CCARD);
  PK_ASSERT_STR_EQ (args.c.card.event, "event-uuid");
  PK_ASSERT_STR_EQ (args.c.card.design, "bday");
  PK_ASSERT (args.c.card.out == NULL);
  PK_ASSERT (args.c.card.locale == NULL);
  PK_ASSERT (args.c.card.layout == NULL);
  PK_ASSERT (args.c.card.paper == NULL);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTPLAIN);
  PK_ASSERT_EQ (args.flags & PKC_FNOBG, (PKDword)0);

  PK_ASSERT_EQ (pkcArgParse (&args, 17, argvOpts), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CCARD);
  PK_ASSERT_STR_EQ (args.c.card.event, "event-uuid");
  PK_ASSERT_STR_EQ (args.c.card.design, "forest");
  PK_ASSERT_STR_EQ (args.c.card.out, "card.jpg");
  PK_ASSERT_STR_EQ (args.c.card.locale, "cs");
  PK_ASSERT_STR_EQ (args.c.card.layout, "business");
  PK_ASSERT_STR_EQ (args.c.card.paper, "letter");
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTJPG);
  PK_ASSERT (args.flags & PKC_FNOBG);

  PK_ASSERT_EQ (pkcArgParse (&args, 8, argvPdf), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFMT (args.flags), PKC_FMTPDF);

  PK_ASSERT_EQ (pkcArgParse (&args, 6, argvSilver), PK_OK);
  PK_ASSERT_STR_EQ (args.c.card.design, "silver");

  PK_ASSERT_EQ (pkcArgParse (&args, 6, argvNineties), PK_OK);
  PK_ASSERT_STR_EQ (args.c.card.design, "nineties");

  PK_ASSERT_EQ (pkcArgParse (&args, 8, argvBadFmt), PK_EARG);
  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvNoEvent), PK_EARG);
  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvNoDesign), PK_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - link
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testLink (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "link", "-e", "event-uuid" };
  PKChar *argvNone[] = { "p6a", "link" };

  PK_ASSERT_EQ (pkcArgParse (&args, 4, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CLINK);
  PK_ASSERT_STR_EQ (args.c.link.event, "event-uuid");

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argvNone), PK_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - media
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testMedia (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "media", "-e", "event-uuid" };
  PKChar *argvOwn[] = { "p6a", "media", "-e", "event-uuid", "-o" };
  PKChar *argvNone[] = { "p6a", "media" };

  PK_ASSERT_EQ (pkcArgParse (&args, 4, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CMEDIA);
  PK_ASSERT_STR_EQ (args.c.media.event, "event-uuid");
  PK_ASSERT_EQ (args.flags & PKC_FOWN, (PKDword)0);

  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvOwn), PK_OK);
  PK_ASSERT (args.flags & PKC_FOWN);

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argvNone), PK_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - download
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testDownload (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "download", "-e", "event-uuid", "-t", "./out" };
  PKChar *argvSingle[] = { "p6a", "download",   "-e", "event-uuid",
                           "-m",  "media-uuid", "-t", "photo.jpg" };
  PKChar *argvNone[] = { "p6a", "download" };

  PK_ASSERT_EQ (pkcArgParse (&args, 6, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CDOWNLOAD);
  PK_ASSERT_STR_EQ (args.c.download.event, "event-uuid");
  PK_ASSERT_STR_EQ (args.c.download.target, "./out");
  PK_ASSERT (args.c.download.media == NULL);

  PK_ASSERT_EQ (pkcArgParse (&args, 8, argvSingle), PK_OK);
  PK_ASSERT_STR_EQ (args.c.download.media, "media-uuid");
  PK_ASSERT_STR_EQ (args.c.download.target, "photo.jpg");

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argvNone), PK_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - upload
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testUpload (void)
{
  PKCArg args;
  PKChar *argv[] = { "p6a", "upload", "-e", "event-uuid", "-s", "photo.jpg" };
  PKChar *argvLong[]
      = { "p6a", "upload", "--event", "event-uuid", "--source", "photo.jpg" };
  PKChar *argvDir[] = { "p6a", "upload",    "-e",    "event-uuid",
                        "-s",  "photo.jpg", "--dir", "/custom/dir" };
  PKChar *argvNoEvent[] = { "p6a", "upload", "-s", "photo.jpg" };
  PKChar *argvNoSource[] = { "p6a", "upload", "-e", "event-uuid" };
  PKChar *argvNone[] = { "p6a", "upload" };

  PK_ASSERT_EQ (pkcArgParse (&args, 6, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CUPLOAD);
  PK_ASSERT_STR_EQ (args.c.upload.event, "event-uuid");
  PK_ASSERT_STR_EQ (args.c.upload.source, "photo.jpg");
  PK_ASSERT (args.dir == NULL);

  PK_ASSERT_EQ (pkcArgParse (&args, 6, argvLong), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CUPLOAD);
  PK_ASSERT_STR_EQ (args.c.upload.event, "event-uuid");
  PK_ASSERT_STR_EQ (args.c.upload.source, "photo.jpg");

  PK_ASSERT_EQ (pkcArgParse (&args, 8, argvDir), PK_OK);
  PK_ASSERT_STR_EQ (args.dir, "/custom/dir");

  // Missing event, source, or both must be rejected.
  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvNoEvent), PK_EARG);
  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvNoSource), PK_EARG);
  PK_ASSERT_EQ (pkcArgParse (&args, 2, argvNone), PK_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - edit
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testEdit (void)
{
  PKCArg args;
  PKChar *argvFav[]
      = { "p6a", "edit", "-e", "event-uuid", "-m", "media-uuid", "-f" };
  PKChar *argvNofav[]
      = { "p6a", "edit", "-e", "event-uuid", "-m", "media-uuid", "-F" };
  PKChar *argvLongFav[] = { "p6a", "edit",       "-e",        "event-uuid",
                            "-m",  "media-uuid", "--favorite" };
  PKChar *argvLongNofav[]
      = { "p6a", "edit",       "-e",           "event-uuid",
          "-m",  "media-uuid", "--no-favorite" };
  PKChar *argvNoEvent[] = { "p6a", "edit", "-m", "media-uuid", "-f" };
  PKChar *argvNoMedia[] = { "p6a", "edit", "-e", "event-uuid", "-f" };
  PKChar *argvNoField[]
      = { "p6a", "edit", "-e", "event-uuid", "-m", "media-uuid" };

  PK_ASSERT_EQ (pkcArgParse (&args, 7, argvFav), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CEDIT);
  PK_ASSERT_STR_EQ (args.c.edit.event, "event-uuid");
  PK_ASSERT_STR_EQ (args.c.edit.media, "media-uuid");
  PK_ASSERT_EQ (PKC_ARGFAV (args.flags), 1);

  PK_ASSERT_EQ (pkcArgParse (&args, 7, argvNofav), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFAV (args.flags), 0);

  PK_ASSERT_EQ (pkcArgParse (&args, 7, argvLongFav), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFAV (args.flags), 1);

  PK_ASSERT_EQ (pkcArgParse (&args, 7, argvLongNofav), PK_OK);
  PK_ASSERT_EQ (PKC_ARGFAV (args.flags), 0);

  // Missing event, media, or field must be rejected.
  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvNoEvent), PK_EARG);
  PK_ASSERT_EQ (pkcArgParse (&args, 5, argvNoMedia), PK_EARG);
  PK_ASSERT_EQ (pkcArgParse (&args, 6, argvNoField), PK_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - approve
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testApprove (void)
{
  PKCArg args;
  PKChar *argv[]
      = { "p6a", "approve", "-e", "event-uuid", "-m", "media-uuid" };
  PKChar *argvLong[]
      = { "p6a", "approve", "--event", "event-uuid", "--media", "media-uuid" };
  PKChar *argvRevoke[]
      = { "p6a", "approve", "-e", "event-uuid", "-m", "media-uuid", "-r" };
  PKChar *argvRevokeLong[] = { "p6a", "approve",    "-e",      "event-uuid",
                               "-m",  "media-uuid", "--revoke" };
  PKChar *argvNoEvent[] = { "p6a", "approve", "-m", "media-uuid" };
  PKChar *argvNoMedia[] = { "p6a", "approve", "-e", "event-uuid" };

  PK_ASSERT_EQ (pkcArgParse (&args, 6, argv), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CAPPROVE);
  PK_ASSERT_STR_EQ (args.c.approve.event, "event-uuid");
  PK_ASSERT_STR_EQ (args.c.approve.media, "media-uuid");
  PK_ASSERT_EQ (args.flags & PKC_FREVOKE, (PKDword)0);

  // -r / --revoke revokes approval instead.
  PK_ASSERT_EQ (pkcArgParse (&args, 7, argvRevoke), PK_OK);
  PK_ASSERT_EQ (args.cmd, PKC_CAPPROVE);
  PK_ASSERT (args.flags & PKC_FREVOKE);

  PK_ASSERT_EQ (pkcArgParse (&args, 7, argvRevokeLong), PK_OK);
  PK_ASSERT (args.flags & PKC_FREVOKE);

  PK_ASSERT_EQ (pkcArgParse (&args, 6, argvLong), PK_OK);
  PK_ASSERT_STR_EQ (args.c.approve.event, "event-uuid");
  PK_ASSERT_STR_EQ (args.c.approve.media, "media-uuid");

  // Missing event or media must be rejected.
  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvNoEvent), PK_EARG);
  PK_ASSERT_EQ (pkcArgParse (&args, 4, argvNoMedia), PK_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - flags cleared between calls
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
testFlagsCleared (void)
{
  // Flags from one parse must not bleed into the next.
  PKCArg args;
  PKChar *argvOwn[] = { "p6a", "list", "--owner-only" };
  PKChar *argvBare[] = { "p6a", "list" };

  PK_ASSERT_EQ (pkcArgParse (&args, 3, argvOwn), PK_OK);
  PK_ASSERT (args.flags & PKC_FOWN);

  PK_ASSERT_EQ (pkcArgParse (&args, 2, argvBare), PK_OK);
  PK_ASSERT_EQ (args.flags & PKC_FOWN, (PKDword)0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Main
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
main (void)
{
  PK_SUITE ("arg - error cases");
  PK_RUN (testNoArgs);
  PK_RUN (testUnknownCommand);

  PK_SUITE ("arg - meta commands");
  PK_RUN (testHelp);
  PK_RUN (testVersion);

  PK_SUITE ("arg - auth commands");
  PK_RUN (testLoginBare);
  PK_RUN (testLoginImport);
  PK_RUN (testLoginDir);
  PK_RUN (testLoginFormat);
  PK_RUN (testLogout);
  PK_RUN (testLogoutDir);
  PK_RUN (testStatus);

  PK_SUITE ("arg - data commands");
  PK_RUN (testSync);
  PK_RUN (testList);
  PK_RUN (testCreate);
  PK_RUN (testUpdate);
  PK_RUN (testQr);
  PK_RUN (testCard);
  PK_RUN (testLink);
  PK_RUN (testMedia);
  PK_RUN (testDownload);
  PK_RUN (testUpload);
  PK_RUN (testEdit);
  PK_RUN (testApprove);

  PK_SUITE ("arg - state isolation");
  PK_RUN (testFlagsCleared);

  PK_SUMMARY ();
}
