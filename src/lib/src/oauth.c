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

#include "base64.h"
#include "http.h"
#include <curl/curl.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define CREDENTIALS_MAX (128)
#define DEVICE_GRANT "urn:ietf:params:oauth:grant-type:device_code"
#define FORM_TYPE "application/x-www-form-urlencoded"

/* RFC 8628: poll every 5 seconds unless told otherwise, 5 more after
 * slow_down. Unreachable servers are retried a few times. */
#define INTERVAL_DEFAULT (5)
#define SLOW_DOWN_SECS (5)
#define POLL_FAILURES_MAX (3)

/* Renew an access token that expires within this many seconds. */
#define RENEW_AHEAD (15 * 60)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Builds a form body from name/value pairs, URL-encoding the values:
 * pkForm (client, &body, "a", va, "b", vb, NULL). Release with pkMemFree().
 */
static PKStatus
pkForm (PKClient *client, PKChar **body, const PKChar *name, ...)
{
  CURL *curl = curl_easy_init ();
  if (!curl)
    {
      return pkFailMem (client);
    }

  PKStatus status = PK_OK;
  PKSize size = 1;
  *body = pkMemZalloc (size);

  PKVaList args;
  PK_VA_START (args, name);
  for (; name && *body; name = PK_VA_ARG (args, const PKChar *))
    {
      const PKChar *value = PK_VA_ARG (args, const PKChar *);
      PKChar *escaped = curl_easy_escape (curl, value, 0);
      PKSize more = escaped ? pkStrLen (name) + pkStrLen (escaped) + 2 : 0;
      PKChar *grown = escaped ? pkMemRealloc (*body, size + more) : NULL;
      if (!grown)
        {
          curl_free (escaped);
          pkMemFree (*body);
          *body = NULL;
          break;
        }

      // Appends "&name=value" (no "&" first) over the terminating NUL.
      *body = grown;
      size += (PKSize)pkStrFmt (*body + size - 1, more + 1, "%s%s=%s",
                                size > 1 ? "&" : "", name, escaped);
      curl_free (escaped);
    }
  PK_VA_END (args);

  curl_easy_cleanup (curl);
  if (!*body)
    {
      status = pkFailMem (client);
    }

  return status;
}

/* Posts a form to an OAuth endpoint; any HTTP status is an answer. */
static PKStatus
pkPost (PKClient *client, const PKChar *url, const PKChar *body,
        const PKChar *basic, PKResponse *response)
{
  PKRequest request = { 0 };
  request.method = "POST";
  request.url = url;
  request.type = FORM_TYPE;
  request.body = body;
  request.length = pkStrLen (body);
  request.basic = basic;

  return pkRequest (client, &request, response);
}

/* Stores the tokens of a token response and saves the login. */
static PKStatus
pkStoreTokens (PKClient *client, const cJSON *json, PKBool keepRefresh)
{
  const cJSON *expires = cJSON_GetObjectItemCaseSensitive (json, "expires_in");
  const PKChar *access = pkJsonString (json, "access_token");
  const PKChar *refresh = pkJsonString (json, "refresh_token");
  if (!refresh && keepRefresh)
    {
      refresh = client->cfg->login->refresh;
    }
  if (!cJSON_IsNumber (expires) || expires->valueint <= 0 || !access
      || !refresh)
    {
      return pkFail (client, PK_EDATA, 0, "Malformed token response");
    }

  PKStatus status = pkCfgSetLogin (client->cfg, access, refresh,
                                   pkTimeNow () + expires->valueint);
  if (status != PK_OK)
    {
      return pkFailMem (client);
    }

  return *client->ini ? pkCfgSave (client, client->cfg, client->ini) : PK_OK;
}

