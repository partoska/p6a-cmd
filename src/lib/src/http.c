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

#include "http.h"
#include <curl/curl.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define URL_MAX (2048)
#define HEADER_MAX (2048)

/* Gives up on a connection after this long, and on a transfer stalled
 * (below one byte per second) for this long. */
#define CONNECT_TIMEOUT (30L)
#define STALL_TIMEOUT (120L)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef long CURLlong;

/* Where the response body goes. */
typedef struct PKSink
{
  PKResponse *response; /* Collects the body, unless file is set. */
  PKFile *file;         /* Receives the body. */
  PKBool failed;        /* The body could not be stored. */
} PKSink;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKSize
pkWrite (PKChar *data, PKSize sz, PKSize n, PKVoid *user)
{
  PKSink *sink = (PKSink *)user;
  PKSize size = sz * n;
  if (sink->file)
    {
      if (pkFileWrite (sink->file, data, size) != size)
        {
          sink->failed = PK_TRUE;
          return 0;
        }
      return size;
    }

  PKResponse *response = sink->response;
  PKChar *grown = pkMemRealloc (response->data, response->size + size + 1);
  if (!grown)
    {
      sink->failed = PK_TRUE;
      return 0;
    }

  response->data = grown;
  pkMemCopy (response->data + response->size, data, size);
  response->size += size;
  response->data[response->size] = '\0';

  return size;
}

static PKInt
pkProgress (PKVoid *user, curl_off_t dltotal, curl_off_t dlnow,
            curl_off_t ultotal, curl_off_t ulnow)
{
  PK_UNUSED (dltotal);
  PK_UNUSED (dlnow);
  PK_UNUSED (ultotal);
  PK_UNUSED (ulnow);

  // Non-zero aborts the transfer.
  return ((PKClient *)user)->stop ? 1 : 0;
}

/* Appends a formatted header; a NULL list means a failure. */
static struct curl_slist *
pkHeader (struct curl_slist *headers, PKBool *ok, const PKChar *fmt,
          const PKChar *value)
{
  if (!*ok)
    {
      return headers;
    }

  PKChar header[HEADER_MAX];
  PKInt length = pkStrFmt (header, sizeof (header), fmt, value);
  struct curl_slist *grown = length < 0 || (PKSize)length >= sizeof (header)
                                 ? NULL
                                 : curl_slist_append (headers, header);
  if (!grown)
    {
      *ok = PK_FALSE;
      return headers;
    }

  return grown;
}

