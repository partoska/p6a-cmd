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
#include "log.h"
#include "pktypes.h"
#include "plat.h"

static PKInt
pkcArgParseFmt (PKCArg *args, const PKChar *value)
{
  args->flags &= ~(PKDword)PKC_FFMTMASK;
  if (pkcStrEq (value, "plain"))
    {
      return PK_OK;
    }
  if (pkcStrEq (value, "json"))
    {
      args->flags |= PKC_FFMTJSON;
      return PK_OK;
    }
  if (pkcStrEq (value, "csv"))
    {
      args->flags |= PKC_FFMTCSV;
      return PK_OK;
    }
  if (pkcStrEq (value, "one"))
    {
      args->flags |= PKC_FFMTONE;
      return PK_OK;
    }
  return PK_EARG;
}

/* Parses commands whose only option is the working directory (logout). */
static PKInt
pkcArgParseDir (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[]
      = { { "dir", 'D', PK_TRUE }, { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        default:
          return PK_EARG;
        }
    }

  return pkcOptIndex () == argc ? PK_OK : PK_EARG;
}

static PKInt
pkcArgParseStatus (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[] = { { "dir", 'D', PK_TRUE },
                                 { "one", '1', PK_FALSE },
                                 { "format", 'F', PK_TRUE },
                                 { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        case '1':
          PKC_DSLOW ("Arg: -1");
          args->flags = (args->flags & ~(PKDword)PKC_FFMTMASK) | PKC_FFMTONE;
          break;

        case 'F':
          PKC_DSLOW ("Arg: --format=%s", value);
          if (pkcArgParseFmt (args, value) != PK_OK)
            {
              return PK_EARG;
            }
          break;

        default:
          return PK_EARG;
        }
    }

  return pkcOptIndex () == argc ? PK_OK : PK_EARG;
}

static PKInt
pkcArgParseLogin (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[] = { { "dir", 'D', PK_TRUE },
                                 { "import", 'i', PK_TRUE },
                                 { "one", '1', PK_FALSE },
                                 { "format", 'F', PK_TRUE },
                                 { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        case 'i':
          PKC_DSLOW ("Arg: --import=%s", value);
          args->c.login.path = value;
          break;

        case '1':
          PKC_DSLOW ("Arg: -1");
          args->flags = (args->flags & ~(PKDword)PKC_FFMTMASK) | PKC_FFMTONE;
          break;

        case 'F':
          PKC_DSLOW ("Arg: --format=%s", value);
          if (pkcArgParseFmt (args, value) != PK_OK)
            {
              return PK_EARG;
            }
          break;

        default:
          return PK_EARG;
        }
    }

  return PK_OK;
}

static PKInt
pkcArgParseSync (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[] = { { "dir", 'D', PK_TRUE },
                                 { "target", 't', PK_TRUE },
                                 { "owner-only", 'o', PK_FALSE },
                                 { "favorite-only", 'f', PK_FALSE },
                                 { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        case 't':
          PKC_DSLOW ("Arg: --target=%s", value);
          args->c.sync.target = value;
          break;

        case 'o':
          PKC_DSLOW ("Arg: --owner-only");
          args->flags |= PKC_FOWN;
          break;

        case 'f':
          PKC_DSLOW ("Arg: --favorite-only");
          args->flags |= PKC_FFAV;
          break;

        default:
          return PK_EARG;
        }
    }

  if (args->c.sync.target == NULL)
    {
      return PK_EARG;
    }

  return PK_OK;
}

static PKInt
pkcArgParseList (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[] = { { "dir", 'D', PK_TRUE },
                                 { "query", 'q', PK_TRUE },
                                 { "owner-only", 'o', PK_FALSE },
                                 { "favorite-only", 'f', PK_FALSE },
                                 { "one", '1', PK_FALSE },
                                 { "format", 'F', PK_TRUE },
                                 { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        case 'q':
          PKC_DSLOW ("Arg: --query=%s", value);
          args->c.list.query = value;
          break;

        case 'o':
          PKC_DSLOW ("Arg: --owner-only");
          args->flags |= PKC_FOWN;
          break;

        case 'f':
          PKC_DSLOW ("Arg: --favorite-only");
          args->flags |= PKC_FFAV;
          break;

        case '1':
          PKC_DSLOW ("Arg: -1");
          args->flags = (args->flags & ~(PKDword)PKC_FFMTMASK) | PKC_FFMTONE;
          break;

        case 'F':
          PKC_DSLOW ("Arg: --format=%s", value);
          if (pkcArgParseFmt (args, value) != PK_OK)
            {
              return PK_EARG;
            }
          break;

        default:
          return PK_EARG;
        }
    }

  return PK_OK;
}

