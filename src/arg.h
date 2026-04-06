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

#ifndef ARG_H
#define ARG_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "types.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define PL_ARGFMT(flags) ((PLArgFmt)(((flags) & PL_FFMTMASK) >> 4))
#define PL_ARGPUB(flags) ((PLInt)(((flags) & PL_FPUBMASK) >> 8) - 1)
#define PL_ARGFAV(flags) ((PLInt)(((flags) & PL_FFAVMASK) >> 10) - 1)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef enum
{
  PL_CLOGIN,
  PL_CLOGOUT,
  PL_CSYNC,
  PL_CLIST,
  PL_CQR,
  PL_CLINK,
  PL_CCREATE,
  PL_CUPDATE,
  PL_CMEDIA,
  PL_CDOWNLOAD,
  PL_CHELP,
  PL_CVERSION,

  PL_CNONE
} PLArgCmd;

typedef enum
{
  PL_FOWN = 0x00000001,
  PL_FFAV = 0x00000002,
  PL_FSVG = 0x00000004,
  PL_FFMTJSON = 0x00000010,
  PL_FFMTCSV = 0x00000020,
  PL_FFMTONE = 0x00000030,
  PL_FFMTMASK = 0x000000F0,
  PL_FPUBFALSE = 0x00000100,
  PL_FPUBTRUE = 0x00000200,
  PL_FPUBMASK = 0x00000300,
  PL_FFAVFALSE = 0x00000400,
  PL_FFAVTRUE = 0x00000800,
  PL_FFAVMASK = 0x00000C00
} PLArgFlag;

typedef enum
{
  PL_FMTPLAIN,
  PL_FMTJSON,
  PL_FMTCSV,
  PL_FMTONE
} PLArgFmt;

typedef struct PLArgs
{
  PLArgCmd cmd;
  PLDword flags;
  const PLChar *dir;
  union
  {
    struct
    {
      const PLChar *path;
    } login;
    struct
    {
      const PLChar *target;
    } sync;
    struct
    {
      const PLChar *query;
    } list;
    struct
    {
      const PLChar *event;
      const PLChar *out;
    } qr;
    struct
    {
      const PLChar *event;
    } link;
    struct
    {
      const PLChar *name;
    } create;
    struct
    {
      const PLChar *event;
      const PLChar *name;
      const PLChar *from;
      const PLChar *to;
    } update;
    struct
    {
      const PLChar *event;
    } media;
    struct
    {
      const PLChar *event;
      const PLChar *target;
      const PLChar *media;
    } download;
  } c;
} PLArg;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Parses command-line arguments into a PLArg structure.
 *
 * @param args Output structure populated with the parsed arguments.
 * @param argc Argument count as received by main.
 * @param argv Argument vector as received by main.
 * @return PL_EOK on success, PL_EARG on invalid arguments.
 */
PLInt plArgParse (PLArg *args, PLInt argc, PLChar **argv);

#endif