/* Renews the access token with the refresh token. */
static PKStatus
pkRenew (PKClient *client)
{
  const PKCfg *cfg = client->cfg;
  if (!cfg->login->refresh || !*cfg->login->refresh)
    {
      return pkFail (client, PK_EAUTH, 0, "Not logged in");
    }

  // The client authenticates with its id and an empty secret.
  PKChar credentials[CREDENTIALS_MAX];
  pkStrFmt (credentials, sizeof (credentials), "%s:", cfg->oauth->client);
  PKChar basic[PK_BASE64_ENCODED_LEN (sizeof (credentials)) + 1];
  if (pkBase64Encode (basic, sizeof (basic), (const PKByte *)credentials,
                      pkStrLen (credentials))
      != PK_OK)
    {
      return pkFail (client, PK_EARG, 0, "Client id too long");
    }

  PKChar *body;
  PKStatus status = pkForm (client, &body, "grant_type", "refresh_token",
                            "refresh_token", cfg->login->refresh, NULL);
  if (status != PK_OK)
    {
      return status;
    }

  pkLog (client, PK_LOG_DEBUG, "Renewing the login");
  PKResponse response;
  status = pkPost (client, cfg->oauth->token, body, basic, &response);
  pkMemFree (body);
  if (status != PK_OK)
    {
      return status;
    }

  cJSON *json = NULL;
  if (response.status == 400 || response.status == 401)
    {
      status = pkFail (client, PK_EAUTH, response.status,
                       "Login refused (HTTP %ld)", response.status);
    }
  else if ((status = pkExpect (client, &response, 200)) == PK_OK
           && (status = pkResponseJson (client, &response, &json)) == PK_OK)
    {
      status = pkStoreTokens (client, json, PK_TRUE);
    }

  cJSON_Delete (json);
  pkResponseFree (&response);

  return status;
}

/*
 * Polls the token endpoint until the user decides or the code expires
 * (RFC 8628, section 3.4).
 */
