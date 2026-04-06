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

#include "api.h"
#include "cJSON.h"
#include "fs.h"
#include "logger.h"
#include "throttle.h"
#include "types.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define URL_MAX (1024)
#define AUTHORIZATION_MAX (1024)
#define THROTTLE_RATE (1)
#define THROTTLE_BURST (5)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
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

static PLThrottle throttle;

static void
plEnsureThrottle (void)
{
  static PLBool init = PL_FALSE;
  if (!init)
    {
      plThrottleInit (&throttle, THROTTLE_RATE, THROTTLE_BURST);
      init = PL_TRUE;
    }
}

static PLSize
plWriteCallback (PLChar *data, PLSize sz, PLSize n, void *uptr)
{
  PLSize size = sz * n;
  PLResponseBuffer *buf = (PLResponseBuffer *)uptr;
  PLChar *ndata = realloc (buf->data, buf->size + size + 1);
  if (!ndata)
    {
      PL_DEBUG ("Out of memory");
      return 0;
    }

  buf->data = ndata;
  memcpy (&(buf->data[buf->size]), data, size);
  buf->size += size;
  buf->data[buf->size] = '\0';

  return size;
}

static PLSize
plDiscardCallback (PLChar *data, PLSize sz, PLSize n, void *uptr)
{
  PL_UNUSED (data);
  PL_UNUSED (uptr);
  return sz * n;
}

static PLSize
plWriteFileCallback (PLChar *data, PLSize sz, PLSize n, void *uptr)
{
  PLFile *file = (PLFile *)uptr;
  if (!file)
    {
      PL_DEBUG ("Invalid file");
      return 0;
    }
  return fwrite (data, sz, n, file);
}

static PLBool
plParseEvent (cJSON *json, PLEvent *event)
{
  cJSON *id = cJSON_GetObjectItemCaseSensitive (json, "id");
  if (!cJSON_IsString (id) || (id->valuestring == NULL))
    {
      return PL_FALSE;
    }
  strncpy (event->id, id->valuestring, PL_CHARSMAX (event->id));
  event->id[PL_CHARSMAX (event->id)] = '\0';

  cJSON *name = cJSON_GetObjectItemCaseSensitive (json, "name");
  if (!cJSON_IsString (name) || (name->valuestring == NULL))
    {
      return PL_FALSE;
    }
  strncpy (event->name, name->valuestring, PL_CHARSMAX (event->name));
  event->name[PL_CHARSMAX (event->name)] = '\0';

  cJSON *created = cJSON_GetObjectItemCaseSensitive (json, "created");
  if (created && cJSON_IsString (created) && (created->valuestring != NULL))
    {
      strncpy (event->created, created->valuestring,
               PL_CHARSMAX (event->created));
      event->created[PL_CHARSMAX (event->created)] = '\0';
    }
  else
    {
      event->created[0] = '\0';
    }

  cJSON *expires = cJSON_GetObjectItemCaseSensitive (json, "expires");
  if (expires && cJSON_IsString (expires) && (expires->valuestring != NULL))
    {
      strncpy (event->expires, expires->valuestring,
               PL_CHARSMAX (event->expires));
      event->expires[PL_CHARSMAX (event->expires)] = '\0';
    }
  else
    {
      event->expires[0] = '\0';
    }

  cJSON *from = cJSON_GetObjectItemCaseSensitive (json, "from");
  if (!cJSON_IsString (from) || (from->valuestring == NULL))
    {
      return PL_FALSE;
    }
  strncpy (event->from, from->valuestring, PL_CHARSMAX (event->from));
  event->from[PL_CHARSMAX (event->from)] = '\0';

  cJSON *to = cJSON_GetObjectItemCaseSensitive (json, "to");
  if (!cJSON_IsString (to) || (to->valuestring == NULL))
    {
      return PL_FALSE;
    }
  strncpy (event->to, to->valuestring, PL_CHARSMAX (event->to));
  event->to[PL_CHARSMAX (event->to)] = '\0';

  cJSON *owner = cJSON_GetObjectItemCaseSensitive (json, "owner");
  if (!cJSON_IsBool (owner))
    {
      return PL_FALSE;
    }
  event->owner = cJSON_IsTrue (owner) ? 1 : 0;

  cJSON *guests = cJSON_GetObjectItemCaseSensitive (json, "guests");
  if (!cJSON_IsNumber (guests))
    {
      return PL_FALSE;
    }
  event->guests = guests->valueint;

  cJSON *media = cJSON_GetObjectItemCaseSensitive (json, "media");
  if (!cJSON_IsNumber (media))
    {
      return PL_FALSE;
    }
  event->media = media->valueint;

  cJSON *favorite = cJSON_GetObjectItemCaseSensitive (json, "favorite");
  if (!cJSON_IsBool (favorite))
    {
      return PL_FALSE;
    }
  event->favorite = cJSON_IsTrue (favorite) ? 1 : 0;

  cJSON *pub = cJSON_GetObjectItemCaseSensitive (json, "public");
  if (!cJSON_IsBool (pub))
    {
      return PL_FALSE;
    }
  event->pub = cJSON_IsTrue (pub) ? 1 : 0;

  return PL_TRUE;
}

