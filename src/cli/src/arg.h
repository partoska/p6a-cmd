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

#ifndef PKC_ARG_H
#define PKC_ARG_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "partoska.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define PKC_ARGFMT(flags) ((PKCArgFmt)(((flags) & PKC_FFMTMASK) >> 4))
#define PKC_ARGPUB(flags) ((PKInt)(((flags) & PKC_FPUBMASK) >> 8) - 1)
#define PKC_ARGFAV(flags) ((PKInt)(((flags) & PKC_FFAVMASK) >> 10) - 1)
#define PKC_ARGMOD(flags) ((PKInt)(((flags) & PKC_FMODMASK) >> 12) - 1)

/* Times -a/--add and -r/--remove of trusted may each be given. */
#define PKC_ARG_LIST_MAX 32

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef enum
{
  PKC_CLOGIN,
  PKC_CLOGOUT,
  PKC_CSTATUS,
  PKC_CSYNC,
  PKC_CLIST,
  PKC_CQR,
  PKC_CLINK,
  PKC_CCREATE,
  PKC_CUPDATE,
  PKC_CMEDIA,
  PKC_CDOWNLOAD,
  PKC_CUPLOAD,
  PKC_CEDIT,
  PKC_CAPPROVE,
  PKC_CCARD,
  PKC_CJOIN,
  PKC_CTRUSTED,
  PKC_CHELP,
  PKC_CVERSION,

  PKC_CNONE
} PKCArgCmd;

typedef enum
{
  PKC_FOWN = 0x00000001,
  PKC_FFAV = 0x00000002,
  PKC_FCON = 0x00000004,
  PKC_FFMTJSON = 0x00000010,
  PKC_FFMTCSV = 0x00000020,
  PKC_FFMTONE = 0x00000030,
  PKC_FFMTSVG = 0x00000040,
  PKC_FFMTJPG = 0x00000050,
  PKC_FFMTPNG = 0x00000060,
  PKC_FFMTPDF = 0x00000070,
  PKC_FFMTMASK = 0x000000F0,
  PKC_FPUBFALSE = 0x00000100,
  PKC_FPUBTRUE = 0x00000200,
  PKC_FPUBMASK = 0x00000300,
  PKC_FFAVFALSE = 0x00000400,
  PKC_FFAVTRUE = 0x00000800,
  PKC_FFAVMASK = 0x00000C00,
  PKC_FMODFALSE = 0x00001000,
  PKC_FMODTRUE = 0x00002000,
  PKC_FMODMASK = 0x00003000,
  PKC_FNOBG = 0x00004000,
  PKC_FREVOKE = 0x00008000,
  PKC_FCLEAR = 0x00010000
} PKCArgFlag;

typedef enum
{
  PKC_FMTPLAIN,
  PKC_FMTJSON,
  PKC_FMTCSV,
  PKC_FMTONE,
  PKC_FMTSVG,
  PKC_FMTJPG,
  PKC_FMTPNG,
  PKC_FMTPDF
} PKCArgFmt;

typedef struct PKCArgs
{
  PKCArgCmd cmd;
  PKDword flags;
  const PKChar *dir;
  union
  {
    struct
    {
      const PKChar *path;
    } login;
    struct
    {
      const PKChar *target;
    } sync;
    struct
    {
      const PKChar *query;
    } list;
    struct
    {
      const PKChar *event;
      const PKChar *out;
    } qr;
    struct
    {
      const PKChar *event;
    } link;
    struct
    {
      const PKChar *name;
    } create;
    struct
    {
      const PKChar *event;
      const PKChar *name;
      const PKChar *from;
      const PKChar *to;
    } update;
    struct
    {
      const PKChar *event;
    } media;
    struct
    {
      const PKChar *event;
      const PKChar *target;
      const PKChar *media;
    } download;
    struct
    {
      const PKChar *event;
      const PKChar *source;
    } upload;
    struct
    {
      const PKChar *event;
      const PKChar *media;
    } edit;
    struct
    {
      const PKChar *event;
      const PKChar *media;
    } approve;
    struct
    {
      const PKChar *event;
      const PKChar *out;
      const PKChar *design;
      const PKChar *locale;
      const PKChar *layout;
      const PKChar *paper;
    } card;
    struct
    {
      const PKChar *invite;
    } join;
    struct
    {
      const PKChar *event;
      const PKChar *add[PKC_ARG_LIST_MAX];
      PKSize adds;
      const PKChar *remove[PKC_ARG_LIST_MAX];
      PKSize removes;
    } trusted;
  } c;
} PKCArg;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Parses command-line arguments into a PKCArg structure.
 *
 * @param args Output structure populated with the parsed arguments.
 * @param argc Argument count as received by main.
 * @param argv Argument vector as received by main.
 * @return PK_OK on success, PK_EARG on invalid arguments.
 */
PKInt pkcArgParse (PKCArg *args, PKInt argc, PKChar **argv);

#endif
