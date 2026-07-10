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
#include <getopt.h>
#include <stdio.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Helpers
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* getopt_long uses global state; reset it before each call. */
static void
resetGetopt (void)
{
#if defined(__linux__) || defined(__GLIBC__) || defined(_WIN32)
  // glibc/MSYS2 extension: 0 triggers full reset.
  optind = 0;
#else
  optind = 1;
  // BSD/macOS extension.
  optreset = 1;
#endif
}

/* Parse helper: resets getopt state then calls plArgParse. */
static PLInt
parse (PLArg *args, PLInt argc, PLChar **argv)
{
  resetGetopt ();
  return plArgParse (args, argc, argv);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - Error cases
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testNoArgs (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a" };
  PL_ASSERT_EQ (parse (&args, 1, argv), PL_EARG);
}

static void
testUnknownCommand (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "notacommand" };
  PL_ASSERT_EQ (parse (&args, 2, argv), PL_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - help / version
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testHelp (void)
{
  PLArg args;
  PLChar *argvHelp[] = { "p6a", "help" };
  PLChar *argvShort[] = { "p6a", "-h" };
  PLChar *argvLong[] = { "p6a", "--help" };

  PL_ASSERT_EQ (parse (&args, 2, argvHelp), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CHELP);

  PL_ASSERT_EQ (parse (&args, 2, argvShort), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CHELP);

  PL_ASSERT_EQ (parse (&args, 2, argvLong), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CHELP);
}

static void
testVersion (void)
{
  PLArg args;
  PLChar *argvVer[] = { "p6a", "version" };
  PLChar *argvShort[] = { "p6a", "-V" };
  PLChar *argvLong[] = { "p6a", "--version" };

  PL_ASSERT_EQ (parse (&args, 2, argvVer), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CVERSION);

  PL_ASSERT_EQ (parse (&args, 2, argvShort), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CVERSION);

  PL_ASSERT_EQ (parse (&args, 2, argvLong), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CVERSION);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - login
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testLoginBare (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "login" };

  PL_ASSERT_EQ (parse (&args, 2, argv), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CLOGIN);
  PL_ASSERT (args.c.login.path == NULL);
  PL_ASSERT (args.dir == NULL);
}

static void
testLoginImport (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "login", "--import", "/path/to/cfg.ini" };

  PL_ASSERT_EQ (parse (&args, 4, argv), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CLOGIN);
  PL_ASSERT_STR_EQ (args.c.login.path, "/path/to/cfg.ini");
}

static void
testLoginDir (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "login", "--dir", "/custom/dir" };

  PL_ASSERT_EQ (parse (&args, 4, argv), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CLOGIN);
  PL_ASSERT_STR_EQ (args.dir, "/custom/dir");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - logout
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testLogout (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "logout" };
  PLChar *argvExtra[] = { "p6a", "logout", "unexpected" };

  PL_ASSERT_EQ (parse (&args, 2, argv), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CLOGOUT);

  // Extra positional args should be rejected.
  PL_ASSERT_EQ (parse (&args, 3, argvExtra), PL_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - sync
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testSync (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "sync", "-t", "/photos" };
  PLChar *argvNoTgt[] = { "p6a", "sync" };
  PLChar *argvOwn[] = { "p6a", "sync", "-t", "/photos", "--owner-only" };
  PLChar *argvFav[] = { "p6a", "sync", "-t", "/photos", "--favorite-only" };
  PLChar *argvBoth[] = { "p6a", "sync", "-t", "/photos", "-o", "-f" };

  PL_ASSERT_EQ (parse (&args, 4, argv), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CSYNC);
  PL_ASSERT_STR_EQ (args.c.sync.target, "/photos");
  PL_ASSERT_EQ (args.flags & PL_FOWN, (PLDword)0);
  PL_ASSERT_EQ (args.flags & PL_FFAV, (PLDword)0);

  PL_ASSERT_EQ (parse (&args, 2, argvNoTgt), PL_EARG);

  PL_ASSERT_EQ (parse (&args, 5, argvOwn), PL_EOK);
  PL_ASSERT (args.flags & PL_FOWN);

  PL_ASSERT_EQ (parse (&args, 5, argvFav), PL_EOK);
  PL_ASSERT (args.flags & PL_FFAV);

  PL_ASSERT_EQ (parse (&args, 6, argvBoth), PL_EOK);
  PL_ASSERT (args.flags & PL_FOWN);
  PL_ASSERT (args.flags & PL_FFAV);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - list
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testList (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "list" };
  PLChar *argvQuery[] = { "p6a", "list", "-q", "Birthday" };
  PLChar *argvOne[] = { "p6a", "list", "-1" };
  PLChar *argvJson[] = { "p6a", "list", "-F", "json" };
  PLChar *argvCsv[] = { "p6a", "list", "-F", "csv" };

  PL_ASSERT_EQ (parse (&args, 2, argv), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CLIST);
  PL_ASSERT (args.c.list.query == NULL);

  PL_ASSERT_EQ (parse (&args, 4, argvQuery), PL_EOK);
  PL_ASSERT_STR_EQ (args.c.list.query, "Birthday");

  PL_ASSERT_EQ (parse (&args, 3, argvOne), PL_EOK);
  PL_ASSERT_EQ (PL_ARGFMT (args.flags), PL_FMTONE);

  PL_ASSERT_EQ (parse (&args, 4, argvJson), PL_EOK);
  PL_ASSERT_EQ (PL_ARGFMT (args.flags), PL_FMTJSON);

  PL_ASSERT_EQ (parse (&args, 4, argvCsv), PL_EOK);
  PL_ASSERT_EQ (PL_ARGFMT (args.flags), PL_FMTCSV);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - create
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testCreate (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "create", "-n", "My Event" };
  PLChar *argvNoName[] = { "p6a", "create" };

  PL_ASSERT_EQ (parse (&args, 4, argv), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CCREATE);
  PL_ASSERT_STR_EQ (args.c.create.name, "My Event");

  PL_ASSERT_EQ (parse (&args, 2, argvNoName), PL_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - update
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testUpdate (void)
{
  PLArg args;
  PLChar *argvRename[]
      = { "p6a", "update", "-e", "event-uuid", "-n", "New Name" };
  PLChar *argvPub[] = { "p6a", "update", "-e", "event-uuid", "--public" };
  PLChar *argvPriv[] = { "p6a", "update", "-e", "event-uuid", "--private" };
  PLChar *argvFav[] = { "p6a", "update", "-e", "event-uuid", "--favorite" };
  PLChar *argvNofav[]
      = { "p6a", "update", "-e", "event-uuid", "--no-favorite" };
  PLChar *argvMod[] = { "p6a", "update", "-e", "event-uuid", "-m" };
  PLChar *argvNomod[] = { "p6a", "update", "-e", "event-uuid", "-M" };
  PLChar *argvLongMod[]
      = { "p6a", "update", "-e", "event-uuid", "--moderated" };
  PLChar *argvLongNomod[]
      = { "p6a", "update", "-e", "event-uuid", "--no-moderated" };

  PL_ASSERT_EQ (parse (&args, 6, argvRename), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CUPDATE);
  PL_ASSERT_STR_EQ (args.c.update.event, "event-uuid");
  PL_ASSERT_STR_EQ (args.c.update.name, "New Name");

  PL_ASSERT_EQ (parse (&args, 5, argvPub), PL_EOK);
  PL_ASSERT_EQ (PL_ARGPUB (args.flags), 1);

  PL_ASSERT_EQ (parse (&args, 5, argvPriv), PL_EOK);
  PL_ASSERT_EQ (PL_ARGPUB (args.flags), 0);

  PL_ASSERT_EQ (parse (&args, 5, argvFav), PL_EOK);
  PL_ASSERT_EQ (PL_ARGFAV (args.flags), 1);

  PL_ASSERT_EQ (parse (&args, 5, argvNofav), PL_EOK);
  PL_ASSERT_EQ (PL_ARGFAV (args.flags), 0);

  PL_ASSERT_EQ (parse (&args, 5, argvMod), PL_EOK);
  PL_ASSERT_EQ (PL_ARGMOD (args.flags), 1);

  PL_ASSERT_EQ (parse (&args, 5, argvNomod), PL_EOK);
  PL_ASSERT_EQ (PL_ARGMOD (args.flags), 0);

  PL_ASSERT_EQ (parse (&args, 5, argvLongMod), PL_EOK);
  PL_ASSERT_EQ (PL_ARGMOD (args.flags), 1);

  PL_ASSERT_EQ (parse (&args, 5, argvLongNomod), PL_EOK);
  PL_ASSERT_EQ (PL_ARGMOD (args.flags), 0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - qr
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testQr (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "qr", "-e", "event-uuid" };
  PLChar *argvOut[] = { "p6a", "qr", "-e", "event-uuid", "-t", "out.png" };
  PLChar *argvSvg[] = { "p6a", "qr", "-e", "event-uuid", "-F", "svg" };
  PLChar *argvPng[] = { "p6a", "qr", "-e", "event-uuid",
                        "--format", "png" };
  PLChar *argvBadFmt[] = { "p6a", "qr", "-e", "event-uuid", "-F", "pdf" };
  PLChar *argvOldSvg[] = { "p6a", "qr", "-e", "event-uuid", "--svg" };
  PLChar *argvNone[] = { "p6a", "qr" };

  PL_ASSERT_EQ (parse (&args, 4, argv), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CQR);
  PL_ASSERT_STR_EQ (args.c.qr.event, "event-uuid");
  PL_ASSERT (args.c.qr.out == NULL);
  PL_ASSERT_EQ (PL_ARGFMT (args.flags), PL_FMTPLAIN);

  PL_ASSERT_EQ (parse (&args, 6, argvOut), PL_EOK);
  PL_ASSERT_STR_EQ (args.c.qr.out, "out.png");

  PL_ASSERT_EQ (parse (&args, 6, argvSvg), PL_EOK);
  PL_ASSERT_EQ (PL_ARGFMT (args.flags), PL_FMTSVG);

  PL_ASSERT_EQ (parse (&args, 6, argvPng), PL_EOK);
  PL_ASSERT_EQ (PL_ARGFMT (args.flags), PL_FMTPNG);

  PL_ASSERT_EQ (parse (&args, 6, argvBadFmt), PL_EARG);
  PL_ASSERT_EQ (parse (&args, 5, argvOldSvg), PL_EARG);

  PL_ASSERT_EQ (parse (&args, 2, argvNone), PL_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - card
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testCard (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "card", "-e", "event-uuid", "-d", "bday" };
  PLChar *argvOpts[]
      = { "p6a",     "card",      "--event",  "event-uuid", "--design",
          "forest",  "--target",  "card.jpg", "--locale",   "cs",
          "--layout", "business", "--paper",   "letter",     "--format",
          "jpg",     "--no-background" };
  PLChar *argvPdf[] = { "p6a", "card", "-e", "event-uuid",
                        "-d",  "tech", "-F", "pdf" };
  PLChar *argvSilver[] = { "p6a", "card", "-e", "event-uuid",
                           "-d",  "silver" };
  PLChar *argvNineties[] = { "p6a", "card", "-e", "event-uuid",
                             "-d",  "nineties" };
  PLChar *argvBadFmt[] = { "p6a", "card", "-e", "event-uuid",
                           "-d",  "tech", "-F", "svg" };
  PLChar *argvNoEvent[] = { "p6a", "card", "-d", "bday" };
  PLChar *argvNoDesign[] = { "p6a", "card", "-e", "event-uuid" };

  PL_ASSERT_EQ (parse (&args, 6, argv), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CCARD);
  PL_ASSERT_STR_EQ (args.c.card.event, "event-uuid");
  PL_ASSERT_STR_EQ (args.c.card.design, "bday");
  PL_ASSERT (args.c.card.out == NULL);
  PL_ASSERT (args.c.card.locale == NULL);
  PL_ASSERT (args.c.card.layout == NULL);
  PL_ASSERT (args.c.card.paper == NULL);
  PL_ASSERT_EQ (PL_ARGFMT (args.flags), PL_FMTPLAIN);
  PL_ASSERT_EQ (args.flags & PL_FNOBG, (PLDword)0);

  PL_ASSERT_EQ (parse (&args, 17, argvOpts), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CCARD);
  PL_ASSERT_STR_EQ (args.c.card.event, "event-uuid");
  PL_ASSERT_STR_EQ (args.c.card.design, "forest");
  PL_ASSERT_STR_EQ (args.c.card.out, "card.jpg");
  PL_ASSERT_STR_EQ (args.c.card.locale, "cs");
  PL_ASSERT_STR_EQ (args.c.card.layout, "business");
  PL_ASSERT_STR_EQ (args.c.card.paper, "letter");
  PL_ASSERT_EQ (PL_ARGFMT (args.flags), PL_FMTJPG);
  PL_ASSERT (args.flags & PL_FNOBG);

  PL_ASSERT_EQ (parse (&args, 8, argvPdf), PL_EOK);
  PL_ASSERT_EQ (PL_ARGFMT (args.flags), PL_FMTPDF);

  PL_ASSERT_EQ (parse (&args, 6, argvSilver), PL_EOK);
  PL_ASSERT_STR_EQ (args.c.card.design, "silver");

  PL_ASSERT_EQ (parse (&args, 6, argvNineties), PL_EOK);
  PL_ASSERT_STR_EQ (args.c.card.design, "nineties");

  PL_ASSERT_EQ (parse (&args, 8, argvBadFmt), PL_EARG);
  PL_ASSERT_EQ (parse (&args, 4, argvNoEvent), PL_EARG);
  PL_ASSERT_EQ (parse (&args, 4, argvNoDesign), PL_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - link
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testLink (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "link", "-e", "event-uuid" };
  PLChar *argvNone[] = { "p6a", "link" };

  PL_ASSERT_EQ (parse (&args, 4, argv), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CLINK);
  PL_ASSERT_STR_EQ (args.c.link.event, "event-uuid");

  PL_ASSERT_EQ (parse (&args, 2, argvNone), PL_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - media
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testMedia (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "media", "-e", "event-uuid" };
  PLChar *argvOwn[] = { "p6a", "media", "-e", "event-uuid", "-o" };
  PLChar *argvNone[] = { "p6a", "media" };

  PL_ASSERT_EQ (parse (&args, 4, argv), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CMEDIA);
  PL_ASSERT_STR_EQ (args.c.media.event, "event-uuid");
  PL_ASSERT_EQ (args.flags & PL_FOWN, (PLDword)0);

  PL_ASSERT_EQ (parse (&args, 5, argvOwn), PL_EOK);
  PL_ASSERT (args.flags & PL_FOWN);

  PL_ASSERT_EQ (parse (&args, 2, argvNone), PL_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - download
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testDownload (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "download", "-e", "event-uuid", "-t", "./out" };
  PLChar *argvSingle[] = { "p6a", "download",   "-e", "event-uuid",
                           "-m",  "media-uuid", "-t", "photo.jpg" };
  PLChar *argvNone[] = { "p6a", "download" };

  PL_ASSERT_EQ (parse (&args, 6, argv), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CDOWNLOAD);
  PL_ASSERT_STR_EQ (args.c.download.event, "event-uuid");
  PL_ASSERT_STR_EQ (args.c.download.target, "./out");
  PL_ASSERT (args.c.download.media == NULL);

  PL_ASSERT_EQ (parse (&args, 8, argvSingle), PL_EOK);
  PL_ASSERT_STR_EQ (args.c.download.media, "media-uuid");
  PL_ASSERT_STR_EQ (args.c.download.target, "photo.jpg");

  PL_ASSERT_EQ (parse (&args, 2, argvNone), PL_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - upload
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testUpload (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "upload", "-e", "event-uuid", "-s", "photo.jpg" };
  PLChar *argvLong[] = { "p6a",      "upload",    "--event",  "event-uuid",
                         "--source", "photo.jpg" };
  PLChar *argvDir[] = { "p6a",    "upload",     "-e",      "event-uuid",
                        "-s",     "photo.jpg",  "--dir",   "/custom/dir" };
  PLChar *argvNoEvent[] = { "p6a", "upload", "-s", "photo.jpg" };
  PLChar *argvNoSource[] = { "p6a", "upload", "-e", "event-uuid" };
  PLChar *argvNone[] = { "p6a", "upload" };

  PL_ASSERT_EQ (parse (&args, 6, argv), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CUPLOAD);
  PL_ASSERT_STR_EQ (args.c.upload.event, "event-uuid");
  PL_ASSERT_STR_EQ (args.c.upload.source, "photo.jpg");
  PL_ASSERT (args.dir == NULL);

  PL_ASSERT_EQ (parse (&args, 6, argvLong), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CUPLOAD);
  PL_ASSERT_STR_EQ (args.c.upload.event, "event-uuid");
  PL_ASSERT_STR_EQ (args.c.upload.source, "photo.jpg");

  PL_ASSERT_EQ (parse (&args, 8, argvDir), PL_EOK);
  PL_ASSERT_STR_EQ (args.dir, "/custom/dir");

  // Missing event, source, or both must be rejected.
  PL_ASSERT_EQ (parse (&args, 4, argvNoEvent), PL_EARG);
  PL_ASSERT_EQ (parse (&args, 4, argvNoSource), PL_EARG);
  PL_ASSERT_EQ (parse (&args, 2, argvNone), PL_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - edit
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testEdit (void)
{
  PLArg args;
  PLChar *argvFav[] = { "p6a", "edit", "-e", "event-uuid",
                        "-m",  "media-uuid", "-f" };
  PLChar *argvNofav[] = { "p6a", "edit", "-e", "event-uuid",
                          "-m",  "media-uuid", "-F" };
  PLChar *argvLongFav[] = { "p6a", "edit",       "-e", "event-uuid",
                            "-m",  "media-uuid", "--favorite" };
  PLChar *argvLongNofav[] = { "p6a", "edit",       "-e", "event-uuid",
                              "-m",  "media-uuid", "--no-favorite" };
  PLChar *argvNoEvent[] = { "p6a", "edit", "-m", "media-uuid", "-f" };
  PLChar *argvNoMedia[] = { "p6a", "edit", "-e", "event-uuid", "-f" };
  PLChar *argvNoField[] = { "p6a", "edit", "-e", "event-uuid",
                            "-m",  "media-uuid" };

  PL_ASSERT_EQ (parse (&args, 7, argvFav), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CEDIT);
  PL_ASSERT_STR_EQ (args.c.edit.event, "event-uuid");
  PL_ASSERT_STR_EQ (args.c.edit.media, "media-uuid");
  PL_ASSERT_EQ (PL_ARGFAV (args.flags), 1);

  PL_ASSERT_EQ (parse (&args, 7, argvNofav), PL_EOK);
  PL_ASSERT_EQ (PL_ARGFAV (args.flags), 0);

  PL_ASSERT_EQ (parse (&args, 7, argvLongFav), PL_EOK);
  PL_ASSERT_EQ (PL_ARGFAV (args.flags), 1);

  PL_ASSERT_EQ (parse (&args, 7, argvLongNofav), PL_EOK);
  PL_ASSERT_EQ (PL_ARGFAV (args.flags), 0);

  // Missing event, media, or field must be rejected.
  PL_ASSERT_EQ (parse (&args, 5, argvNoEvent), PL_EARG);
  PL_ASSERT_EQ (parse (&args, 5, argvNoMedia), PL_EARG);
  PL_ASSERT_EQ (parse (&args, 6, argvNoField), PL_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - approve
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testApprove (void)
{
  PLArg args;
  PLChar *argv[] = { "p6a", "approve", "-e", "event-uuid",
                     "-m",  "media-uuid" };
  PLChar *argvLong[] = { "p6a",    "approve", "--event", "event-uuid",
                         "--media", "media-uuid" };
  PLChar *argvNoEvent[] = { "p6a", "approve", "-m", "media-uuid" };
  PLChar *argvNoMedia[] = { "p6a", "approve", "-e", "event-uuid" };

  PL_ASSERT_EQ (parse (&args, 6, argv), PL_EOK);
  PL_ASSERT_EQ (args.cmd, PL_CAPPROVE);
  PL_ASSERT_STR_EQ (args.c.approve.event, "event-uuid");
  PL_ASSERT_STR_EQ (args.c.approve.media, "media-uuid");

  PL_ASSERT_EQ (parse (&args, 6, argvLong), PL_EOK);
  PL_ASSERT_STR_EQ (args.c.approve.event, "event-uuid");
  PL_ASSERT_STR_EQ (args.c.approve.media, "media-uuid");

  // Missing event or media must be rejected.
  PL_ASSERT_EQ (parse (&args, 4, argvNoEvent), PL_EARG);
  PL_ASSERT_EQ (parse (&args, 4, argvNoMedia), PL_EARG);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Tests - flags cleared between calls
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
testFlagsCleared (void)
{
  // Flags from one parse must not bleed into the next.
  PLArg args;
  PLChar *argvOwn[] = { "p6a", "list", "--owner-only" };
  PLChar *argvBare[] = { "p6a", "list" };

  PL_ASSERT_EQ (parse (&args, 3, argvOwn), PL_EOK);
  PL_ASSERT (args.flags & PL_FOWN);

  PL_ASSERT_EQ (parse (&args, 2, argvBare), PL_EOK);
  PL_ASSERT_EQ (args.flags & PL_FOWN, (PLDword)0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Main
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
main (void)
{
  PL_SUITE ("arg - error cases");
  PL_RUN (testNoArgs);
  PL_RUN (testUnknownCommand);

  PL_SUITE ("arg - meta commands");
  PL_RUN (testHelp);
  PL_RUN (testVersion);

  PL_SUITE ("arg - auth commands");
  PL_RUN (testLoginBare);
  PL_RUN (testLoginImport);
  PL_RUN (testLoginDir);
  PL_RUN (testLogout);

  PL_SUITE ("arg - data commands");
  PL_RUN (testSync);
  PL_RUN (testList);
  PL_RUN (testCreate);
  PL_RUN (testUpdate);
  PL_RUN (testQr);
  PL_RUN (testCard);
  PL_RUN (testLink);
  PL_RUN (testMedia);
  PL_RUN (testDownload);
  PL_RUN (testUpload);
  PL_RUN (testEdit);
  PL_RUN (testApprove);

  PL_SUITE ("arg - state isolation");
  PL_RUN (testFlagsCleared);

  PL_SUMMARY ();
}
