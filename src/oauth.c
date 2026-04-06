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
#include "hash.h"
#include "logger.h"
#include "rng.h"
#include "types.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define VERIFIER_LENGTH (64)
#define VERIFIER_MAX ((VERIFIER_LENGTH) + (16))
#define STATE_LENGTH (32)
#define STATE_MAX (PL_BASE64_ENCODED_LEN (STATE_LENGTH) + (16))
#define CHALLENGE_MAX (PL_BASE64_ENCODED_LEN (PL_HASH_DIGEST_SIZE) + (16))
#define INPUT_MAX (128)
#define URL_MAX (1024)
#define CREDENTIALS_MAX (128)
#define FORMAT_MAX (256)

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

static PLInt
plGenVerifier (PLChar *verifier, PLSize len)
{
  PLByte bytes[VERIFIER_LENGTH];
  if (len < sizeof (bytes))
    {
      return PL_EMEM;
    }

  PLInt result = plGenRandomBytes (bytes, VERIFIER_LENGTH);
  if (result != PL_EOK)
    {
      return result;
    }

  for (PLSize i = 0; i < sizeof (bytes); ++i)
    {
      verifier[i] = plBase64Char (bytes[i]);
    }

  // Optional null termination.
  if (len > sizeof (bytes))
    {
      verifier[sizeof (bytes)] = '\0';
    }

  return PL_EOK;
}

static PLInt
plGenChallenge (PLChar *challenge, PLSize len, const PLChar *verifier)
{
  PLHashCtx ctx;
  PLByte hash[PL_HASH_DIGEST_SIZE];
  if (len < PL_BASE64_ENCODED_LEN (sizeof (hash)))
    {
      return PL_EMEM;
    }

  plHashInit (&ctx);
  plHashUpdate (&ctx, (const PLByte *)verifier, strlen (verifier));
  plHashFinal (&ctx, hash);

  PLInt result = plBase64UrlEncode (challenge, len, hash, PL_HASH_DIGEST_SIZE);
  if (result != PL_EOK)
    {
      return result;
    }

  // Optional null termination.
  if (len > PL_BASE64_ENCODED_LEN (sizeof (hash)))
    {
      challenge[PL_BASE64_ENCODED_LEN (sizeof (hash))] = '\0';
    }

  return PL_EOK;
}

static PLInt
plGenState (PLChar *state, PLSize len)
{
  PLByte bytes[STATE_LENGTH];
  if (len < PL_BASE64_ENCODED_LEN (sizeof (bytes)))
    {
      return PL_EMEM;
    }

  PLInt result = plGenRandomBytes (bytes, STATE_LENGTH);
  if (result != PL_EOK)
    {
      return result;
    }

  result = plBase64UrlEncode (state, len, bytes, STATE_LENGTH);
  if (result != PL_EOK)
    {
      return result;
    }

  // Optional null termination.
  if (len > PL_BASE64_ENCODED_LEN (sizeof (bytes)))
    {
      state[PL_BASE64_ENCODED_LEN (sizeof (bytes))] = '\0';
    }

  return PL_EOK;
}

static PLChar *
plBuildUrl (const PLCfgOAuth *config, const PLChar *challenge,
            const PLChar *state)
{
  if (config == NULL)
    {
      PL_DEBUG ("Invalid config");
      return NULL;
    }

  CURL *curl = curl_easy_init ();
  if (!curl)
    {
      PL_DEBUG ("Failed to initialize cURL");
      return NULL;
    }

  PLChar *eredirect = plUrlEncode (curl, config->redirect);
  PLChar *eclient = plUrlEncode (curl, config->client);
  PLChar *echallenge = plUrlEncode (curl, challenge);
  PLChar *estate = plUrlEncode (curl, state);
  PLChar *escope = plUrlEncode (curl, config->scope);
  if (!eredirect || !eclient || !echallenge || !estate || !escope)
    {
      curl_free (eredirect);
      curl_free (eclient);
      curl_free (echallenge);
      curl_free (estate);
      curl_free (escope);
      curl_easy_cleanup (curl);
      return NULL;
    }

  PLSize size = strlen (config->authorize);
  size += strlen (eredirect);
  size += strlen (eclient);
  size += strlen (echallenge);
  size += strlen (estate);
  size += strlen (escope);
  size += FORMAT_MAX;
  PLChar *url = malloc (size);
  if (!url)
    {
      curl_free (eredirect);
      curl_free (eclient);
      curl_free (echallenge);
      curl_free (estate);
      curl_free (escope);
      curl_easy_cleanup (curl);
      return NULL;
    }
  snprintf (url, size - 1,
            "%s?response_type=code"
            "&client_id=%s"
            "&redirect_uri=%s"
            "&code_challenge=%s"
            "&code_challenge_method=S256"
            "&state=%s"
            "&scope=%s",
            config->authorize, eclient, eredirect, echallenge, estate, escope);
  url[size - 1] = '\0';

  curl_free (eredirect);
  curl_free (eclient);
  curl_free (echallenge);
  curl_free (estate);
  curl_free (escope);
  curl_easy_cleanup (curl);

  return url;
}