static PLBool
plParseMedia (cJSON *json, PLMedia *media)
{
  cJSON *id = cJSON_GetObjectItemCaseSensitive (json, "id");
  if (!cJSON_IsString (id) || (id->valuestring == NULL))
    {
      return PL_FALSE;
    }
  strncpy (media->id, id->valuestring, PL_CHARSMAX (media->id));
  media->id[PL_CHARSMAX (media->id)] = '\0';

  cJSON *type = cJSON_GetObjectItemCaseSensitive (json, "type");
  if (!cJSON_IsString (type) || (type->valuestring == NULL))
    {
      return PL_FALSE;
    }
  strncpy (media->type, type->valuestring, PL_CHARSMAX (media->type));
  media->type[PL_CHARSMAX (media->type)] = '\0';

  cJSON *uploaded = cJSON_GetObjectItemCaseSensitive (json, "uploaded");
  if (!cJSON_IsString (uploaded) || (uploaded->valuestring == NULL))
    {
      return PL_FALSE;
    }
  strncpy (media->uploaded, uploaded->valuestring,
           PL_CHARSMAX (media->uploaded));
  media->uploaded[PL_CHARSMAX (media->uploaded)] = '\0';

  cJSON *taken = cJSON_GetObjectItemCaseSensitive (json, "taken");
  if (taken && cJSON_IsString (taken) && (taken->valuestring != NULL))
    {
      strncpy (media->taken, taken->valuestring, PL_CHARSMAX (media->taken));
      media->taken[PL_CHARSMAX (media->taken)] = '\0';
    }
  else
    {
      media->taken[0] = '\0';
    }

  cJSON *owner = cJSON_GetObjectItemCaseSensitive (json, "owner");
  if (!cJSON_IsBool (owner))
    {
      return PL_FALSE;
    }
  media->owner = cJSON_IsTrue (owner) ? 1 : 0;

  cJSON *favorite = cJSON_GetObjectItemCaseSensitive (json, "favorite");
  if (!cJSON_IsBool (favorite))
    {
      return PL_FALSE;
    }
  media->favorite = cJSON_IsTrue (favorite) ? 1 : 0;

  cJSON *favorites = cJSON_GetObjectItemCaseSensitive (json, "favorites");
  if (!cJSON_IsNumber (favorites))
    {
      return PL_FALSE;
    }
  media->favorites = (PLInt)favorites->valueint;

  return PL_TRUE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PLInt
plApiEventCreate (PLEvent *event, const PLChar *base, const PLChar *token,
                  const PLChar *name)
{
  if (!base || !token || !name || !event)
    {
      PL_ERROR ("Endpoint, token, name, and/or result are invalid");
      return PL_EARG;
    }

  CURL *curl = curl_easy_init ();
  if (!curl)
    {
      PL_ERROR ("Failed to initialize curl");
      return PL_EMEM;
    }
#ifdef _WIN32
  curl_easy_setopt (curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
#endif

  PLInt rc = PL_EOK;

  PLChar url[URL_MAX];
  snprintf (url, PL_CHARSMAX (url), "%s/event", base);
  url[PL_CHARSMAX (url)] = '\0';

  PLChar authorization[AUTHORIZATION_MAX];
  snprintf (authorization, PL_CHARSMAX (authorization),
            "Authorization: Bearer %s", token);
  authorization[PL_CHARSMAX (authorization)] = '\0';

  cJSON *body = cJSON_CreateObject ();
  if (!body)
    {
      PL_ERROR ("Out of memory");
      curl_easy_cleanup (curl);
      return PL_EMEM;
    }
  cJSON_AddStringToObject (body, "name", name);
  PLChar *bodystr = cJSON_PrintUnformatted (body);
  cJSON_Delete (body);
  if (!bodystr)
    {
      PL_ERROR ("Out of memory");
      curl_easy_cleanup (curl);
      return PL_EMEM;
    }

  struct curl_slist *headers = NULL;
  headers = curl_slist_append (headers, "Accept: application/json");
  headers = curl_slist_append (headers, "Content-Type: application/json");
  headers = curl_slist_append (headers, "User-Agent: p6a/" PL_VERSION_STRING);
  headers = curl_slist_append (headers, authorization);

  PLResponseBuffer response = { .data = NULL, .size = 0 };
  curl_easy_setopt (curl, CURLOPT_URL, url);
  curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt (curl, CURLOPT_POST, 1L);
  curl_easy_setopt (curl, CURLOPT_POSTFIELDS, bodystr);
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, plWriteCallback);
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, &response);

  PL_DEBUG ("--- Api Request ---");
  PL_DEBUG ("URL: %s", url);
  PL_DSLOW ("%s", authorization);
  PL_DSLOW ("%s", bodystr);
  plEnsureThrottle ();
  plThrottleAcquire (&throttle);
  CURLcode res = curl_easy_perform (curl);
  if (res != CURLE_OK)
    {
      PL_ERROR ("Request failed: %s", curl_easy_strerror (res));
      rc = PL_ENET;
      goto cleanup;
    }

  CURLlong httpcode;
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &httpcode);
  PL_DEBUG ("--- Api Response (HTTP %ld) ---", httpcode);
  PL_DSLOW ("%s", response.data ? response.data : "(empty)");
  if (httpcode != 201)
    {
      PL_ERROR ("Request failed with HTTP %ld", httpcode);
      rc = PL_ENET;
      goto cleanup;
    }

  if (!response.data)
    {
      PL_ERROR ("Empty response");
      rc = PL_ENET;
      goto cleanup;
    }

  cJSON *json = cJSON_Parse (response.data);
  if (!json)
    {
      PL_ERROR ("Failed to parse JSON response");
      rc = PL_ENET;
      goto cleanup;
    }

  cJSON *jid = cJSON_GetObjectItemCaseSensitive (json, "id");
  cJSON *jname = cJSON_GetObjectItemCaseSensitive (json, "name");
  cJSON *jcreated = cJSON_GetObjectItemCaseSensitive (json, "created");
  cJSON *jexpires = cJSON_GetObjectItemCaseSensitive (json, "expires");
  cJSON *jfrom = cJSON_GetObjectItemCaseSensitive (json, "from");
  cJSON *jto = cJSON_GetObjectItemCaseSensitive (json, "to");
  cJSON *jowner = cJSON_GetObjectItemCaseSensitive (json, "owner");
  cJSON *jfavorite = cJSON_GetObjectItemCaseSensitive (json, "favorite");
  cJSON *jpublic = cJSON_GetObjectItemCaseSensitive (json, "public");
  cJSON *jguests = cJSON_GetObjectItemCaseSensitive (json, "guests");
  cJSON *jmedia = cJSON_GetObjectItemCaseSensitive (json, "media");
  if (!cJSON_IsString (jid) || !jid->valuestring || !cJSON_IsString (jname)
      || !jname->valuestring || !cJSON_IsString (jcreated)
      || !jcreated->valuestring || !cJSON_IsString (jexpires)
      || !jexpires->valuestring || !cJSON_IsString (jfrom)
      || !jfrom->valuestring || !cJSON_IsString (jto) || !jto->valuestring
      || !cJSON_IsBool (jowner) || !cJSON_IsBool (jfavorite)
      || !cJSON_IsBool (jpublic) || !cJSON_IsNumber (jguests)
      || !cJSON_IsNumber (jmedia))
    {
      PL_ERROR ("Unexpected JSON structure in response");
      cJSON_Delete (json);
      rc = PL_ENET;
      goto cleanup;
    }

  strncpy (event->id, jid->valuestring, PL_CHARSMAX (event->id));
  event->id[PL_CHARSMAX (event->id)] = '\0';

  strncpy (event->name, jname->valuestring, PL_CHARSMAX (event->name));
  event->name[PL_CHARSMAX (event->name)] = '\0';

  strncpy (event->created, jcreated->valuestring,
           PL_CHARSMAX (event->created));
  event->created[PL_CHARSMAX (event->created)] = '\0';

  strncpy (event->expires, jexpires->valuestring,
           PL_CHARSMAX (event->expires));
  event->expires[PL_CHARSMAX (event->expires)] = '\0';

  strncpy (event->from, jfrom->valuestring, PL_CHARSMAX (event->from));
  event->from[PL_CHARSMAX (event->from)] = '\0';

  strncpy (event->to, jto->valuestring, PL_CHARSMAX (event->to));
  event->to[PL_CHARSMAX (event->to)] = '\0';

  event->owner = cJSON_IsTrue (jowner) ? 1 : 0;
  event->favorite = cJSON_IsTrue (jfavorite) ? 1 : 0;
  event->pub = cJSON_IsTrue (jpublic) ? 1 : 0;
  event->guests = jguests->valueint;
  event->media = jmedia->valueint;

  cJSON_Delete (json);