static PKInt
pkcArgParseCreate (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[] = { { "dir", 'D', PK_TRUE },
                                 { "name", 'n', PK_TRUE },
                                 { "one", '1', PK_FALSE },
                                 { "format", 'F', PK_TRUE },
                                 { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        case 'n':
          PKC_DSLOW ("Arg: --name=%s", value);
          args->c.create.name = value;
          break;

        case '1':
          PKC_DSLOW ("Arg: -1");
          args->flags = (args->flags & ~(PKDword)PKC_FFMTMASK) | PKC_FFMTONE;
          break;

        case 'F':
          PKC_DSLOW ("Arg: --format=%s", value);
          if (pkcArgParseFmt (args, value) != PK_OK)
            {
              return PK_EARG;
            }
          break;

        default:
          return PK_EARG;
        }
    }

  if (args->c.create.name == NULL)
    {
      return PK_EARG;
    }

  return PK_OK;
}

static PKInt
pkcArgParseUpdate (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[] = { { "dir", 'D', PK_TRUE },
                                 { "event", 'e', PK_TRUE },
                                 { "name", 'n', PK_TRUE },
                                 { "from", 'S', PK_TRUE },
                                 { "to", 'E', PK_TRUE },
                                 { "public", 'p', PK_FALSE },
                                 { "private", 'P', PK_FALSE },
                                 { "favorite", 'f', PK_FALSE },
                                 { "no-favorite", 'F', PK_FALSE },
                                 { "moderated", 'm', PK_FALSE },
                                 { "no-moderated", 'M', PK_FALSE },
                                 { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        case 'e':
          PKC_DSLOW ("Arg: --event=%s", value);
          args->c.update.event = value;
          break;

        case 'n':
          PKC_DSLOW ("Arg: --name=%s", value);
          args->c.update.name = value;
          break;

        case 'S':
          PKC_DSLOW ("Arg: --from=%s", value);
          args->c.update.from = value;
          break;

        case 'E':
          PKC_DSLOW ("Arg: --to=%s", value);
          args->c.update.to = value;
          break;

        case 'p':
          PKC_DSLOW ("Arg: --public");
          args->flags = (args->flags & ~(PKDword)PKC_FPUBMASK) | PKC_FPUBTRUE;
          break;

        case 'P':
          PKC_DSLOW ("Arg: --private");
          args->flags = (args->flags & ~(PKDword)PKC_FPUBMASK) | PKC_FPUBFALSE;
          break;

        case 'f':
          PKC_DSLOW ("Arg: --favorite");
          args->flags = (args->flags & ~(PKDword)PKC_FFAVMASK) | PKC_FFAVTRUE;
          break;

        case 'F':
          PKC_DSLOW ("Arg: --no-favorite");
          args->flags = (args->flags & ~(PKDword)PKC_FFAVMASK) | PKC_FFAVFALSE;
          break;

        case 'm':
          PKC_DSLOW ("Arg: --moderated");
          args->flags = (args->flags & ~(PKDword)PKC_FMODMASK) | PKC_FMODTRUE;
          break;

        case 'M':
          PKC_DSLOW ("Arg: --no-moderated");
          args->flags = (args->flags & ~(PKDword)PKC_FMODMASK) | PKC_FMODFALSE;
          break;

        default:
          return PK_EARG;
        }
    }

  if (args->c.update.event == NULL)
    {
      return PK_EARG;
    }

  if (args->c.update.name == NULL && args->c.update.from == NULL
      && args->c.update.to == NULL && (args->flags & PKC_FPUBMASK) == 0
      && (args->flags & PKC_FFAVMASK) == 0
      && (args->flags & PKC_FMODMASK) == 0)
    {
      return PK_EARG;
    }

  return PK_OK;
}