static PKStatus
pkAwait (PKClient *client, const PKChar *code, PKLong interval,
         PKTime deadline)
{
  PKChar *body;
  PKStatus status
      = pkForm (client, &body, "grant_type", DEVICE_GRANT, "client_id",
                client->cfg->oauth->client, "device_code", code, NULL);
  if (status != PK_OK)
    {
      return status;
    }

  PKInt failures = 0;
  status = PK_EAUTH;
  while (pkTimeNow () < deadline)
    {
      status = pkNap (client, interval * 1000);
      if (status != PK_OK)
        {
          break;
        }

      PKResponse response;
      status
          = pkPost (client, client->cfg->oauth->token, body, NULL, &response);
      if (status == PK_ENET && ++failures < POLL_FAILURES_MAX)
        {
          // A dropped connection is not the user's answer, try again.
          pkLog (client, PK_LOG_WARNING, "%s, retrying",
                 pkError (client)->message);
          continue;
        }
      if (status != PK_OK)
        {
          break;
        }
      failures = 0;

      cJSON *json = NULL;
      status = pkResponseJson (client, &response, &json);
      if (status == PK_OK && response.status == 200)
        {
          status = pkStoreTokens (client, json, PK_FALSE);
          cJSON_Delete (json);
          pkResponseFree (&response);
          break;
        }

      const PKChar *error = json ? pkJsonString (json, "error") : NULL;
      if (status != PK_OK)
        {
          // Malformed answer, recorded.
        }
      else if (error
               && pkStrCmp (error, "authorization_pending", PK_FALSE) == 0)
        {
          pkLog (client, PK_LOG_DEBUG, "Authorization pending");
        }
      else if (error && pkStrCmp (error, "slow_down", PK_FALSE) == 0)
        {
          interval += SLOW_DOWN_SECS;
          pkLog (client, PK_LOG_DEBUG, "Slowing down to %ld seconds",
                 interval);
        }
      else if (error && pkStrCmp (error, "access_denied", PK_FALSE) == 0)
        {
          status = pkFail (client, PK_EAUTH, response.status,
                           "Login denied in the browser");
        }
      else if (error && pkStrCmp (error, "expired_token", PK_FALSE) == 0)
        {
          status = pkFail (client, PK_EAUTH, response.status,
                           "The code has expired");
        }
      else
        {
          status
              = pkFail (client, PK_EHTTP, response.status, "Login failed (%s)",
                        error ? error : "unknown error");
        }

      cJSON_Delete (json);
      pkResponseFree (&response);
      if (status != PK_OK)
        {
          break;
        }
      status = PK_EAUTH;
    }

  pkMemFree (body);
  if (status == PK_EAUTH && pkTimeNow () >= deadline)
    {
      status = pkFail (client, PK_EAUTH, 0, "The code has expired");
    }

  return status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Internal
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKStatus
pkBearer (PKClient *client, const PKChar **token)
{
  const PKCfgLogin *login = client->cfg->login;
  if (!pkLoginValid (client))
    {
      return pkFail (client, PK_EAUTH, 0, "Not logged in");
    }

  PKTime left = login->expires - pkTimeNow ();
  if (!login->access || !*login->access || left <= 0)
    {
      pkLog (client, PK_LOG_DEBUG, "Access token expired, renewing");
      PKStatus status = pkRenew (client);
      if (status != PK_OK)
        {
          return pkErrorContext (client, "Cannot renew the login");
        }
    }
  else if (left < RENEW_AHEAD && pkRenew (client) != PK_OK)
    {
      // Still valid for a while; the next call tries again.
      pkLog (client, PK_LOG_WARNING, "Cannot renew the login: %s",
             pkError (client)->message);
    }

  *token = client->cfg->login->access;

  return PK_OK;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKStatus
pkLogin (PKClient *client, PKLoginCodeFn prompt, PKVoid *user)
{
  if (!prompt)
    {
      return pkFail (client, PK_EARG, 0, "No prompt for the login");
    }

  const PKCfgOAuth *auth = client->cfg->oauth;
  pkLog (client, PK_LOG_DEBUG, "Device %s, token %s, client %s, scope %s",
         auth->device, auth->token, auth->client, auth->scope);

  PKChar *body;
  PKStatus status = pkForm (client, &body, "client_id", auth->client, "scope",
                            auth->scope, NULL);
  if (status != PK_OK)
    {
      return status;
    }

  PKResponse response;
  status = pkPost (client, auth->device, body, NULL, &response);
  pkMemFree (body);
  if (status != PK_OK)
    {
      return pkErrorContext (client, "Cannot start the login");
    }

  cJSON *json = NULL;
  status = pkExpect (client, &response, 200);
  if (status == PK_OK)
    {
      status = pkResponseJson (client, &response, &json);
    }
  pkResponseFree (&response);
  if (status != PK_OK)
    {
      return pkErrorContext (client, "Cannot start the login");
    }

  const PKChar *code = pkJsonString (json, "device_code");
  const PKChar *usercode = pkJsonString (json, "user_code");
  const PKChar *uri = pkJsonString (json, "verification_uri");
  const PKChar *complete = pkJsonString (json, "verification_uri_complete");
  const cJSON *expires = cJSON_GetObjectItemCaseSensitive (json, "expires_in");
  const cJSON *interval = cJSON_GetObjectItemCaseSensitive (json, "interval");
  if (!code || !usercode || !uri || !cJSON_IsNumber (expires)
      || expires->valueint <= 0)
    {
      cJSON_Delete (json);
      return pkFail (client, PK_EDATA, 0,
                     "Malformed device authorization response");
    }

  PKLong wait = cJSON_IsNumber (interval) && interval->valueint > 0
                    ? (PKLong)interval->valueint
                    : INTERVAL_DEFAULT;
  PKTime deadline = pkTimeNow () + expires->valueint;

  prompt (user, complete ? complete : uri, usercode, expires->valueint);
  status = pkAwait (client, code, wait, deadline);
  cJSON_Delete (json);

  return status;
}