cleanup:
  curl_slist_free_all (headers);
  free (response.data);
  free (bodystr);
  curl_easy_cleanup (curl);

  return rc;
}

PLEventList *
plApiEventList (const PLChar *base, const PLChar *token, const PLChar *query)
{
  if (!base || !token)
    {
      PL_ERROR ("Endpoint and/or token are invalid");
      return NULL;
    }

  CURL *curl = curl_easy_init ();
  if (!curl)
    {
      PL_ERROR ("Failed to initialize curl");
      return NULL;
    }
#ifdef _WIN32
  curl_easy_setopt (curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
#endif

  PLEventList *result = NULL;

  PLChar url[URL_MAX];
  if (query != NULL)
    {
      PLChar *escaped = curl_easy_escape (curl, query, 0);
      snprintf (url, PL_CHARSMAX (url), "%s/event?q=%s", base,
                escaped ? escaped : "");
      if (escaped)
        {
          curl_free (escaped);
        }
    }
  else
    {
      snprintf (url, PL_CHARSMAX (url), "%s/event", base);
    }
  url[PL_CHARSMAX (url)] = '\0';

  PLChar authorization[AUTHORIZATION_MAX];
  snprintf (authorization, PL_CHARSMAX (authorization),
            "Authorization: Bearer %s", token);
  authorization[PL_CHARSMAX (authorization)] = '\0';

  struct curl_slist *headers = NULL;
  headers = curl_slist_append (headers, "Accept: application/json");
  headers = curl_slist_append (headers, "User-Agent: p6a/" PL_VERSION_STRING);
  headers = curl_slist_append (headers, authorization);

  PLResponseBuffer response = { .data = NULL, .size = 0 };
  curl_easy_setopt (curl, CURLOPT_URL, url);
  curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, plWriteCallback);
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, &response);

  PL_DEBUG ("--- Api Request ---");
  PL_DEBUG ("URL: %s", url);
  PL_DSLOW ("%s", authorization);
  plEnsureThrottle ();
  plThrottleAcquire (&throttle);
  CURLcode res = curl_easy_perform (curl);
  if (res != CURLE_OK)
    {
      PL_ERROR ("Request failed: %s", curl_easy_strerror (res));
      goto cleanup_headers;
    }

  CURLlong httpcode;
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &httpcode);
  PL_DEBUG ("--- Api Response (HTTP %ld) ---", httpcode);
  PL_DSLOW ("%s", response.data ? response.data : "(empty)");
  if (httpcode != 200)
    {
      PL_ERROR ("Request failed with HTTP %ld", httpcode);
      goto cleanup_headers;
    }

  if (!response.data)
    {
      PL_ERROR ("Empty response");
      goto cleanup_headers;
    }

  cJSON *json = cJSON_Parse (response.data);
  if (!json)
    {
      PL_ERROR ("Failed to parse JSON response");
      goto cleanup_headers;
    }

  if (!cJSON_IsObject (json))
    {
      PL_ERROR ("Expected JSON object in response");
      goto cleanup_json;
    }

  cJSON *items = cJSON_GetObjectItemCaseSensitive (json, "items");
  if (!items || !cJSON_IsArray (items))
    {
      PL_ERROR ("Expected 'items' array in response");
      goto cleanup_json;
    }

  PLSize count = cJSON_GetArraySize (items);
  result = malloc (sizeof (PLEventList));
  if (!result)
    {
      PL_ERROR ("Out of memory");
      goto cleanup_json;
    }

  result->count = count;
  result->events = malloc (sizeof (PLEvent) * count);
  if (!result->events && count > 0)
    {
      PL_ERROR ("Out of memory");
      free (result);
      result = NULL;
      goto cleanup_json;
    }

  PLSize i = 0;
  cJSON *item = NULL;
  cJSON_ArrayForEach (item, items)
  {
    PLEvent *e = &result->events[i];
    if (!plParseEvent (item, e))
      {
        PL_ERROR ("Failed to parse event at index %lu", i);
        free (result->events);
        free (result);
        result = NULL;
        goto cleanup_json;
      }
    ++i;
  }