/* Waits for the throttle, the pace of the API. */
static PKStatus
pkThrottle (PKClient *client)
{
  PKDword wait;
  while ((wait = pkThrottleTry (&client->throttle)) > 0)
    {
      PK_TRACE (client, "Throttle: waiting %u ms", wait);
      PKStatus status = pkNap (client, wait);
      if (status != PK_OK)
        {
          return status;
        }
    }

  return PK_OK;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Internal
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKStatus
pkRequest (PKClient *client, const PKRequest *request, PKResponse *response)
{
  *response = (PKResponse){ 0 };
  if (client->stop)
    {
      return pkFail (client, PK_ESTOPPED, 0, "Stopped");
    }

  PKChar url[URL_MAX];
  PKInt length = pkStrFmt (url, sizeof (url), "%s%s",
                           request->api ? client->cfg->glob->endpoint : "",
                           request->url);
  if (length < 0 || (PKSize)length >= sizeof (url))
    {
      return pkFail (client, PK_EARG, 0, "URL too long: %s", request->url);
    }

  const PKChar *token = NULL;
  if (request->api)
    {
      PKStatus status = pkBearer (client, &token);
      if (status == PK_OK)
        {
          status = pkThrottle (client);
        }
      if (status != PK_OK)
        {
          return status;
        }
    }

  CURL *curl = curl_easy_init ();
  if (!curl)
    {
      return pkFailMem (client);
    }

  PKBool ok = PK_TRUE;
  struct curl_slist *headers = NULL;
  headers = pkHeader (headers, &ok, "User-Agent: p6a/%s", PK_VERSION_STRING);
  if (request->api && !request->file)
    {
      headers = pkHeader (headers, &ok, "%s", "Accept: application/json");
    }
  if (request->type)
    {
      headers = pkHeader (headers, &ok, "Content-Type: %s", request->type);
    }
  if (token)
    {
      headers = pkHeader (headers, &ok, "Authorization: Bearer %s", token);
    }
  else if (request->basic)
    {
      headers
          = pkHeader (headers, &ok, "Authorization: Basic %s", request->basic);
    }
  if (!ok)
    {
      curl_slist_free_all (headers);
      curl_easy_cleanup (curl);
      return pkFailMem (client);
    }

  const PKChar *method = request->method ? request->method : "GET";
  PKSink sink = { response, request->file, PK_FALSE };
  curl_easy_setopt (curl, CURLOPT_URL, url);
  curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, pkWrite);
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, &sink);
  curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt (curl, CURLOPT_XFERINFOFUNCTION, pkProgress);
  curl_easy_setopt (curl, CURLOPT_XFERINFODATA, client);
  curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);
  curl_easy_setopt (curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
  curl_easy_setopt (curl, CURLOPT_LOW_SPEED_TIME, STALL_TIMEOUT);
#ifdef _WIN32
  curl_easy_setopt (curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
#endif
  if (request->body || pkStrCmp (method, "POST", PK_FALSE) == 0)
    {
      curl_easy_setopt (curl, CURLOPT_POSTFIELDS,
                        request->body ? request->body : "");
      curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE_LARGE,
                        (curl_off_t)request->length);
    }
  if (pkStrCmp (method, "GET", PK_FALSE) != 0
      && pkStrCmp (method, "POST", PK_FALSE) != 0)
    {
      curl_easy_setopt (curl, CURLOPT_CUSTOMREQUEST, method);
    }

  pkLog (client, PK_LOG_DEBUG, "%s %s", method, url);
  if (request->body && request->type
      && pkStrCmp (request->type, "application/octet-stream", PK_FALSE) != 0)
    {
      PK_TRACE (client, "%.*s", (PKInt)request->length,
                (const PKChar *)request->body);
    }

  PKStatus status = PK_OK;
  CURLcode code = curl_easy_perform (curl);
  if (code == CURLE_ABORTED_BY_CALLBACK && client->stop)
    {
      status = pkFail (client, PK_ESTOPPED, 0, "Stopped");
    }
  else if (code == CURLE_WRITE_ERROR && sink.failed)
    {
      status = request->file ? pkFail (client, PK_EFS, pkSysError (),
                                       "Cannot write the response: %s",
                                       pkSysErrorText (pkSysError ()))
                             : pkFailMem (client);
    }
  else if (code != CURLE_OK)
    {
      status = pkFail (client, PK_ENET, code, "Request failed: %s",
                       curl_easy_strerror (code));
    }
  else
    {
      CURLlong http = 0;
      curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http);
      response->status = http;
      pkLog (client, PK_LOG_DEBUG, "HTTP %ld", http);
      PK_TRACE (client, "%s", response->data ? response->data : "(empty)");
    }

  curl_slist_free_all (headers);
  curl_easy_cleanup (curl);
  if (status != PK_OK)
    {
      pkResponseFree (response);
    }

  return status;
}

PKStatus
pkExpect (PKClient *client, const PKResponse *response, PKLong expected)
{
  if (response->status == expected)
    {
      return PK_OK;
    }

  // Quote the server's own explanation, when it gives one.
  static const PKChar *const keys[]
      = { "message", "error_description", "error", "detail", "title" };
  cJSON *json = response->data ? cJSON_Parse (response->data) : NULL;
  const PKChar *message = NULL;
  for (PKSize i = 0;
       cJSON_IsObject (json) && !message && i < PK_COUNTOF (keys); ++i)
    {
      message = pkJsonString (json, keys[i]);
    }

  PKStatus status = response->status == 401 ? PK_EAUTH : PK_EHTTP;
  if (message)
    {
      pkFail (client, status, response->status,
              "Request failed with HTTP %ld: %s", response->status, message);
    }
  else
    {
      pkFail (client, status, response->status, "Request failed with HTTP %ld",
              response->status);
    }
  cJSON_Delete (json);

  return status;
}

PKStatus
pkResponseJson (PKClient *client, const PKResponse *response, cJSON **json)
{
  *json = response->data ? cJSON_Parse (response->data) : NULL;
  if (!cJSON_IsObject (*json))
    {
      cJSON_Delete (*json);
      *json = NULL;
      return pkFail (client, PK_EDATA, 0, "Malformed response");
    }

  return PK_OK;
}

PKVoid
pkResponseFree (PKResponse *response)
{
  pkMemFree (response->data);
  response->data = NULL;
  response->size = 0;
}

const PKChar *
pkJsonString (const cJSON *json, const PKChar *name)
{
  const cJSON *item = cJSON_GetObjectItemCaseSensitive (json, name);
  return cJSON_IsString (item) && item->valuestring ? item->valuestring : NULL;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKStatus
pkGlobalInit (void)
{
  return curl_global_init (CURL_GLOBAL_DEFAULT) == CURLE_OK ? PK_OK : PK_ENET;
}

PKVoid
pkGlobalCleanup (void)
{
  curl_global_cleanup ();
}
