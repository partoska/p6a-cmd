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

#ifndef PK_HTTP_H
#define PK_HTTP_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "cJSON.h"
#include "client.h"
#include "plat.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct PKRequest
{
  const PKChar *method; /* GET when NULL. */
  const PKChar *url;    /* Absolute URL; for API calls, the path below
                           the endpoint, e.g. "/event". */
  PKBool api;           /* Throttled, with the bearer token. */
  const PKChar *type;   /* Content type of the body, or NULL. */
  const PKVoid *body;   /* Request body, or NULL. */
  PKSize length;        /* Size of the body in bytes. */
  const PKChar *basic;  /* Basic credentials (base64), or NULL. */
  PKFile *file;         /* Receives the response body, or NULL. */
} PKRequest;

typedef struct PKResponse
{
  PKLong status; /* HTTP status. */
  PKChar *data;  /* NUL-terminated body, or NULL; see pkResponseFree(). */
  PKSize size;   /* Size of the body in bytes. */
} PKResponse;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Performs a request. Any HTTP status is a response; only transport
 * failures (PK_ENET), stops (PK_ESTOPPED) and, for API calls, a missing
 * login (PK_EAUTH) fail.
 *
 * @param client   Client to run in.
 * @param request  What to send.
 * @param response Receives the answer; release with pkResponseFree().
 * @return PK_OK on success, or a PK_E* status (recorded).
 */
PKStatus pkRequest (PKClient *client, const PKRequest *request,
                    PKResponse *response);

/**
 * Requires a response status: 401 fails with PK_EAUTH, anything else but
 * the expected status with PK_EHTTP, quoting the server's message.
 *
 * @param client   Client to record failures in.
 * @param response Response to check.
 * @param expected Expected HTTP status.
 * @return PK_OK when it matches, or the recorded status.
 */
PKStatus pkExpect (PKClient *client, const PKResponse *response,
                   PKLong expected);

/**
 * Parses a response body as a JSON object.
 *
 * @param client   Client to record failures in.
 * @param response Response to parse.
 * @param json     Receives the object; release with cJSON_Delete().
 * @return PK_OK on success, PK_EDATA or PK_EMEM.
 */
PKStatus pkResponseJson (PKClient *client, const PKResponse *response,
                         cJSON **json);

/**
 * Releases the body of a response.
 *
 * @param response Response to release.
 */
PKVoid pkResponseFree (PKResponse *response);

/**
 * Gets a string member of a JSON object.
 *
 * @param json Object.
 * @param name Member name.
 * @return The string, or NULL when missing or not a string.
 */
const PKChar *pkJsonString (const cJSON *json, const PKChar *name);

#endif
