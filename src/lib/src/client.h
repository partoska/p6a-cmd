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

#ifndef PK_CLIENT_H
#define PK_CLIENT_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "config.h"
#include "partoska.h"
#include "throttle.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define PK_PATH_MAX (1024)

/*
 * Messages that may carry secrets (tokens, request bodies) are only built
 * in with DEBUG_SLOW.
 */
#if DEBUG_SLOW
#define PK_TRACE(client, ...) pkLog (client, PK_LOG_DEBUG, __VA_ARGS__)
#else
#define PK_TRACE(client, ...) PK_UNUSED (client)
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

struct PKClient
{
  PKCfg *cfg;                /* Configuration and login. */
  PKChar dir[PK_PATH_MAX];   /* Working directory, or empty. */
  PKChar ini[PK_PATH_MAX];   /* Its configuration file. */
  PKThrottle throttle;       /* Pace of the API requests. */
  PKError error;             /* Last failure. */
  PKLogFn log;               /* Log handler, or NULL. */
  PKVoid *user;              /* Passed to the log handler. */
  PKLogLevel level;          /* Least severity logged. */
  volatile PKSigAtomic stop; /* Set by pkStop(). */
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Records a failure.
 *
 * @param client Client to record in.
 * @param status Category of the failure, not PK_OK.
 * @param code   Underlying code (errno, libcurl code, HTTP status), or 0.
 * @param fmt    printf-style message.
 * @return status, so that callers can return the call.
 */
PKStatus pkFail (PKClient *client, PKStatus status, PKLong code,
                 const PKChar *fmt, ...) PK_PRINTF (4, 5);

/**
 * Records running out of memory.
 *
 * @param client Client to record in.
 * @return PK_EMEM.
 */
PKStatus pkFailMem (PKClient *client);

/**
 * Prefixes the message of the last failure with context, "context: cause".
 *
 * @param client Client whose failure to extend.
 * @param fmt    printf-style context.
 * @return Status of the last failure.
 */
PKStatus pkErrorContext (PKClient *client, const PKChar *fmt, ...)
    PK_PRINTF (2, 3);

/**
 * Logs a message through the client's handler, if it is at the threshold.
 *
 * @param client Client to log through.
 * @param level  Severity of the message.
 * @param fmt    printf-style message.
 */
PKVoid pkLog (PKClient *client, PKLogLevel level, const PKChar *fmt, ...)
    PK_PRINTF (3, 4);

/**
 * Sleeps, returning early on a stop request.
 *
 * @param client Client whose stop requests to honor.
 * @param ms     Milliseconds to sleep.
 * @return PK_OK, or PK_ESTOPPED (recorded) when stopped.
 */
PKStatus pkNap (PKClient *client, PKLong ms);

/**
 * Resolves the working directory, as pkOpen() describes it.
 *
 * @param client Client to record failures in.
 * @param dir    Requested directory, or NULL.
 * @param out    Receives the path.
 * @param size   Capacity of out in bytes.
 * @return PK_OK on success, or a PK_E* status.
 */
PKStatus pkWorkdir (PKClient *client, const PKChar *dir, PKChar *out,
                    PKSize size);

/**
 * Gets a valid access token, renewing the login when it has expired or is
 * about to; a renewed login is saved to the working directory.
 *
 * @param client Logged-in client.
 * @param token  Receives the token, owned by the client.
 * @return PK_OK on success, PK_EAUTH when not logged in or refused.
 */
PKStatus pkBearer (PKClient *client, const PKChar **token);

#endif
