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

#include "rng.h"
#include "types.h"

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
PL_COMPILE_ASSERT (PL_SIZEOF (PUCHAR) == PL_SIZEOF (PLByte *));
#else
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PLInt
plGenRandomBytes (PLByte *buf, PLSize len)
{
#ifdef _WIN32
  if (len > 0xFFFFFFFF)
    {
      return PL_EFS;
    }

  NTSTATUS status = BCryptGenRandom (NULL, (PUCHAR)buf, (ULONG)len,
                                     BCRYPT_USE_SYSTEM_PREFERRED_RNG);
  return BCRYPT_SUCCESS (status) ? PL_EOK : PL_EFS;
#else
  PLInt fd = open ("/dev/urandom", O_RDONLY | O_CLOEXEC);
  if (fd < 0)
    {
      return PL_EFS;
    }

  PLSize remaining = len;
  PLByte *ptr = buf;
  while (remaining > 0)
    {
      PLSSize n = read (fd, ptr, remaining);
      if (n < 0)
        {
          if (errno == EINTR)
            {
              continue;
            }

          close (fd);
          return PL_EFS;
        }

      ptr += (PLSize)n;
      remaining -= (PLSize)n;
    }

  close (fd);
  return PL_EOK;
#endif
}
