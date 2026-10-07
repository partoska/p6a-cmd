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

#include "oauth.h"
#include "base64.h"
#include "cJSON.h"
#include "config.h"
#include "logger.h"
#include "types.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define CREDENTIALS_MAX (128)
#define FORMAT_MAX (256)
#define DEVICE_GRANT "urn:ietf:params:oauth:grant-type:device_code"
#define INTERVAL_DEFAULT (5)
#define SLOW_DOWN_SECS (5)
#define POLL_FAILURES_MAX (3)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct PLResponseBuffer
{
  PLChar *data;
  PLSize size;
} PLResponseBuffer;

typedef long CURLlong;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PLSize
plWriteCallback (PLChar *data, PLSize sz, PLSize n, void *uptr)
{
  PLSize size = sz * n;
  PLResponseBuffer *buff = (PLResponseBuffer *)uptr;
  PLChar *ndata = realloc (buff->data, buff->size + size + 1);
  if (!ndata)
    {
      PL_DEBUG ("Out of memory");
      return 0;
    }

  buff->data = ndata;
  memcpy (&(buff->data[buff->size]), data, size);
  buff->size += size;
  buff->data[buff->size] = '\0';

  return size;
}

static PLChar *
plUrlEncode (CURL *curl, const PLChar *s)
{
  return curl_easy_escape (curl, s, 0);
}

static void
plSleep (PLLong secs)
{
#ifdef _WIN32
  Sleep ((DWORD)(secs * 1000));
#else
  struct timespec ts;
  ts.tv_sec = (time_t)secs;
  ts.tv_nsec = 0;
  nanosleep (&ts, NULL);
#endif
}

/*
 * Posts a form to an OAuth endpoint and returns the response body, whatever
 * its status: the device flow reads its errors from 4xx bodies. Returns NULL
 * only when the request itself failed.
 */
static PLChar *
plPostForm (const PLChar *url, const PLChar *payload, CURLlong *httpcode)
{
  CURL *curl = curl_easy_init ();
  if (!curl)
    {
      PL_DEBUG ("Failed to initialize curl");
      return NULL;
    }
#ifdef _WIN32
  curl_easy_setopt (curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
#endif

  struct curl_slist *headers = NULL;
  headers = curl_slist_append (
      headers, "Content-Type: application/x-www-form-urlencoded");
  headers = curl_slist_append (headers, "User-Agent: p6a/" PL_VERSION_STRING);

  PLResponseBuffer response = { .data = NULL, .size = 0 };
  curl_easy_setopt (curl, CURLOPT_URL, url);
  curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt (curl, CURLOPT_POSTFIELDS, payload);
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, plWriteCallback);
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, &response);

  PL_DEBUG ("--- OAuth Request ---");
  PL_DEBUG ("URL: %s", url);
  PL_DSLOW ("POST data: %s", payload);
  PLChar *result = NULL;
  CURLcode res = curl_easy_perform (curl);
  if (res != CURLE_OK)
    {
      PL_DEBUG ("OAuth request failed: %s", curl_easy_strerror (res));
      goto cleanup;
    }

  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, httpcode);
  PL_DEBUG ("--- OAuth Response (HTTP %ld) ---", *httpcode);
  PL_DSLOW ("%s", response.data ? response.data : "(empty)");
  result = response.data ? response.data : strdup ("{}");
  response.data = NULL;

cleanup:
  curl_slist_free_all (headers);
  free (response.data);
  curl_easy_cleanup (curl);

  return result;
}