cleanup_json:
  cJSON_Delete (json);
cleanup_headers:
  curl_slist_free_all (headers);
  free (response.data);
  curl_easy_cleanup (curl);

  return result;
}

void
plFreeEventList (PLEventList *list)
{
  if (list)
    {
      free (list->events);
      free (list);
    }
}

PLMediaList *
plApiMediaList (const PLChar *base, const PLChar *token, const PLChar *event)
{
  if (!base || !token || !event)
    {
      PL_ERROR ("Endpoint, token, and/or event are invalid");
      return NULL;
    }

  CURL *curl = curl_easy_init ();
  if (!curl)
    {
      PL_ERROR ("Failed to initialize curl");
      return NULL;
    }
#ifdef _WIN32
  curl_easy_setopt (curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
#endif

  PLMediaList *result = NULL;

  PLChar url[URL_MAX];
  snprintf (url, PL_CHARSMAX (url), "%s/event/%s/media", base, event);
  url[PL_CHARSMAX (url)] = '\0';

  PLChar authorization[AUTHORIZATION_MAX];
  snprintf (authorization, PL_CHARSMAX (authorization),
            "Authorization: Bearer %s", token);
  authorization[PL_CHARSMAX (authorization)] = '\0';

  struct curl_slist *headers = NULL;
  headers = curl_slist_append (headers, "Accept: application/json");
  headers = curl_slist_append (headers, "User-Agent: p6a/" PL_VERSION_STRING);
  headers = curl_slist_append (headers, authorization);

  PLResponseBuffer response = { .data = NULL, .size = 0 };
  curl_easy_setopt (curl, CURLOPT_URL, url);
  curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, plWriteCallback);
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, &response);

  PL_DEBUG ("--- Api Request ---");
  PL_DEBUG ("URL: %s", url);
  PL_DSLOW ("%s", authorization);
  plEnsureThrottle ();
  plThrottleAcquire (&throttle);
  CURLcode res = curl_easy_perform (curl);
  if (res != CURLE_OK)
    {
      PL_ERROR ("Request failed: %s", curl_easy_strerror (res));
      goto cleanup_response;
    }

  CURLlong httpcode;
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &httpcode);
  PL_DEBUG ("--- Api Response (HTTP %ld) ---", httpcode);
  PL_DSLOW ("%s", response.data ? response.data : "(empty)");
  if (httpcode != 200)
    {
      PL_ERROR ("Request failed with HTTP %ld", httpcode);
      goto cleanup_response;
    }

  if (!response.data)
    {
      PL_ERROR ("Empty response");
      goto cleanup_response;
    }

  cJSON *json = cJSON_Parse (response.data);
  if (!json)
    {
      PL_ERROR ("Failed to parse JSON response");
      goto cleanup_response;
    }

  if (!cJSON_IsObject (json))
    {
      PL_ERROR ("Expected JSON object in response");
      goto cleanup_json;
    }

  cJSON *items = cJSON_GetObjectItemCaseSensitive (json, "items");
  if (!items || !cJSON_IsArray (items))
    {
      PL_ERROR ("Expected 'items' array in response");
      goto cleanup_json;
    }

  PLSize count = cJSON_GetArraySize (items);
  result = malloc (sizeof (PLMediaList));
  if (!result)
    {
      PL_ERROR ("Out of memory");
      goto cleanup_json;
    }

  result->count = count;
  result->media = malloc (sizeof (PLMedia) * count);
  if (!result->media && count > 0)
    {
      PL_ERROR ("Out of memory");
      free (result);
      result = NULL;
      goto cleanup_json;
    }

  PLSize i = 0;
  cJSON *item = NULL;
  cJSON_ArrayForEach (item, items)
  {
    PLMedia *m = &result->media[i];
    if (!plParseMedia (item, m))
      {
        PL_ERROR ("Failed to parse media at index %lu", i);
        free (result->media);
        free (result);
        result = NULL;
        goto cleanup_json;
      }
    ++i;
  }

