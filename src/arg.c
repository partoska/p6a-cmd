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
#include "logger.h"
#include "types.h"

#include <getopt.h>
#include <string.h>

static PLInt
plArgParseFmt (PLArg *args, const PLChar *value)
{
  args->flags &= ~(PLDword)PL_FFMTMASK;
  if (strcmp (value, "plain") == 0)
    {
      return PL_EOK;
    }
  if (strcmp (value, "json") == 0)
    {
      args->flags |= PL_FFMTJSON;
      return PL_EOK;
    }
  if (strcmp (value, "csv") == 0)
    {
      args->flags |= PL_FFMTCSV;
      return PL_EOK;
    }
  if (strcmp (value, "one") == 0)
    {
      args->flags |= PL_FFMTONE;
      return PL_EOK;
    }
  return PL_EARG;
}

static PLInt
plArgParseLogin (PLArg *args, PLInt argc, PLChar **argv)
{
  static struct option opts[] = { { "dir", required_argument, NULL, 'D' },
                                  { "import", required_argument, NULL, 'i' },
                                  { NULL, 0, NULL, 0 } };
  PLInt c;
  opterr = 0;
  while ((c = getopt_long (argc, argv, "D:i:", opts, NULL)) != -1)
    {
      switch (c)
        {
        case 'D':
          PL_DSLOW ("Arg: --dir=%s", optarg);
          args->dir = optarg;
          break;

        case 'i':
          PL_DSLOW ("Arg: --import=%s", optarg);
          args->c.login.path = optarg;
          break;

        default:
          return PL_EARG;
        }
    }

  return PL_EOK;
}

static PLInt
plArgParseSync (PLArg *args, PLInt argc, PLChar **argv)
{
  static struct option opts[] = { { "dir", required_argument, NULL, 'D' },
                                  { "target", required_argument, NULL, 't' },
                                  { "owner-only", no_argument, NULL, 'o' },
                                  { "favorite-only", no_argument, NULL, 'f' },
                                  { NULL, 0, NULL, 0 } };
  PLInt c;
  opterr = 0;
  while ((c = getopt_long (argc, argv, "D:t:of", opts, NULL)) != -1)
    {
      switch (c)
        {
        case 'D':
          PL_DSLOW ("Arg: --dir=%s", optarg);
          args->dir = optarg;
          break;

        case 't':
          PL_DSLOW ("Arg: --target=%s", optarg);
          args->c.sync.target = optarg;
          break;

        case 'o':
          PL_DSLOW ("Arg: --owner-only");
          args->flags |= PL_FOWN;
          break;

        case 'f':
          PL_DSLOW ("Arg: --favorite-only");
          args->flags |= PL_FFAV;
          break;

        default:
          return PL_EARG;
        }
    }

  if (args->c.sync.target == NULL)
    {
      return PL_EARG;
    }

  return PL_EOK;
}

static PLInt
plArgParseList (PLArg *args, PLInt argc, PLChar **argv)
{
  static struct option opts[] = { { "dir", required_argument, NULL, 'D' },
                                  { "query", required_argument, NULL, 'q' },
                                  { "owner-only", no_argument, NULL, 'o' },
                                  { "favorite-only", no_argument, NULL, 'f' },
                                  { "one", no_argument, NULL, '1' },
                                  { "format", required_argument, NULL, 'F' },
                                  { NULL, 0, NULL, 0 } };
  PLInt c;
  opterr = 0;
  while ((c = getopt_long (argc, argv, "D:q:of1F:", opts, NULL)) != -1)
    {
      switch (c)
        {
        case 'D':
          PL_DSLOW ("Arg: --dir=%s", optarg);
          args->dir = optarg;
          break;

        case 'q':
          PL_DSLOW ("Arg: --query=%s", optarg);
          args->c.list.query = optarg;
          break;

        case 'o':
          PL_DSLOW ("Arg: --owner-only");
          args->flags |= PL_FOWN;
          break;

        case 'f':
          PL_DSLOW ("Arg: --favorite-only");
          args->flags |= PL_FFAV;
          break;

        case '1':
          PL_DSLOW ("Arg: -1");
          args->flags = (args->flags & ~(PLDword)PL_FFMTMASK) | PL_FFMTONE;
          break;

        case 'F':
          PL_DSLOW ("Arg: --format=%s", optarg);
          if (plArgParseFmt (args, optarg) != PL_EOK)
            {
              return PL_EARG;
            }
          break;

        default:
          return PL_EARG;
        }
    }

  return PL_EOK;
}