static PKInt
pkcArgParseQr (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[] = { { "dir", 'D', PK_TRUE },
                                 { "event", 'e', PK_TRUE },
                                 { "target", 't', PK_TRUE },
                                 { "format", 'F', PK_TRUE },
                                 { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        case 'e':
          PKC_DSLOW ("Arg: --event=%s", value);
          args->c.qr.event = value;
          break;

        case 't':
          PKC_DSLOW ("Arg: --target=%s", value);
          args->c.qr.out = value;
          break;

        case 'F':
          PKC_DSLOW ("Arg: --format=%s", value);
          if (pkcStrEq (value, "svg"))
            {
              args->flags
                  = (args->flags & ~(PKDword)PKC_FFMTMASK) | PKC_FFMTSVG;
            }
          else if (pkcStrEq (value, "png"))
            {
              args->flags
                  = (args->flags & ~(PKDword)PKC_FFMTMASK) | PKC_FFMTPNG;
            }
          else
            {
              return PK_EARG;
            }
          break;

        default:
          return PK_EARG;
        }
    }

  if (args->c.qr.event == NULL)
    {
      return PK_EARG;
    }

  return PK_OK;
}

static PKInt
pkcArgParseMedia (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[] = { { "dir", 'D', PK_TRUE },
                                 { "event", 'e', PK_TRUE },
                                 { "owner-only", 'o', PK_FALSE },
                                 { "favorite-only", 'f', PK_FALSE },
                                 { "one", '1', PK_FALSE },
                                 { "format", 'F', PK_TRUE },
                                 { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        case 'e':
          PKC_DSLOW ("Arg: --event=%s", value);
          args->c.media.event = value;
          break;

        case 'o':
          PKC_DSLOW ("Arg: --owner-only");
          args->flags |= PKC_FOWN;
          break;

        case 'f':
          PKC_DSLOW ("Arg: --favorite-only");
          args->flags |= PKC_FFAV;
          break;

        case '1':
          PKC_DSLOW ("Arg: -1");
          args->flags = (args->flags & ~(PKDword)PKC_FFMTMASK) | PKC_FFMTONE;
          break;

        case 'F':
          PKC_DSLOW ("Arg: --format=%s", value);
          if (pkcArgParseFmt (args, value) != PK_OK)
            {
              return PK_EARG;
            }
          break;

        default:
          return PK_EARG;
        }
    }

  if (args->c.media.event == NULL)
    {
      return PK_EARG;
    }

  return PK_OK;
}

static PKInt
pkcArgParseDownload (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[] = { { "dir", 'D', PK_TRUE },
                                 { "event", 'e', PK_TRUE },
                                 { "media", 'm', PK_TRUE },
                                 { "target", 't', PK_TRUE },
                                 { "owner-only", 'o', PK_FALSE },
                                 { "favorite-only", 'f', PK_FALSE },
                                 { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        case 'e':
          PKC_DSLOW ("Arg: --event=%s", value);
          args->c.download.event = value;
          break;

        case 'm':
          PKC_DSLOW ("Arg: --media=%s", value);
          args->c.download.media = value;
          break;

        case 't':
          PKC_DSLOW ("Arg: --target=%s", value);
          args->c.download.target = value;
          break;

        case 'o':
          PKC_DSLOW ("Arg: --owner-only");
          args->flags |= PKC_FOWN;
          break;

        case 'f':
          PKC_DSLOW ("Arg: --favorite-only");
          args->flags |= PKC_FFAV;
          break;

        default:
          return PK_EARG;
        }
    }

  if (args->c.download.event == NULL || args->c.download.target == NULL)
    {
      return PK_EARG;
    }

  return PK_OK;
}

static PKInt
pkcArgParseUpload (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[] = { { "dir", 'D', PK_TRUE },
                                 { "event", 'e', PK_TRUE },
                                 { "source", 's', PK_TRUE },
                                 { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        case 'e':
          PKC_DSLOW ("Arg: --event=%s", value);
          args->c.upload.event = value;
          break;

        case 's':
          PKC_DSLOW ("Arg: --source=%s", value);
          args->c.upload.source = value;
          break;

        default:
          return PK_EARG;
        }
    }

  if (args->c.upload.event == NULL || args->c.upload.source == NULL)
    {
      return PK_EARG;
    }

  return PK_OK;
}

