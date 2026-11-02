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

#include "log.h"
#include "plat.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKInt
pkcInfo (const PKChar *fmt, ...)
{
  PKVaList args;
  PK_VA_START (args, fmt);

  PKInt result = pkcPrintV (PK_FALSE, fmt, args);

  PK_VA_END (args);

  return result;
}

PKInt
pkcWarn (const PKChar *fmt, ...)
{
  PKVaList args;
  PK_VA_START (args, fmt);

  PKInt result = pkcPrintV (PK_TRUE, fmt, args);

  PK_VA_END (args);

  return result;
}

PKInt
pkcError (const PKChar *fmt, ...)
{
  PKVaList args;
  PK_VA_START (args, fmt);

  PKInt result = pkcPrintV (PK_TRUE, fmt, args);

  PK_VA_END (args);

  return result;
}

PKInt
pkcFatal (const PKChar *fmt, ...)
{
  PKVaList args;
  PK_VA_START (args, fmt);

  PKInt result = pkcPrintV (PK_TRUE, fmt, args);

  PK_VA_END (args);

  return result;
}

PKInt
pkcDebug (const PKChar *fmt, ...)
{
#ifdef _DEBUG
  PKVaList args;
  PK_VA_START (args, fmt);

  PKInt result = pkcPrintV (PK_TRUE, fmt, args);

  PK_VA_END (args);

  return result;
#else
  PK_UNUSED (fmt);
  return 0;
#endif
}