static PLInt
plArgParseCreate (PLArg *args, PLInt argc, PLChar **argv)
{
  static struct option opts[] = { { "dir", required_argument, NULL, 'D' },
                                  { "name", required_argument, NULL, 'n' },
                                  { "one", no_argument, NULL, '1' },
                                  { "format", required_argument, NULL, 'F' },
                                  { NULL, 0, NULL, 0 } };
  PLInt c;
  opterr = 0;
  while ((c = getopt_long (argc, argv, "D:n:1F:", opts, NULL)) != -1)
    {
      switch (c)
        {
        case 'D':
          PL_DSLOW ("Arg: --dir=%s", optarg);
          args->dir = optarg;
          break;

        case 'n':
          PL_DSLOW ("Arg: --name=%s", optarg);
          args->c.create.name = optarg;
          break;

        case '1':
          PL_DSLOW ("Arg: -1");
          args->flags = (args->flags & ~(PLDword)PL_FFMTMASK) | PL_FFMTONE;
          break;

        case 'F':
          PL_DSLOW ("Arg: --format=%s", optarg);
          if (plArgParseFmt (args, optarg) != PL_EOK)
            {
              return PL_EARG;
            }
          break;

        default:
          return PL_EARG;
        }
    }

  if (args->c.create.name == NULL)
    {
      return PL_EARG;
    }

  return PL_EOK;
}

static PLInt
plArgParseUpdate (PLArg *args, PLInt argc, PLChar **argv)
{
  static struct option opts[] = { { "dir", required_argument, NULL, 'D' },
                                  { "event", required_argument, NULL, 'e' },
                                  { "name", required_argument, NULL, 'n' },
                                  { "from", required_argument, NULL, 'S' },
                                  { "to", required_argument, NULL, 'E' },
                                  { "public", no_argument, NULL, 'p' },
                                  { "private", no_argument, NULL, 'P' },
                                  { "favorite", no_argument, NULL, 'f' },
                                  { "no-favorite", no_argument, NULL, 'F' },
                                  { NULL, 0, NULL, 0 } };
  PLInt c;
  opterr = 0;
  while ((c = getopt_long (argc, argv, "D:e:n:S:E:pPfF", opts, NULL)) != -1)
    {
      switch (c)
        {
        case 'D':
          PL_DSLOW ("Arg: --dir=%s", optarg);
          args->dir = optarg;
          break;

        case 'e':
          PL_DSLOW ("Arg: --event=%s", optarg);
          args->c.update.event = optarg;
          break;

        case 'n':
          PL_DSLOW ("Arg: --name=%s", optarg);
          args->c.update.name = optarg;
          break;

        case 'S':
          PL_DSLOW ("Arg: --from=%s", optarg);
          args->c.update.from = optarg;
          break;

        case 'E':
          PL_DSLOW ("Arg: --to=%s", optarg);
          args->c.update.to = optarg;
          break;

        case 'p':
          PL_DSLOW ("Arg: --public");
          args->flags = (args->flags & ~(PLDword)PL_FPUBMASK) | PL_FPUBTRUE;
          break;

        case 'P':
          PL_DSLOW ("Arg: --private");
          args->flags = (args->flags & ~(PLDword)PL_FPUBMASK) | PL_FPUBFALSE;
          break;

        case 'f':
          PL_DSLOW ("Arg: --favorite");
          args->flags = (args->flags & ~(PLDword)PL_FFAVMASK) | PL_FFAVTRUE;
          break;

        case 'F':
          PL_DSLOW ("Arg: --no-favorite");
          args->flags = (args->flags & ~(PLDword)PL_FFAVMASK) | PL_FFAVFALSE;
          break;

        default:
          return PL_EARG;
        }
    }

  if (args->c.update.event == NULL)
    {
      return PL_EARG;
    }

  if (args->c.update.name == NULL && args->c.update.from == NULL
      && args->c.update.to == NULL && (args->flags & PL_FPUBMASK) == 0
      && (args->flags & PL_FFAVMASK) == 0)
    {
      return PL_EARG;
    }

  return PL_EOK;
}