static PLInt
plRedirectWait (PLChar *code, PLSize size)
{
  PL_INFO ("");
  PL_INFO ("Enter pairing code below:");
  fprintf (stdout, "> ");
  fflush (stdout);
  if (fgets (code, size, stdin) == NULL)
    {
      PL_DEBUG ("Failed to read input");
      return PL_EARG;
    }

  PL_INFO ("");

  PLSize len = strlen (code);
  if (len > 0 && code[len - 1] == '\n')
    {
      code[len - 1] = '\0';
    }

  return PL_EOK;
}

static PLBool
plConfirm (const PLChar *prompt)
{
  fprintf (stdout, "%s (Enter=Yes,C=Cancel):", prompt);
  fflush (stdout);
  PLInt c = getchar ();
  if (c == EOF)
    {
      PL_DEBUG ("Failed to read input");
      return PL_FALSE;
    }
  return (c == '\n');
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

static PLChar *
plExchangeCode (const PLCfgOAuth *config, const PLChar *code,
                const PLChar *verifier)
{
  if (config == NULL)
    {
      PL_DEBUG ("Invalid configuration");
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
  PLChar *eredirect = plUrlEncode (curl, config->redirect);
  PLChar *ecode = plUrlEncode (curl, code);
  PLChar *everifier = plUrlEncode (curl, verifier);
  if (!eredirect || !ecode || !everifier)
    {
      PL_DEBUG ("Failed to URL encode redirect uri");
      goto cleanup_curl;
    }

  PLSize size = strlen (eredirect);
  size += strlen (ecode);
  size += strlen (everifier);
  size += FORMAT_MAX;
  PLChar *payload = malloc (size);
  if (!payload)
    {
      PL_DEBUG ("Out of memory");
      goto cleanup_curl;
    }

  snprintf (payload, size - 1,
            "grant_type=authorization_code"
            "&code=%s"
            "&redirect_uri=%s"
            "&code_verifier=%s",
            ecode, eredirect, everifier);
  payload[size - 1] = '\0';

  PLChar cred[CREDENTIALS_MAX];
  snprintf (cred, PL_CHARSMAX (cred), "%s:", config->client);
  cred[PL_CHARSMAX (cred) - 1] = '\0';

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
  authorization[PL_CHARSMAX (authorization) - 1] = '\0';

  struct curl_slist *headers = NULL;
  headers = curl_slist_append (
      headers, "Content-Type: application/x-www-form-urlencoded");
  headers = curl_slist_append (headers, "User-Agent: p6a/" PL_VERSION_STRING);
  headers = curl_slist_append (headers, authorization);

  PLResponseBuffer response = { .data = NULL, .size = 0 };
  curl_easy_setopt (curl, CURLOPT_URL, config->token);
  curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt (curl, CURLOPT_POSTFIELDS, payload);
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, plWriteCallback);
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, &response);

  PL_DEBUG ("--- Token Exchange Request ---");
  PL_DEBUG ("URL: %s", config->token);
  PL_DSLOW ("Authorization: Basic %s", ecred);
  PL_DSLOW ("POST data: %s", payload);
  CURLcode res = curl_easy_perform (curl);
  if (res != CURLE_OK)
    {
      PL_DEBUG ("Token exchange failed: %s", curl_easy_strerror (res));
      goto cleanup_response;
    }

  CURLlong httpcode;
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &httpcode);
  PL_DEBUG ("--- Token Exchange Response (HTTP %ld) ---", httpcode);
  PL_DSLOW ("%s", response.data ? response.data : "(empty)");
  if (httpcode != 200)
    {
      PL_DEBUG ("Token exchange failed with HTTP %ld", httpcode);
      goto cleanup_response;
    }

  if (httpcode == 200)
    {
      result = response.data ? strdup (response.data) : strdup ("{}");
    }
  else
    {
      PL_DEBUG ("Token exchange failed with HTTP %ld", httpcode);
    }

cleanup_response:
  curl_slist_free_all (headers);
  free (response.data);
cleanup_payload:
  free (payload);