static PLChar *
plRequestDeviceCode (const PLCfgOAuth *config, CURLlong *httpcode)
{
  CURL *curl = curl_easy_init ();
  if (!curl)
    {
      PL_DEBUG ("Failed to initialize curl");
      return NULL;
    }

  PLChar *result = NULL;
  PLChar *eclient = plUrlEncode (curl, config->client);
  PLChar *escope = plUrlEncode (curl, config->scope);
  if (!eclient || !escope)
    {
      PL_DEBUG ("Failed to URL encode device request");
      goto cleanup;
    }

  PLSize size = strlen (eclient) + strlen (escope) + FORMAT_MAX;
  PLChar *payload = malloc (size);
  if (!payload)
    {
      PL_DEBUG ("Out of memory");
      goto cleanup;
    }

  snprintf (payload, size, "client_id=%s&scope=%s", eclient, escope);
  result = plPostForm (config->device, payload, httpcode);
  free (payload);

cleanup:
  curl_free (escope);
  curl_free (eclient);
  curl_easy_cleanup (curl);

  return result;
}

static PLChar *
plPollDeviceCode (const PLCfgOAuth *config, const PLChar *code,
                  CURLlong *httpcode)
{
  CURL *curl = curl_easy_init ();
  if (!curl)
    {
      PL_DEBUG ("Failed to initialize curl");
      return NULL;
    }

  PLChar *result = NULL;
  PLChar *eclient = plUrlEncode (curl, config->client);
  PLChar *ecode = plUrlEncode (curl, code);
  PLChar *egrant = plUrlEncode (curl, DEVICE_GRANT);
  if (!eclient || !ecode || !egrant)
    {
      PL_DEBUG ("Failed to URL encode token request");
      goto cleanup;
    }

  PLSize size = strlen (eclient) + strlen (ecode) + strlen (egrant);
  size += FORMAT_MAX;
  PLChar *payload = malloc (size);
  if (!payload)
    {
      PL_DEBUG ("Out of memory");
      goto cleanup;
    }

  snprintf (payload, size, "grant_type=%s&client_id=%s&device_code=%s",
            egrant, eclient, ecode);
  result = plPostForm (config->token, payload, httpcode);
  free (payload);

cleanup:
  curl_free (egrant);
  curl_free (ecode);
  curl_free (eclient);
  curl_easy_cleanup (curl);

  return result;
}

static const PLChar *
plJsonString (const cJSON *json, const PLChar *name)
{
  const cJSON *item = cJSON_GetObjectItemCaseSensitive (json, name);
  return (cJSON_IsString (item) && item->valuestring != NULL)
             ? item->valuestring
             : NULL;
}

static PLInt
plStoreTokens (PLCfg *config, const PLChar *ini, const cJSON *json)
{
  cJSON *expires = cJSON_GetObjectItemCaseSensitive (json, "expires_in");
  if (!cJSON_IsNumber (expires) || expires->valueint <= 0)
    {
      PL_ERROR ("Field 'expires_in' not found or invalid in response");
      return PL_EARG;
    }

  const PLChar *access = plJsonString (json, "access_token");
  if (!access)
    {
      PL_ERROR ("Field 'access_token' not found or invalid in response");
      return PL_EARG;
    }

  const PLChar *refresh = plJsonString (json, "refresh_token");
  if (!refresh)
    {
      PL_ERROR ("Field 'refresh_token' not found or invalid in response");
      return PL_EARG;
    }

  PLTime exp = time (NULL) + expires->valueint;
  if (plCfgSetLogin (config, access, refresh, exp) < 0)
    {
      PL_ERROR ("Failed to set login information");
      return PL_EARG;
    }

  if (plCfgSave (config, ini) < 0)
    {
      PL_ERROR ("Failed to save configuration");
      return PL_EARG;
    }

  return PL_EOK;
}

/*
 * Polls the token endpoint until the user decides or the code expires
 * (RFC 8628, section 3.4). Returns PL_EOK once the tokens are saved.
 */