static PKInt
pkcArgParseEdit (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[]
      = { { "dir", 'D', PK_TRUE },          { "event", 'e', PK_TRUE },
          { "media", 'm', PK_TRUE },        { "favorite", 'f', PK_FALSE },
          { "no-favorite", 'F', PK_FALSE }, { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        case 'e':
          PKC_DSLOW ("Arg: --event=%s", value);
          args->c.edit.event = value;
          break;

        case 'm':
          PKC_DSLOW ("Arg: --media=%s", value);
          args->c.edit.media = value;
          break;

        case 'f':
          PKC_DSLOW ("Arg: --favorite");
          args->flags = (args->flags & ~(PKDword)PKC_FFAVMASK) | PKC_FFAVTRUE;
          break;

        case 'F':
          PKC_DSLOW ("Arg: --no-favorite");
          args->flags = (args->flags & ~(PKDword)PKC_FFAVMASK) | PKC_FFAVFALSE;
          break;

        default:
          return PK_EARG;
        }
    }

  if (args->c.edit.event == NULL || args->c.edit.media == NULL)
    {
      return PK_EARG;
    }

  if ((args->flags & PKC_FFAVMASK) == 0)
    {
      return PK_EARG;
    }

  return PK_OK;
}

static PKInt
pkcArgParseCard (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[] = { { "dir", 'D', PK_TRUE },
                                 { "event", 'e', PK_TRUE },
                                 { "design", 'd', PK_TRUE },
                                 { "target", 't', PK_TRUE },
                                 { "locale", 'l', PK_TRUE },
                                 { "layout", 'L', PK_TRUE },
                                 { "paper", 'p', PK_TRUE },
                                 { "format", 'F', PK_TRUE },
                                 { "no-background", 'b', PK_FALSE },
                                 { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        case 'e':
          PKC_DSLOW ("Arg: --event=%s", value);
          args->c.card.event = value;
          break;

        case 'd':
          PKC_DSLOW ("Arg: --design=%s", value);
          args->c.card.design = value;
          break;

        case 't':
          PKC_DSLOW ("Arg: --target=%s", value);
          args->c.card.out = value;
          break;

        case 'l':
          PKC_DSLOW ("Arg: --locale=%s", value);
          args->c.card.locale = value;
          break;

        case 'L':
          PKC_DSLOW ("Arg: --layout=%s", value);
          args->c.card.layout = value;
          break;

        case 'p':
          PKC_DSLOW ("Arg: --paper=%s", value);
          args->c.card.paper = value;
          break;

        case 'F':
          PKC_DSLOW ("Arg: --format=%s", value);
          if (pkcStrEq (value, "jpg"))
            {
              args->flags
                  = (args->flags & ~(PKDword)PKC_FFMTMASK) | PKC_FFMTJPG;
            }
          else if (pkcStrEq (value, "pdf"))
            {
              args->flags
                  = (args->flags & ~(PKDword)PKC_FFMTMASK) | PKC_FFMTPDF;
            }
          else
            {
              return PK_EARG;
            }
          break;

        case 'b':
          PKC_DSLOW ("Arg: --no-background");
          args->flags |= PKC_FNOBG;
          break;

        default:
          return PK_EARG;
        }
    }

  if (args->c.card.event == NULL || args->c.card.design == NULL)
    {
      return PK_EARG;
    }

  return PK_OK;
}

static PKInt
pkcArgParseApprove (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[] = { { "dir", 'D', PK_TRUE },
                                 { "event", 'e', PK_TRUE },
                                 { "media", 'm', PK_TRUE },
                                 { "revoke", 'r', PK_FALSE },
                                 { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        case 'e':
          PKC_DSLOW ("Arg: --event=%s", value);
          args->c.approve.event = value;
          break;

        case 'm':
          PKC_DSLOW ("Arg: --media=%s", value);
          args->c.approve.media = value;
          break;

        case 'r':
          PKC_DSLOW ("Arg: --revoke");
          args->flags |= PKC_FREVOKE;
          break;

        default:
          return PK_EARG;
        }
    }

  if (args->c.approve.event == NULL || args->c.approve.media == NULL)
    {
      return PK_EARG;
    }

  return PK_OK;
}