cleanup_json:
  cJSON_Delete (json);
cleanup_response:
  curl_slist_free_all (headers);
  free (response.data);
  curl_easy_cleanup (curl);

  return result;
}

void
plFreeMediaList (PLMediaList *list)
{
  if (list)
    {
      free (list->media);
      free (list);
    }
}

PLInt
plApiMediaFetch (const PLChar *base, const PLChar *token, const PLChar *event,
                 const PLChar *media, const PLChar *path)
{
  if (!base || !token || !event || !media || !path)
    {
      PL_ERROR ("Endpoint, token, event, media, and/or path are invalid");
      return PL_EARG;
    }

  CURL *curl = curl_easy_init ();
  if (!curl)
    {
      PL_ERROR ("Failed to initialize curl");
      return PL_EMEM;
    }
#ifdef _WIN32
  curl_easy_setopt (curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
#endif

  PLFile *file = plFileOpen (path, "wb");
  if (!file)
    {
      PL_ERROR ("Failed to open file for writing: %s", path);
      curl_easy_cleanup (curl);
      return PL_EFS;
    }

  PLInt result = PL_EOK;

  PLChar url[URL_MAX];
  snprintf (url, PL_CHARSMAX (url), "%s/event/%s/media/%s", base, event,
            media);
  url[PL_CHARSMAX (url)] = '\0';

  PLChar authorization[AUTHORIZATION_MAX];
  snprintf (authorization, PL_CHARSMAX (authorization),
            "Authorization: Bearer %s", token);
  authorization[PL_CHARSMAX (authorization)] = '\0';

  struct curl_slist *headers = NULL;
  headers = curl_slist_append (headers, "User-Agent: p6a/" PL_VERSION_STRING);
  headers = curl_slist_append (headers, authorization);

  curl_easy_setopt (curl, CURLOPT_URL, url);
  curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, plWriteFileCallback);
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, file);

  PL_DEBUG ("--- Api Request ---");
  PL_DEBUG ("URL: %s", url);
  PL_DSLOW ("%s", authorization);
  plEnsureThrottle ();
  plThrottleAcquire (&throttle);
  CURLcode res = curl_easy_perform (curl);
  if (res != CURLE_OK)
    {
      PL_ERROR ("Request failed: %s", curl_easy_strerror (res));
      result = PL_ENET;
      goto cleanup;
    }

  CURLlong httpcode;
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &httpcode);
  PL_DEBUG ("--- Api Response (HTTP %ld) ---", httpcode);
  if (httpcode != 200)
    {
      PL_ERROR ("Request failed with HTTP %ld", httpcode);
      result = PL_ENET;
      goto cleanup;
    }