static PLInt
plAwaitDeviceCode (PLCfg *config, const PLChar *ini, const PLChar *code,
                   PLLong interval, PLTime deadline)
{
  const PLCfgOAuth *auth = config->oauth;
  PLInt failures = 0;
  while (time (NULL) < deadline)
    {
      plSleep (interval);

      CURLlong httpcode = 0;
      PLChar *body = plPollDeviceCode (auth, code, &httpcode);
      if (!body)
        {
          // A dropped connection is not the user's answer, try again.
          if (++failures >= POLL_FAILURES_MAX)
            {
              PL_ERROR ("Could not reach Partoska, please try again later");
              return PL_ENET;
            }
          continue;
        }
      failures = 0;

      cJSON *json = cJSON_Parse (body);
      free (body);
      if (!json)
        {
          PL_ERROR ("Failed to parse JSON response");
          return PL_EARG;
        }

      if (httpcode == 200)
        {
          PLInt result = plStoreTokens (config, ini, json);
          cJSON_Delete (json);
          return result;
        }

      const PLChar *error = plJsonString (json, "error");
      PLInt result = PL_EOK;
      if (error && strcmp (error, "authorization_pending") == 0)
        {
          PL_DEBUG ("Authorization pending ...");
        }
      else if (error && strcmp (error, "slow_down") == 0)
        {
          interval += SLOW_DOWN_SECS;
          PL_DEBUG ("Slowing down to %ld seconds ...", interval);
        }
      else if (error && strcmp (error, "access_denied") == 0)
        {
          PL_ERROR ("Login denied in the browser");
          result = PL_EARG;
        }
      else if (error && strcmp (error, "expired_token") == 0)
        {
          PL_ERROR ("The code has expired, please run login again");
          result = PL_EARG;
        }
      else
        {
          PL_ERROR ("Login failed (%s)", error ? error : "unknown error");
          result = PL_ENET;
        }

      cJSON_Delete (json);
      if (result != PL_EOK)
        {
          return result;
        }
    }

  PL_ERROR ("The code has expired, please run login again");
  return PL_EARG;
}

