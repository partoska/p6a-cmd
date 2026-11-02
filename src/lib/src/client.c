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
#include "plat.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* API requests: one per second on average, bursts of five. */
#define THROTTLE_RATE (1)
#define THROTTLE_BURST (5)

/* Longest sleep between two looks at the stop flag. */
#define NAP_SLICE_MS (100)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Internal
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKStatus
pkFail (PKClient *client, PKStatus status, PKLong code, const PKChar *fmt, ...)
{
  client->error.status = status;
  client->error.code = code;

  PKVaList args;
  PK_VA_START (args, fmt);
  pkStrFmtV (client->error.message, sizeof (client->error.message), fmt, args);
  PK_VA_END (args);

  return status;
}

PKStatus
pkFailMem (PKClient *client)
{
  return pkFail (client, PK_EMEM, 0, "Out of memory");
}

PKStatus
pkErrorContext (PKClient *client, const PKChar *fmt, ...)
{
  PKChar context[PK_ERROR_MAX];
  PKVaList args;
  PK_VA_START (args, fmt);
  pkStrFmtV (context, sizeof (context), fmt, args);
  PK_VA_END (args);

  PKChar message[PK_ERROR_MAX];
  pkStrFmt (message, sizeof (message), "%s: %s", context,
            client->error.message);
  pkMemCopy (client->error.message, message, sizeof (message));

  return client->error.status;
}

PKVoid
pkLog (PKClient *client, PKLogLevel level, const PKChar *fmt, ...)
{
  if (!client->log || level < client->level || level >= PK_LOG_SILENT)
    {
      return;
    }

  PKChar message[PK_ERROR_MAX];
  PKVaList args;
  PK_VA_START (args, fmt);
  pkStrFmtV (message, sizeof (message), fmt, args);
  PK_VA_END (args);

  client->log (client->user, level, message);
}

PKStatus
pkNap (PKClient *client, PKLong ms)
{
  while (ms > 0)
    {
      if (client->stop)
        {
          break;
        }

      PKLong slice = ms < NAP_SLICE_MS ? ms : NAP_SLICE_MS;
      pkSleep (slice);
      ms -= slice;
    }

  return client->stop ? pkFail (client, PK_ESTOPPED, 0, "Stopped") : PK_OK;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKClient *
pkInit (void)
{
  PKClient *client = pkMemZalloc (sizeof (PKClient));
  if (!client)
    {
      return NULL;
    }

  client->cfg = pkCfgInit ();
  if (!client->cfg)
    {
      pkMemFree (client);
      return NULL;
    }

  client->level = PK_LOG_INFO;
  pkThrottleInit (&client->throttle, THROTTLE_RATE, THROTTLE_BURST);

  return client;
}

PKVoid
pkDestroy (PKClient *client)
{
  if (!client)
    {
      return;
    }

  pkCfgDestroy (client->cfg);
  pkMemFree (client);
}

const PKError *
pkError (const PKClient *client)
{
  return &client->error;
}

const PKChar *
pkStatusText (PKStatus status)
{
  switch (status)
    {
    case PK_OK:
      return "Success";
    case PK_EARG:
      return "Invalid argument";
    case PK_EMEM:
      return "Out of memory";
    case PK_EFS:
      return "File system error";
    case PK_ENET:
      return "Network error";
    case PK_EHTTP:
      return "Unexpected response from Partoska";
    case PK_EAUTH:
      return "Not logged in";
    case PK_EDATA:
      return "Malformed data";
    case PK_ESTOPPED:
      return "Stopped";
    }

  return "Unknown error";
}

PKVoid
pkLogSetHandler (PKClient *client, PKLogFn handler, PKVoid *user)
{
  client->log = handler;
  client->user = user;
}

PKVoid
pkLogSetLevel (PKClient *client, PKLogLevel level)
{
  client->level = level;
}

PKVoid
pkStop (PKClient *client)
{
  client->stop = 1;
}

PKBool
pkStopping (const PKClient *client)
{
  return client->stop ? PK_TRUE : PK_FALSE;
}