static PLInt
plArgParseQr (PLArg *args, PLInt argc, PLChar **argv)
{
  static struct option opts[] = { { "dir", required_argument, NULL, 'D' },
                                  { "event", required_argument, NULL, 'e' },
                                  { "target", required_argument, NULL, 't' },
                                  { "svg", no_argument, NULL, 's' },
                                  { NULL, 0, NULL, 0 } };
  PLInt c;
  opterr = 0;
  while ((c = getopt_long (argc, argv, "D:e:t:s", opts, NULL)) != -1)
    {
      switch (c)
        {
        case 'D':
          PL_DSLOW ("Arg: --dir=%s", optarg);
          args->dir = optarg;
          break;

        case 'e':
          PL_DSLOW ("Arg: --event=%s", optarg);
          args->c.qr.event = optarg;
          break;

        case 't':
          PL_DSLOW ("Arg: --target=%s", optarg);
          args->c.qr.out = optarg;
          break;

        case 's':
          PL_DSLOW ("Arg: --svg");
          args->flags |= PL_FSVG;
          break;

        default:
          return PL_EARG;
        }
    }

  if (args->c.qr.event == NULL)
    {
      return PL_EARG;
    }

  return PL_EOK;
}

static PLInt
plArgParseMedia (PLArg *args, PLInt argc, PLChar **argv)
{
  static struct option opts[] = { { "dir", required_argument, NULL, 'D' },
                                  { "event", required_argument, NULL, 'e' },
                                  { "owner-only", no_argument, NULL, 'o' },
                                  { "favorite-only", no_argument, NULL, 'f' },
                                  { "one", no_argument, NULL, '1' },
                                  { "format", required_argument, NULL, 'F' },
                                  { NULL, 0, NULL, 0 } };
  PLInt c;
  opterr = 0;
  while ((c = getopt_long (argc, argv, "D:e:of1F:", opts, NULL)) != -1)
    {
      switch (c)
        {
        case 'D':
          PL_DSLOW ("Arg: --dir=%s", optarg);
          args->dir = optarg;
          break;

        case 'e':
          PL_DSLOW ("Arg: --event=%s", optarg);
          args->c.media.event = optarg;
          break;

        case 'o':
          PL_DSLOW ("Arg: --owner-only");
          args->flags |= PL_FOWN;
          break;

        case 'f':
          PL_DSLOW ("Arg: --favorite-only");
          args->flags |= PL_FFAV;
          break;

        case '1':
          PL_DSLOW ("Arg: -1");
          args->flags = (args->flags & ~(PLDword)PL_FFMTMASK) | PL_FFMTONE;
          break;

        case 'F':
          PL_DSLOW ("Arg: --format=%s", optarg);
          if (plArgParseFmt (args, optarg) != PL_EOK)
            {
              return PL_EARG;
            }
          break;

        default:
          return PL_EARG;
        }
    }

  if (args->c.media.event == NULL)
    {
      return PL_EARG;
    }

  return PL_EOK;
}

static PLInt
plArgParseDownload (PLArg *args, PLInt argc, PLChar **argv)
{
  static struct option opts[] = { { "dir", required_argument, NULL, 'D' },
                                  { "event", required_argument, NULL, 'e' },
                                  { "media", required_argument, NULL, 'm' },
                                  { "target", required_argument, NULL, 't' },
                                  { "owner-only", no_argument, NULL, 'o' },
                                  { "favorite-only", no_argument, NULL, 'f' },
                                  { NULL, 0, NULL, 0 } };
  PLInt c;
  opterr = 0;
  while ((c = getopt_long (argc, argv, "D:e:m:t:of", opts, NULL)) != -1)
    {
      switch (c)
        {
        case 'D':
          PL_DSLOW ("Arg: --dir=%s", optarg);
          args->dir = optarg;
          break;

        case 'e':
          PL_DSLOW ("Arg: --event=%s", optarg);
          args->c.download.event = optarg;
          break;

        case 'm':
          PL_DSLOW ("Arg: --media=%s", optarg);
          args->c.download.media = optarg;
          break;

        case 't':
          PL_DSLOW ("Arg: --target=%s", optarg);
          args->c.download.target = optarg;
          break;

        case 'o':
          PL_DSLOW ("Arg: --owner-only");
          args->flags |= PL_FOWN;
          break;

        case 'f':
          PL_DSLOW ("Arg: --favorite-only");
          args->flags |= PL_FFAV;
          break;

        default:
          return PL_EARG;
        }
    }

  if (args->c.download.event == NULL || args->c.download.target == NULL)
    {
      return PL_EARG;
    }

  return PL_EOK;
}