static PLChar *
plOAuthRenew (PLCfg *config, const PLChar *ini)
{
  if (!ini)
    {
      PL_DEBUG ("Invalid INI path");
      return NULL;
    }

  if (!plCfgCheck (config))
    {
      PL_DEBUG ("Invalid configuration");
      return NULL;
    }

  const PLCfgLogin *login = config->login;
  if (login->refresh == NULL)
    {
      PL_DEBUG ("No refresh token found");
      return NULL;
    }

  CURL *curl = curl_easy_init ();
  if (!curl)
    {
      PL_DEBUG ("Failed to initialize curl");
      return NULL;
    }
#ifdef _WIN32
  curl_easy_setopt (curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
#endif

  PLChar *result = NULL;
  PLChar *erefresh = plUrlEncode (curl, login->refresh);
  if (!erefresh)
    {
      PL_DEBUG ("Failed to URL encode refresh token");
      goto cleanup_curl;
    }

  static const PLSize EXTRA_BYTES = 200;
  PLSize size = strlen (erefresh) + EXTRA_BYTES;
  PLChar *payload = malloc (size);
  if (!payload)
    {
      PL_DEBUG ("Out of memory");
      goto cleanup_curl;
    }

  snprintf (payload, size - 1, "grant_type=refresh_token&refresh_token=%s",
            erefresh);
  payload[size - 1] = '\0';

  const PLCfgOAuth *auth = config->oauth;
  PLChar cred[CREDENTIALS_MAX];
  snprintf (cred, PL_CHARSMAX (cred), "%s:", auth->client);
  cred[PL_CHARSMAX (cred)] = '\0';

  PLSize clen = strlen (cred);
  PLChar ecred[PL_BASE64_ENCODED_LEN (sizeof (cred)) + 1];
  if (plBase64Encode (ecred, PL_CHARSMAX (ecred), (PLByte *)cred, clen)
      != PL_EOK)
    {
      PL_DEBUG ("Failed to encode auth");
      goto cleanup_payload;
    }
  ecred[PL_CHARSMAX (ecred)] = '\0';

  PLChar authorization[sizeof (ecred) + 64];
  snprintf (authorization, PL_CHARSMAX (authorization),
            "Authorization: Basic %s", ecred);
  authorization[PL_CHARSMAX (authorization)] = '\0';

  struct curl_slist *headers = NULL;
  headers = curl_slist_append (
      headers, "Content-Type: application/x-www-form-urlencoded");
  headers = curl_slist_append (headers, "User-Agent: p6a/" PL_VERSION_STRING);
  headers = curl_slist_append (headers, authorization);

  PLResponseBuffer response = { 0 };
  curl_easy_setopt (curl, CURLOPT_URL, auth->token);
  curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt (curl, CURLOPT_POSTFIELDS, payload);
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, plWriteCallback);
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, &response);

  PL_DEBUG ("--- Token Refresh Request ---");
  PL_DEBUG ("URL: %s", auth->token);
  PL_DSLOW ("Authorization: Basic %s", ecred);
  PL_DSLOW ("POST data: %s", payload);
  CURLcode res = curl_easy_perform (curl);
  if (res != CURLE_OK)
    {
      PL_DEBUG ("Token refresh failed: %s", curl_easy_strerror (res));
      goto cleanup_response;
    }

  CURLlong httpcode;
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &httpcode);
  PL_DEBUG ("--- Token Refresh Response (HTTP %ld) ---", httpcode);
  PL_DSLOW ("%s", response.data ? response.data : "(empty)");
  if (httpcode != 200)
    {
      PL_DEBUG ("Token refresh failed with HTTP %ld", httpcode);
      goto cleanup_response;
    }

  cJSON *json = cJSON_Parse (response.data);
  if (!json)
    {
      PL_DEBUG ("Failed to parse JSON response");
      goto cleanup_response;
    }

  cJSON *expires = cJSON_GetObjectItemCaseSensitive (json, "expires_in");
  cJSON *access = cJSON_GetObjectItemCaseSensitive (json, "access_token");
  cJSON *refresh = cJSON_GetObjectItemCaseSensitive (json, "refresh_token");
  if (!cJSON_IsNumber (expires) || expires->valueint <= 0
      || !cJSON_IsString (access) || (access->valuestring == NULL))
    {
      PL_DEBUG ("Missing or invalid fields in refresh response");
      goto cleanup_json;
    }

  PLTime exp = time (NULL) + expires->valueint;
  const PLChar *nrefresh
      = (cJSON_IsString (refresh) && (refresh->valuestring != NULL))
            ? refresh->valuestring
            : config->login->refresh;

  if (plCfgSetLogin (config, access->valuestring, nrefresh, exp) != PL_EOK)
    {
      PL_DEBUG ("Failed to set login");
      goto cleanup_json;
    }
  if (plCfgSave (config, ini) != PL_EOK)
    {
      PL_DEBUG ("Failed to save configuration");
      goto cleanup_json;
    }

  result = strdup (access->valuestring);
  if (result == NULL)
    {
      PL_DEBUG ("Out of memory");
      goto cleanup_json;
    }

cleanup_json:
  cJSON_Delete (json);
cleanup_response:
  curl_slist_free_all (headers);
  free (response.data);
cleanup_payload:
  free (payload);
cleanup_curl:
  curl_free (erefresh);
  curl_easy_cleanup (curl);

  return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PLInt