cleanup:
  curl_slist_free_all (headers);
  plFileClose (file);
  curl_easy_cleanup (curl);
  if (result != PL_EOK)
    {
      remove (path);
    }

  return result;
}

PLInt
plApiQrFetch (const PLChar *base, const PLChar *token, const PLChar *event,
              const PLChar *path, PLBool svg)
{
  if (!base || !token || !event || !path)
    {
      PL_ERROR ("Endpoint, token, event, and/or path are invalid");
      return PL_EARG;
    }

  CURL *curl = curl_easy_init ();
  if (!curl)
    {
      PL_ERROR ("Failed to initialize curl");
      return PL_EMEM;
    }
#ifdef _WIN32
  curl_easy_setopt (curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
#endif

  PLFile *file = plFileOpen (path, "wb");
  if (!file)
    {
      PL_ERROR ("Failed to open file for writing: %s", path);
      curl_easy_cleanup (curl);
      return PL_EFS;
    }

  PLInt result = PL_EOK;

  PLChar url[URL_MAX];
  if (svg)
    snprintf (url, PL_CHARSMAX (url), "%s/event/%s/qr?format=svg", base,
              event);
  else
    snprintf (url, PL_CHARSMAX (url), "%s/event/%s/qr", base, event);
  url[PL_CHARSMAX (url)] = '\0';

  PLChar authorization[AUTHORIZATION_MAX];
  snprintf (authorization, PL_CHARSMAX (authorization),
            "Authorization: Bearer %s", token);
  authorization[PL_CHARSMAX (authorization)] = '\0';

  struct curl_slist *headers = NULL;
  headers = curl_slist_append (headers, "User-Agent: p6a/" PL_VERSION_STRING);
  headers = curl_slist_append (headers, authorization);

  curl_easy_setopt (curl, CURLOPT_URL, url);
  curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, plWriteFileCallback);
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, file);

  PL_DEBUG ("--- Api Request ---");
  PL_DEBUG ("URL: %s", url);
  PL_DSLOW ("%s", authorization);
  plEnsureThrottle ();
  plThrottleAcquire (&throttle);
  CURLcode res = curl_easy_perform (curl);
  if (res != CURLE_OK)
    {
      PL_ERROR ("Request failed: %s", curl_easy_strerror (res));
      result = PL_ENET;
      goto cleanup;
    }

  CURLlong httpcode;
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &httpcode);
  PL_DEBUG ("--- Api Response (HTTP %ld) ---", httpcode);
  if (httpcode != 200)
    {
      PL_ERROR ("Request failed with HTTP %ld", httpcode);
      result = PL_ENET;
      goto cleanup;
    }