static PLInt
plArgParseLink (PLArg *args, PLInt argc, PLChar **argv)
{
  static struct option opts[] = { { "dir", required_argument, NULL, 'D' },
                                  { "event", required_argument, NULL, 'e' },
                                  { NULL, 0, NULL, 0 } };
  PLInt c;
  opterr = 0;
  while ((c = getopt_long (argc, argv, "D:e:", opts, NULL)) != -1)
    {
      switch (c)
        {
        case 'D':
          PL_DSLOW ("Arg: --dir=%s", optarg);
          args->dir = optarg;
          break;

        case 'e':
          PL_DSLOW ("Arg: --event=%s", optarg);
          args->c.link.event = optarg;
          break;

        default:
          return PL_EARG;
        }
    }

  if (args->c.link.event == NULL)
    {
      return PL_EARG;
    }

  return PL_EOK;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PLInt
plArgParse (PLArg *args, PLInt argc, PLChar **argv)
{
  args->cmd = PL_CNONE;
  args->flags = 0;
  args->dir = NULL;
  memset (&args->c, 0, sizeof (args->c));
  if (argc < 2)
    {
      return PL_EARG;
    }

  const PLChar *cmd = argv[1];
  if (strcmp (cmd, "help") == 0 || strcmp (cmd, "-h") == 0
      || strcmp (cmd, "--help") == 0)
    {
      args->cmd = PL_CHELP;
      return PL_EOK;
    }
  else if (strcmp (cmd, "version") == 0 || strcmp (cmd, "-V") == 0
           || strcmp (cmd, "--version") == 0)
    {
      args->cmd = PL_CVERSION;
      return PL_EOK;
    }
  else if (strcmp (cmd, "sync") == 0)
    {
      args->cmd = PL_CSYNC;
      return plArgParseSync (args, argc - 1, &argv[1]);
    }

  else if (strcmp (cmd, "login") == 0)
    {
      args->cmd = PL_CLOGIN;
      return plArgParseLogin (args, argc - 1, &argv[1]);
    }
  else if (strcmp (cmd, "logout") == 0)
    {
      args->cmd = PL_CLOGOUT;
      return argc == 2 ? PL_EOK : PL_EARG;
    }
  else if (strcmp (cmd, "list") == 0)
    {
      args->cmd = PL_CLIST;
      return plArgParseList (args, argc - 1, &argv[1]);
    }
  else if (strcmp (cmd, "create") == 0)
    {
      args->cmd = PL_CCREATE;
      return plArgParseCreate (args, argc - 1, &argv[1]);
    }
  else if (strcmp (cmd, "update") == 0)
    {
      args->cmd = PL_CUPDATE;
      return plArgParseUpdate (args, argc - 1, &argv[1]);
    }
  else if (strcmp (cmd, "qr") == 0)
    {
      args->cmd = PL_CQR;
      return plArgParseQr (args, argc - 1, &argv[1]);
    }
  else if (strcmp (cmd, "link") == 0)
    {
      args->cmd = PL_CLINK;
      return plArgParseLink (args, argc - 1, &argv[1]);
    }
  else if (strcmp (cmd, "media") == 0)
    {
      args->cmd = PL_CMEDIA;
      return plArgParseMedia (args, argc - 1, &argv[1]);
    }
  else if (strcmp (cmd, "download") == 0)
    {
      args->cmd = PL_CDOWNLOAD;
      return plArgParseDownload (args, argc - 1, &argv[1]);
    }
  else
    {
      return PL_EARG;
    }
}