plOAuthLogin (PLCfg *config, const PLChar *ini)
{
  if (!ini)
    {
      PL_ERROR ("Invalid INI path");
      return PL_EARG;
    }

  if (!plCfgCheck (config))
    {
      PL_ERROR ("Invalid configuration");
      return PL_EARG;
    }

  const PLCfgOAuth *auth = config->oauth;
  PL_DSLOW ("OAuth Configuration:");
  PL_DSLOW ("  Client:         %s", auth->client);
  PL_DSLOW ("  Device:         %s", auth->device);
  PL_DSLOW ("  Token:          %s", auth->token);
  PL_DSLOW ("  Scope:          %s", auth->scope);

  CURLlong httpcode = 0;
  PLChar *body = plRequestDeviceCode (auth, &httpcode);
  if (!body)
    {
      PL_ERROR ("Could not reach Partoska, please try again later");
      return PL_ENET;
    }

  cJSON *json = cJSON_Parse (body);
  free (body);
  if (!json)
    {
      PL_ERROR ("Failed to parse JSON response");
      return PL_EARG;
    }

  PLInt result = PL_EOK;
  if (httpcode != 200)
    {
      const PLChar *error = plJsonString (json, "error");
      PL_ERROR ("Could not start login (%s)", error ? error : "unknown error");
      result = PL_ENET;
      goto cleanup_json;
    }

  const PLChar *code = plJsonString (json, "device_code");
  const PLChar *user = plJsonString (json, "user_code");
  const PLChar *uri = plJsonString (json, "verification_uri");
  const PLChar *complete = plJsonString (json, "verification_uri_complete");
  cJSON *expires = cJSON_GetObjectItemCaseSensitive (json, "expires_in");
  cJSON *interval = cJSON_GetObjectItemCaseSensitive (json, "interval");
  if (!code || !user || !uri || !cJSON_IsNumber (expires)
      || expires->valueint <= 0)
    {
      PL_ERROR ("Invalid device authorization response");
      result = PL_EARG;
      goto cleanup_json;
    }

  PLLong wait = (cJSON_IsNumber (interval) && interval->valueint > 0)
                    ? (PLLong)interval->valueint
                    : INTERVAL_DEFAULT;
  PLTime deadline = time (NULL) + expires->valueint;

  PL_INFO ("Open this URL in a browser, on this or any other device:");
  PL_INFO ("");
  PL_INFO ("  %s", complete ? complete : uri);
  PL_INFO ("");
  PL_INFO ("Check that it shows the code: %s", user);
  PL_INFO ("");
  PL_INFO ("Waiting for you to allow access (expires in %d minutes) ...",
           (expires->valueint + 59) / 60);

  result = plAwaitDeviceCode (config, ini, code, wait, deadline);
  if (result == PL_EOK)
    {
      PL_INFO ("");
      PL_INFO ("Login successful!");
    }

cleanup_json:
  cJSON_Delete (json);

  return result;
}

PLChar *
plOAuthGet (PLCfg *config, const PLChar *ini)
{
  if (!ini)
    {
      PL_ERROR ("Invalid INI path");
      return NULL;
    }

  if (!plCfgCheck (config))
    {
      PL_ERROR ("Invalid configuration");
      return NULL;
    }

  if ((config->login->access == NULL || strlen (config->login->access) == 0)
      && (config->login->refresh == NULL
          || strlen (config->login->refresh) == 0))
    {
      PL_ERROR ("No token found, please login again");
      return NULL;
    }

  if (config->login->access == NULL)
    {
      PL_DEBUG ("No token found, refreshing ...");
      PLChar *ntoken = plOAuthRenew (config, ini);
      if (!ntoken)
        {
          PL_ERROR ("Refresh failed, if this persists, please (re-)login");
          return NULL;
        }
      return ntoken;
    }

  PLTime now = time (NULL);
  PLTime threshold = 15 * 60;
  PLTime delta = PL_MAX (config->login->expires - now, 0);
  if (delta <= 0)
    {
      PL_DEBUG ("Token expired, refreshing ...");
      PLChar *ntoken = plOAuthRenew (config, ini);
      if (!ntoken)
        {
          PL_ERROR ("Refresh failed, if this persists, please (re-)login");
          return NULL;
        }
      return ntoken;
    }

  if (delta < threshold)
    {
      PL_DEBUG ("Token expires soon (%ld), refreshing ...", delta);
      PLChar *ntoken = plOAuthRenew (config, ini);
      if (ntoken)
        {
          return ntoken;
        }
      else
        {
          PL_WARN ("Refresh failed, using the current token ...");
        }
    }

  PLChar *result = strdup (config->login->access);
  if (!result)
    {
      PL_ERROR ("Out of memory");
      return NULL;
    }

  return result;
}