cleanup_curl:
  curl_free (everifier);
  curl_free (ecode);
  curl_free (eredirect);
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
  PL_DSLOW ("  Redirect:       %s", auth->redirect);
  PL_DSLOW ("  Authorize:      %s", auth->authorize);
  PL_DSLOW ("  Token:          %s", auth->token);
  PL_DSLOW ("  Scope:          %s", auth->scope);

  PLChar verifier[VERIFIER_MAX];
  PLInt result = plGenVerifier (verifier, PL_CHARSMAX (verifier));
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to generate code verifier");
      return result;
    }
  verifier[PL_CHARSMAX (verifier)] = '\0';

  PLChar challenge[CHALLENGE_MAX];
  result = plGenChallenge (challenge, PL_CHARSMAX (challenge), verifier);
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to generate code challenge");
      return result;
    }
  challenge[PL_CHARSMAX (challenge)] = '\0';

  PLChar state[STATE_MAX];
  result = plGenState (state, PL_CHARSMAX (state));
  if (result != PL_EOK)
    {
      PL_ERROR ("Failed to generate state");
      return result;
    }
  state[PL_CHARSMAX (state)] = '\0';

  PL_DSLOW ("Generated PKCE Parameters:");
  PL_DSLOW ("  Code Verifier:  %s", verifier);
  PL_DSLOW ("  Code Challenge: %s", challenge);
  PL_DSLOW ("  State:          %s", state);
  PLChar *url = plBuildUrl (auth, challenge, state);
  if (!url)
    {
      PL_ERROR ("Failed to build authorization URL");
      return PL_EMEM;
    }

  if (!plConfirm ("Step 1/2: Show authorization URL?"))
    {
      PL_INFO ("");
      PL_INFO ("Login cancelled by user.");
      result = PL_EOK;
      goto cleanup_url;
    }

  PL_INFO ("");
  PL_INFO ("Open this URL in your browser:");
  PL_INFO ("%s", url);
  PL_INFO ("");

  if (!plConfirm ("Step 2/2: Ready to paste copied code?"))
    {
      PL_INFO ("");
      PL_INFO ("Login cancelled by user.");
      result = PL_EOK;
      goto cleanup_url;
    }

  PLChar input[INPUT_MAX];
  result = plRedirectWait (input, sizeof (input));
  if (result != PL_EOK)
    {
      PL_ERROR ("No pairing code received");
      goto cleanup_url;
    }

  PLChar *dot = strchr (input, '.');
  if (!dot)
    {
      PL_ERROR ("Invalid pairing code format");
      result = PL_EARG;
      goto cleanup_url;
    }

  *dot = '\0';
  PLChar *icode = input;
  PLChar *istate = dot + 1;
  if (strcmp (istate, state) != 0)
    {
      PL_ERROR ("State mismatch");
      result = PL_EARG;
      goto cleanup_url;
    }

  PLChar *tokens = plExchangeCode (auth, icode, verifier);
  if (!tokens)
    {
      PL_ERROR ("Could not exchange code");
      result = PL_ENET;
      goto cleanup_url;
    }

  cJSON *json = cJSON_Parse (tokens);
  if (!json)
    {
      PL_ERROR ("Failed to parse JSON response");
      result = PL_EARG;
      goto cleanup_tokens;
    }

  cJSON *expires = cJSON_GetObjectItemCaseSensitive (json, "expires_in");
  if (!cJSON_IsNumber (expires) || expires->valueint <= 0)
    {
      PL_ERROR ("Field 'expires_in' not found or invalid in response");
      result = PL_EARG;
      goto cleanup_json;
    }

  cJSON *access = cJSON_GetObjectItemCaseSensitive (json, "access_token");
  if (!cJSON_IsString (access) || (access->valuestring == NULL))
    {
      PL_ERROR ("Field 'access_token' not found or invalid in response");
      result = PL_EARG;
      goto cleanup_json;
    }

  cJSON *refresh = cJSON_GetObjectItemCaseSensitive (json, "refresh_token");
  if (!cJSON_IsString (refresh) || (refresh->valuestring == NULL))
    {
      PL_ERROR ("Field 'refresh_token' not found or invalid in response");
      result = PL_EARG;
      goto cleanup_json;
    }

  PLTime exp = time (NULL) + expires->valueint;
  if (plCfgSetLogin (config, access->valuestring, refresh->valuestring, exp)
      < 0)
    {
      PL_ERROR ("Failed to set login information");
      result = PL_EARG;
      goto cleanup_json;
    }

  if (plCfgSave (config, ini) < 0)
    {
      PL_ERROR ("Failed to save configuration");
      result = PL_EARG;
      goto cleanup_json;
    }

  result = PL_EOK;
  PL_INFO ("Login successful!");

cleanup_json:
  cJSON_Delete (json);
cleanup_tokens:
  free (tokens);
cleanup_url:
  free (url);

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