static PKInt
pkcArgParseLink (PKCArg *args, PKInt argc, PKChar **argv)
{
  static const PKCOpt opts[] = { { "dir", 'D', PK_TRUE },
                                 { "event", 'e', PK_TRUE },
                                 { NULL, 0, PK_FALSE } };
  const PKChar *value;
  PKInt c;
  while ((c = pkcOptNext (argc, argv, opts, &value)) != -1)
    {
      switch (c)
        {
        case 'D':
          PKC_DSLOW ("Arg: --dir=%s", value);
          args->dir = value;
          break;

        case 'e':
          PKC_DSLOW ("Arg: --event=%s", value);
          args->c.link.event = value;
          break;

        default:
          return PK_EARG;
        }
    }

  if (args->c.link.event == NULL)
    {
      return PK_EARG;
    }

  return PK_OK;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKInt
pkcArgParse (PKCArg *args, PKInt argc, PKChar **argv)
{
  pkcOptReset ();
  args->cmd = PKC_CNONE;
  args->flags = 0;
  args->dir = NULL;
  pkcMemZero (&args->c, sizeof (args->c));
  if (argc < 2)
    {
      return PK_EARG;
    }

  const PKChar *cmd = argv[1];
  if (pkcStrEq (cmd, "help") || pkcStrEq (cmd, "-h")
      || pkcStrEq (cmd, "--help"))
    {
      args->cmd = PKC_CHELP;
      return PK_OK;
    }
  else if (pkcStrEq (cmd, "version") || pkcStrEq (cmd, "-V")
           || pkcStrEq (cmd, "--version"))
    {
      args->cmd = PKC_CVERSION;
      return PK_OK;
    }
  else if (pkcStrEq (cmd, "sync"))
    {
      args->cmd = PKC_CSYNC;
      return pkcArgParseSync (args, argc - 1, &argv[1]);
    }

  else if (pkcStrEq (cmd, "login"))
    {
      args->cmd = PKC_CLOGIN;
      return pkcArgParseLogin (args, argc - 1, &argv[1]);
    }
  else if (pkcStrEq (cmd, "logout"))
    {
      args->cmd = PKC_CLOGOUT;
      return pkcArgParseDir (args, argc - 1, &argv[1]);
    }
  else if (pkcStrEq (cmd, "status"))
    {
      args->cmd = PKC_CSTATUS;
      return pkcArgParseStatus (args, argc - 1, &argv[1]);
    }
  else if (pkcStrEq (cmd, "list"))
    {
      args->cmd = PKC_CLIST;
      return pkcArgParseList (args, argc - 1, &argv[1]);
    }
  else if (pkcStrEq (cmd, "create"))
    {
      args->cmd = PKC_CCREATE;
      return pkcArgParseCreate (args, argc - 1, &argv[1]);
    }
  else if (pkcStrEq (cmd, "update"))
    {
      args->cmd = PKC_CUPDATE;
      return pkcArgParseUpdate (args, argc - 1, &argv[1]);
    }
  else if (pkcStrEq (cmd, "qr"))
    {
      args->cmd = PKC_CQR;
      return pkcArgParseQr (args, argc - 1, &argv[1]);
    }
  else if (pkcStrEq (cmd, "link"))
    {
      args->cmd = PKC_CLINK;
      return pkcArgParseLink (args, argc - 1, &argv[1]);
    }
  else if (pkcStrEq (cmd, "media"))
    {
      args->cmd = PKC_CMEDIA;
      return pkcArgParseMedia (args, argc - 1, &argv[1]);
    }
  else if (pkcStrEq (cmd, "download"))
    {
      args->cmd = PKC_CDOWNLOAD;
      return pkcArgParseDownload (args, argc - 1, &argv[1]);
    }
  else if (pkcStrEq (cmd, "upload"))
    {
      args->cmd = PKC_CUPLOAD;
      return pkcArgParseUpload (args, argc - 1, &argv[1]);
    }
  else if (pkcStrEq (cmd, "edit"))
    {
      args->cmd = PKC_CEDIT;
      return pkcArgParseEdit (args, argc - 1, &argv[1]);
    }
  else if (pkcStrEq (cmd, "approve"))
    {
      args->cmd = PKC_CAPPROVE;
      return pkcArgParseApprove (args, argc - 1, &argv[1]);
    }
  else if (pkcStrEq (cmd, "card"))
    {
      args->cmd = PKC_CCARD;
      return pkcArgParseCard (args, argc - 1, &argv[1]);
    }
  else
    {
      return PK_EARG;
    }
}