cleanup:
  curl_slist_free_all (headers);
  plFileClose (file);
  curl_easy_cleanup (curl);
  if (result != PL_EOK)
    {
      remove (path);
    }

  return result;
}

PLInt
plApiEventUpdate (const PLChar *base, const PLChar *token, const PLChar *id,
                  const PLChar *name, const PLChar *from, const PLChar *to,
                  PLInt pub, PLInt fav)
{
  if (!base || !token || !id)
    {
      PL_ERROR ("Endpoint, token, and/or id are invalid");
      return PL_EARG;
    }

  CURL *curl = curl_easy_init ();
  if (!curl)
    {
      PL_ERROR ("Failed to initialize curl");
      return PL_EMEM;
    }
#ifdef _WIN32
  curl_easy_setopt (curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
#endif

  PLInt rc = PL_EOK;

  PLChar url[URL_MAX];
  snprintf (url, PL_CHARSMAX (url), "%s/event/%s", base, id);
  url[PL_CHARSMAX (url)] = '\0';

  PLChar authorization[AUTHORIZATION_MAX];
  snprintf (authorization, PL_CHARSMAX (authorization),
            "Authorization: Bearer %s", token);
  authorization[PL_CHARSMAX (authorization)] = '\0';

  cJSON *body = cJSON_CreateObject ();
  if (!body)
    {
      PL_ERROR ("Out of memory");
      curl_easy_cleanup (curl);
      return PL_EMEM;
    }
  if (name != NULL)
    {
      cJSON_AddStringToObject (body, "name", name);
    }
  if (from != NULL)
    {
      cJSON_AddStringToObject (body, "from", from);
    }
  if (to != NULL)
    {
      cJSON_AddStringToObject (body, "to", to);
    }
  if (pub != -1)
    {
      cJSON_AddBoolToObject (body, "public", pub ? 1 : 0);
    }
  if (fav != -1)
    {
      cJSON_AddBoolToObject (body, "favorite", fav ? 1 : 0);
    }
  PLChar *bodystr = cJSON_PrintUnformatted (body);
  cJSON_Delete (body);
  if (!bodystr)
    {
      PL_ERROR ("Out of memory");
      curl_easy_cleanup (curl);
      return PL_EMEM;
    }

  struct curl_slist *headers = NULL;
  headers = curl_slist_append (headers, "Accept: application/json");
  headers = curl_slist_append (headers, "Content-Type: application/json");
  headers = curl_slist_append (headers, "User-Agent: p6a/" PL_VERSION_STRING);
  headers = curl_slist_append (headers, authorization);

  curl_easy_setopt (curl, CURLOPT_URL, url);
  curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt (curl, CURLOPT_CUSTOMREQUEST, "PATCH");
  curl_easy_setopt (curl, CURLOPT_POSTFIELDS, bodystr);
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, plDiscardCallback);

  PL_DEBUG ("--- Api Request ---");
  PL_DEBUG ("URL: %s", url);
  PL_DSLOW ("%s", authorization);
  PL_DSLOW ("%s", bodystr);
  plEnsureThrottle ();
  plThrottleAcquire (&throttle);
  CURLcode res = curl_easy_perform (curl);
  if (res != CURLE_OK)
    {
      PL_ERROR ("Request failed: %s", curl_easy_strerror (res));
      rc = PL_ENET;
      goto cleanup;
    }

  CURLlong httpcode;
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &httpcode);
  PL_DEBUG ("--- Api Response (HTTP %ld) ---", httpcode);
  if (httpcode != 200)
    {
      PL_ERROR ("Request failed with HTTP %ld", httpcode);
      rc = PL_ENET;
      goto cleanup;
    }

cleanup:
  curl_slist_free_all (headers);
  free (bodystr);
  curl_easy_cleanup (curl);

  return rc;
}

PLInt
plApiLinkFetch (PLChar *link, PLSize size, const PLChar *base,
                const PLChar *token, const PLChar *event)
{
  if (!base || !token || !event || !link || size == 0)
    {
      PL_ERROR ("Endpoint, token, event, and/or link buffer are invalid");
      return PL_EARG;
    }

  CURL *curl = curl_easy_init ();
  if (!curl)
    {
      PL_ERROR ("Failed to initialize curl");
      return PL_EMEM;
    }
#ifdef _WIN32
  curl_easy_setopt (curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
#endif

  PLInt result = PL_EOK;

  PLChar url[URL_MAX];
  snprintf (url, PL_CHARSMAX (url), "%s/event/%s/link", base, event);
  url[PL_CHARSMAX (url)] = '\0';

  PLChar authorization[AUTHORIZATION_MAX];
  snprintf (authorization, PL_CHARSMAX (authorization),
            "Authorization: Bearer %s", token);
  authorization[PL_CHARSMAX (authorization)] = '\0';

  struct curl_slist *headers = NULL;
  headers = curl_slist_append (headers, "Accept: application/json");
  headers = curl_slist_append (headers, "User-Agent: p6a/" PL_VERSION_STRING);
  headers = curl_slist_append (headers, authorization);

  PLResponseBuffer response = { NULL, 0 };

  curl_easy_setopt (curl, CURLOPT_URL, url);
  curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, plWriteCallback);
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, &response);

  PL_DEBUG ("--- Api Request ---");
  PL_DEBUG ("URL: %s", url);
  PL_DSLOW ("%s", authorization);
  plEnsureThrottle ();
  plThrottleAcquire (&throttle);
  CURLcode res = curl_easy_perform (curl);
  if (res != CURLE_OK)
    {
      PL_ERROR ("Request failed: %s", curl_easy_strerror (res));
      result = PL_ENET;
      goto cleanup;
    }

  CURLlong httpcode;
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &httpcode);
  PL_DEBUG ("--- Api Response (HTTP %ld) ---", httpcode);
  if (httpcode != 200)
    {
      PL_ERROR ("Request failed with HTTP %ld", httpcode);
      result = PL_ENET;
      goto cleanup;
    }

  PL_DSLOW ("%s", response.data ? response.data : "(empty)");

  {
    cJSON *json = cJSON_Parse (response.data);
    if (!json)
      {
        PL_ERROR ("Failed to parse response JSON");
        result = PL_ENET;
        goto cleanup;
      }

    cJSON *jlink = cJSON_GetObjectItemCaseSensitive (json, "link");
    if (!cJSON_IsString (jlink) || jlink->valuestring == NULL)
      {
        PL_ERROR ("Missing or invalid 'link' field in response");
        cJSON_Delete (json);
        result = PL_ENET;
        goto cleanup;
      }

    strncpy (link, jlink->valuestring, size - 1);
    link[size - 1] = '\0';
    cJSON_Delete (json);
  }

cleanup:
  curl_slist_free_all (headers);
  free (response.data);
  curl_easy_cleanup (curl);

  return result;
}
