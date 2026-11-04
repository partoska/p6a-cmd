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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define PATH_MAX_API (1536)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Formats an API path, refusing to truncate it. */
static PKStatus pkPath (PKClient *client, PKChar *path, PKSize size,
                        const PKChar *fmt, ...) PK_PRINTF (4, 5);

static PKStatus
pkPath (PKClient *client, PKChar *path, PKSize size, const PKChar *fmt, ...)
{
  PKVaList args;
  PK_VA_START (args, fmt);
  PKInt length = pkStrFmtV (path, size, fmt, args);
  PK_VA_END (args);
  if (length < 0 || (PKSize)length >= size)
    {
      return pkFail (client, PK_EARG, 0, "Request path too long");
    }

  return PK_OK;
}

/* Appends "&name=value" (or "?name=value" first) with value
 * percent-encoded (RFC 3986), refusing to truncate. */
static PKStatus
pkQuery (PKClient *client, PKChar *path, PKSize size, const PKChar *name,
         const PKChar *value)
{
  static const PKChar hex[] = "0123456789ABCDEF";
  PKSize n = pkStrLen (path);
  PKInt length = pkStrFmt (path + n, size - n,
                           "%c%s=", pkStrFind (path, '?') ? '&' : '?', name);
  if (length < 0 || (PKSize)length >= size - n)
    {
      return pkFail (client, PK_EARG, 0, "Request path too long");
    }

  n += (PKSize)length;
  for (const PKChar *c = value; *c; ++c)
    {
      PKByte b = (PKByte)*c;
      PKBool plain = (b >= 'A' && b <= 'Z') || (b >= 'a' && b <= 'z')
                     || (b >= '0' && b <= '9') || b == '-' || b == '.'
                     || b == '_' || b == '~';
      if (n + (plain ? 1 : 3) >= size)
        {
          return pkFail (client, PK_EARG, 0, "Request path too long");
        }
      if (plain)
        {
          path[n++] = (PKChar)b;
        }
      else
        {
          path[n++] = '%';
          path[n++] = hex[b >> 4];
          path[n++] = hex[b & 0x0F];
        }
    }
  path[n] = '\0';

  return PK_OK;
}

/*
 * Calls a JSON endpoint of the API: sends body (when not NULL), requires
 * the expected status and parses the answer into *json (when not NULL).
 */
static PKStatus
pkCall (PKClient *client, const PKChar *method, const PKChar *path,
        const cJSON *body, PKLong expected, cJSON **json)
{
  PKChar *text = NULL;
  if (body && !(text = cJSON_PrintUnformatted (body)))
    {
      return pkFailMem (client);
    }

  PKRequest request = { 0 };
  request.method = method;
  request.url = path;
  request.api = PK_TRUE;
  if (text)
    {
      request.type = "application/json";
      request.body = text;
      request.length = pkStrLen (text);
    }

  PKResponse response;
  PKStatus status = pkRequest (client, &request, &response);
  cJSON_free (text);
  if (status == PK_OK)
    {
      status = pkExpect (client, &response, expected);
    }
  if (status == PK_OK && json)
    {
      status = pkResponseJson (client, &response, json);
    }
  pkResponseFree (&response);

  return status;
}

/* Downloads from the API into a file, removed again on failure. */
static PKStatus
pkFetch (PKClient *client, const PKChar *path, const PKChar *file)
{
  if (!file)
    {
      return pkFail (client, PK_EARG, 0, "No destination file");
    }

  PKFile *stream = pkFileOpen (file, PK_TRUE);
  if (!stream)
    {
      return pkFail (client, PK_EFS, pkSysError (), "Cannot write %s: %s",
                     file, pkSysErrorText (pkSysError ()));
    }

  PKRequest request = { 0 };
  request.url = path;
  request.api = PK_TRUE;
  request.file = stream;

  PKResponse response;
  PKStatus status = pkRequest (client, &request, &response);
  pkFileClose (stream);
  if (status == PK_OK)
    {
      status = pkExpect (client, &response, 200);
    }
  pkResponseFree (&response);
  if (status != PK_OK)
    {
      pkFileRemove (file);
    }

  return status;
}

/* Copies a string member of a JSON object, or fails. */
static PKStatus
pkJsonCopy (PKClient *client, const cJSON *json, const PKChar *name,
            PKChar *out, PKSize size)
{
  const PKChar *value = pkJsonString (json, name);
  if (!value || !size)
    {
      return pkFail (client, PK_EDATA, 0, "Response without '%s'", name);
    }

  pkStrFmt (out, size, "%s", value);

  return PK_OK;
}

static PKBool
pkParseEvent (cJSON *json, PKEvent *event)
{
  cJSON *id = cJSON_GetObjectItemCaseSensitive (json, "id");
  if (!cJSON_IsString (id) || (id->valuestring == NULL))
    {
      return PK_FALSE;
    }
  pkStrFmt (event->id, sizeof (event->id), "%s", id->valuestring);

  cJSON *name = cJSON_GetObjectItemCaseSensitive (json, "name");
  if (!cJSON_IsString (name) || (name->valuestring == NULL))
    {
      return PK_FALSE;
    }
  pkStrFmt (event->name, sizeof (event->name), "%s", name->valuestring);

  cJSON *created = cJSON_GetObjectItemCaseSensitive (json, "created");
  if (created && cJSON_IsString (created) && (created->valuestring != NULL))
    {
      pkStrFmt (event->created, sizeof (event->created), "%s",
                created->valuestring);
    }
  else
    {
      event->created[0] = '\0';
    }

  cJSON *expires = cJSON_GetObjectItemCaseSensitive (json, "expires");
  if (expires && cJSON_IsString (expires) && (expires->valuestring != NULL))
    {
      pkStrFmt (event->expires, sizeof (event->expires), "%s",
                expires->valuestring);
    }
  else
    {
      event->expires[0] = '\0';
    }

  cJSON *from = cJSON_GetObjectItemCaseSensitive (json, "from");
  if (!cJSON_IsString (from) || (from->valuestring == NULL))
    {
      return PK_FALSE;
    }
  pkStrFmt (event->from, sizeof (event->from), "%s", from->valuestring);

  cJSON *to = cJSON_GetObjectItemCaseSensitive (json, "to");
  if (!cJSON_IsString (to) || (to->valuestring == NULL))
    {
      return PK_FALSE;
    }
  pkStrFmt (event->to, sizeof (event->to), "%s", to->valuestring);

  cJSON *owner = cJSON_GetObjectItemCaseSensitive (json, "owner");
  if (!cJSON_IsBool (owner))
    {
      return PK_FALSE;
    }
  event->owner = cJSON_IsTrue (owner) ? 1 : 0;

  // Newer servers only; contributor access is unknown on older ones.
  cJSON *contributor
      = cJSON_GetObjectItemCaseSensitive (json, "isContributor");
  event->contributor = cJSON_IsTrue (contributor) ? 1 : 0;

  cJSON *guests = cJSON_GetObjectItemCaseSensitive (json, "guests");
  if (!cJSON_IsNumber (guests))
    {
      return PK_FALSE;
    }
  event->guests = guests->valueint;

  cJSON *media = cJSON_GetObjectItemCaseSensitive (json, "media");
  if (!cJSON_IsNumber (media))
    {
      return PK_FALSE;
    }
  event->media = media->valueint;

  cJSON *favorite = cJSON_GetObjectItemCaseSensitive (json, "favorite");
  if (!cJSON_IsBool (favorite))
    {
      return PK_FALSE;
    }
  event->favorite = cJSON_IsTrue (favorite) ? 1 : 0;

  cJSON *pub = cJSON_GetObjectItemCaseSensitive (json, "public");
  if (!cJSON_IsBool (pub))
    {
      return PK_FALSE;
    }
  event->pub = cJSON_IsTrue (pub) ? 1 : 0;

  cJSON *moderated = cJSON_GetObjectItemCaseSensitive (json, "moderated");
  if (!cJSON_IsBool (moderated))
    {
      return PK_FALSE;
    }
  event->moderated = cJSON_IsTrue (moderated) ? 1 : 0;

  return PK_TRUE;
}

static PKBool
pkParseMedia (cJSON *json, PKMedia *media)
{
  cJSON *id = cJSON_GetObjectItemCaseSensitive (json, "id");
  if (!cJSON_IsString (id) || (id->valuestring == NULL))
    {
      return PK_FALSE;
    }
  pkStrFmt (media->id, sizeof (media->id), "%s", id->valuestring);

  cJSON *type = cJSON_GetObjectItemCaseSensitive (json, "type");
  if (!cJSON_IsString (type) || (type->valuestring == NULL))
    {
      return PK_FALSE;
    }
  pkStrFmt (media->type, sizeof (media->type), "%s", type->valuestring);

  cJSON *uploaded = cJSON_GetObjectItemCaseSensitive (json, "uploaded");
  if (!cJSON_IsString (uploaded) || (uploaded->valuestring == NULL))
    {
      return PK_FALSE;
    }
  pkStrFmt (media->uploaded, sizeof (media->uploaded), "%s",
            uploaded->valuestring);

  cJSON *taken = cJSON_GetObjectItemCaseSensitive (json, "taken");
  if (taken && cJSON_IsString (taken) && (taken->valuestring != NULL))
    {
      pkStrFmt (media->taken, sizeof (media->taken), "%s", taken->valuestring);
    }
  else
    {
      media->taken[0] = '\0';
    }

  cJSON *owner = cJSON_GetObjectItemCaseSensitive (json, "owner");
  if (!cJSON_IsBool (owner))
    {
      return PK_FALSE;
    }
  media->owner = cJSON_IsTrue (owner) ? 1 : 0;

  cJSON *favorite = cJSON_GetObjectItemCaseSensitive (json, "favorite");
  if (!cJSON_IsBool (favorite))
    {
      return PK_FALSE;
    }
  media->favorite = cJSON_IsTrue (favorite) ? 1 : 0;

  cJSON *favorites = cJSON_GetObjectItemCaseSensitive (json, "favorites");
  if (!cJSON_IsNumber (favorites))
    {
      return PK_FALSE;
    }
  media->favorites = (PKInt)favorites->valueint;

  cJSON *approved = cJSON_GetObjectItemCaseSensitive (json, "approved");
  if (!cJSON_IsBool (approved))
    {
      return PK_FALSE;
    }
  media->approved = cJSON_IsTrue (approved) ? 1 : 0;

  return PK_TRUE;
}

/* Copies an array of strings, a member of a JSON object, into a list. */
static PKStatus
pkJsonStrings (PKClient *client, const cJSON *json, const PKChar *name,
               PKStringList *list)
{
  *list = (PKStringList){ 0 };

  const cJSON *items = cJSON_GetObjectItemCaseSensitive (json, name);
  if (!cJSON_IsArray (items))
    {
      return pkFail (client, PK_EDATA, 0, "Response without '%s'", name);
    }

  PKSize count = (PKSize)cJSON_GetArraySize (items);
  list->items = pkMemZallocArray (count ? count : 1, sizeof (PKChar *));
  if (!list->items)
    {
      return pkFailMem (client);
    }

  const cJSON *item = NULL;
  cJSON_ArrayForEach (item, items)
  {
    if (!cJSON_IsString (item) || !item->valuestring)
      {
        pkStringListFree (list);
        return pkFail (client, PK_EDATA, 0, "Malformed '%s'", name);
      }
    PKChar *copy = pkStrDup (item->valuestring, pkStrLen (item->valuestring));
    if (!copy)
      {
        pkStringListFree (list);
        return pkFailMem (client);
      }
    list->items[list->count++] = copy;
  }

  return PK_OK;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKStatus
pkEventList (PKClient *client, const PKChar *query, PKEventList *list)
{
  *list = (PKEventList){ 0 };

  PKChar path[PATH_MAX_API] = "/event";
  PKStatus status
      = query ? pkQuery (client, path, sizeof (path), "q", query) : PK_OK;
  cJSON *json = NULL;
  if (status == PK_OK)
    {
      status = pkCall (client, "GET", path, NULL, 200, &json);
    }
  if (status != PK_OK)
    {
      return status;
    }

  const cJSON *items = cJSON_GetObjectItemCaseSensitive (json, "items");
  if (!cJSON_IsArray (items))
    {
      cJSON_Delete (json);
      return pkFail (client, PK_EDATA, 0, "Response without 'items'");
    }

  PKSize count = (PKSize)cJSON_GetArraySize (items);
  list->events = pkMemZallocArray (count, sizeof (PKEvent));
  if (!list->events)
    {
      cJSON_Delete (json);
      return pkFailMem (client);
    }

  const cJSON *item = NULL;
  cJSON_ArrayForEach (item, items)
  {
    if (!pkParseEvent ((cJSON *)item, &list->events[list->count]))
      {
        status = pkFail (client, PK_EDATA, 0, "Malformed event at index %lu",
                         (unsigned long)list->count);
        break;
      }
    ++list->count;
  }

  cJSON_Delete (json);
  if (status != PK_OK)
    {
      pkEventListFree (list);
    }

  return status;
}

PKVoid
pkEventListFree (PKEventList *list)
{
  pkMemFree (list->events);
  list->events = NULL;
  list->count = 0;
}

PKStatus
pkEventCreate (PKClient *client, const PKChar *name, PKEvent *event)
{
  if (!name || !event)
    {
      return pkFail (client, PK_EARG, 0, "No event name");
    }

  cJSON *body = cJSON_CreateObject ();
  if (!body || !cJSON_AddStringToObject (body, "name", name))
    {
      cJSON_Delete (body);
      return pkFailMem (client);
    }

  cJSON *json = NULL;
  PKStatus status = pkCall (client, "POST", "/event", body, 201, &json);
  cJSON_Delete (body);
  if (status == PK_OK && !pkParseEvent (json, event))
    {
      status = pkFail (client, PK_EDATA, 0, "Malformed event");
    }
  cJSON_Delete (json);

  return status;
}

PKStatus
pkEventUpdate (PKClient *client, const PKChar *event, const PKChar *name,
               const PKChar *from, const PKChar *to, PKInt pub, PKInt favorite,
               PKInt moderated)
{
  PKChar path[PATH_MAX_API];
  PKStatus status
      = event ? pkPath (client, path, sizeof (path), "/event/%s", event)
              : pkFail (client, PK_EARG, 0, "No event");
  if (status != PK_OK)
    {
      return status;
    }

  cJSON *body = cJSON_CreateObject ();
  PKBool ok = body != NULL;
  ok = ok && (!name || cJSON_AddStringToObject (body, "name", name));
  ok = ok && (!from || cJSON_AddStringToObject (body, "from", from));
  ok = ok && (!to || cJSON_AddStringToObject (body, "to", to));
  ok = ok && (pub == -1 || cJSON_AddBoolToObject (body, "public", pub));
  ok = ok
       && (favorite == -1
           || cJSON_AddBoolToObject (body, "favorite", favorite));
  ok = ok
       && (moderated == -1
           || cJSON_AddBoolToObject (body, "moderated", moderated));
  if (!ok)
    {
      cJSON_Delete (body);
      return pkFailMem (client);
    }

  status = pkCall (client, "PATCH", path, body, 200, NULL);
  cJSON_Delete (body);

  return status;
}

PKStatus
pkEventJoin (PKClient *client, const PKChar *invite, PKEvent *event,
             PKChar *url, PKSize size)
{
  if (!invite || !event)
    {
      return pkFail (client, PK_EARG, 0, "No invite");
    }
  *event = (PKEvent){ 0 };

  cJSON *body = cJSON_CreateObject ();
  if (!body || !cJSON_AddStringToObject (body, "invite", invite))
    {
      cJSON_Delete (body);
      return pkFailMem (client);
    }

  cJSON *json = NULL;
  PKStatus status = pkCall (client, "POST", "/event/join", body, 200, &json);
  cJSON_Delete (body);
  if (status == PK_EHTTP)
    {
      PKLong code = pkError (client)->code;
      if (code == 400)
        {
          status
              = pkFail (client, PK_EHTTP, code, "Not an invite code or link");
        }
      else if (code == 404)
        {
          status = pkFail (client, PK_EHTTP, code,
                           "Invite not found or revoked, or the event is "
                           "inactive");
        }
      else if (code == 403)
        {
          status = pkFail (client, PK_EHTTP, code,
                           "Not allowed to join the event");
        }
    }
  if (status == PK_OK && !pkParseEvent (json, event))
    {
      status = pkFail (client, PK_EDATA, 0, "Malformed event");
    }
  if (status == PK_OK && url)
    {
      status = pkJsonCopy (client, json, "url", url, size);
    }
  cJSON_Delete (json);

  return status;
}

PKStatus
pkEventTrusted (PKClient *client, const PKChar *event, PKStringList *list)
{
  *list = (PKStringList){ 0 };

  PKChar path[PATH_MAX_API];
  PKStatus status = event ? pkPath (client, path, sizeof (path),
                                    "/event/%s/trusted", event)
                          : pkFail (client, PK_EARG, 0, "No event");
  cJSON *json = NULL;
  if (status == PK_OK)
    {
      status = pkCall (client, "GET", path, NULL, 200, &json);
    }
  // The list envelope always holds the whole list (hasMore is false).
  if (status == PK_OK)
    {
      status = pkJsonStrings (client, json, "items", list);
    }
  cJSON_Delete (json);

  return status;
}

/* Changes the trusted list: POST (add) or DELETE (remove) on
 * /event/{id}/trusted with {"guests":[…]}; the answer is the resulting
 * list. */
static PKStatus
pkTrustedWrite (PKClient *client, const PKChar *method, const PKChar *event,
                const PKChar *const *entries, PKSize count, PKStringList *list)
{
  PKStringList dummy;
  list = list ? list : &dummy;
  *list = (PKStringList){ 0 };

  PKChar path[PATH_MAX_API];
  PKStatus status
      = event && (entries || !count)
            ? pkPath (client, path, sizeof (path), "/event/%s/trusted", event)
            : pkFail (client, PK_EARG, 0, "No event or entries");
  if (status != PK_OK)
    {
      return status;
    }

  cJSON *body = cJSON_CreateObject ();
  cJSON *array = body ? cJSON_AddArrayToObject (body, "guests") : NULL;
  PKBool ok = array != NULL;
  for (PKSize i = 0; ok && i < count; ++i)
    {
      cJSON *entry = entries[i] ? cJSON_CreateString (entries[i]) : NULL;
      ok = entry && cJSON_AddItemToArray (array, entry);
      if (!ok)
        {
          cJSON_Delete (entry);
        }
    }
  if (!ok)
    {
      cJSON_Delete (body);
      return pkFailMem (client);
    }

  cJSON *json = NULL;
  status = pkCall (client, method, path, body, 200, &json);
  cJSON_Delete (body);
  if (status == PK_EHTTP && pkError (client)->code == 400)
    {
      status = pkFail (client, PK_EHTTP, 400,
                       "Invalid email or domain, or more than 200 entries");
    }
  if (status == PK_OK)
    {
      status = pkJsonStrings (client, json, "items", list);
    }
  cJSON_Delete (json);
  if (list == &dummy)
    {
      pkStringListFree (&dummy);
    }

  return status;
}

PKStatus
pkEventTrustedAdd (PKClient *client, const PKChar *event,
                   const PKChar *const *entries, PKSize count,
                   PKStringList *list)
{
  return pkTrustedWrite (client, "POST", event, entries, count, list);
}

PKStatus
pkEventTrustedRemove (PKClient *client, const PKChar *event,
                      const PKChar *const *entries, PKSize count,
                      PKStringList *list)
{
  return pkTrustedWrite (client, "DELETE", event, entries, count, list);
}

PKVoid
pkStringListFree (PKStringList *list)
{
  for (PKSize i = 0; list->items && i < list->count; ++i)
    {
      pkMemFree (list->items[i]);
    }
  pkMemFree (list->items);
  list->items = NULL;
  list->count = 0;
}

PKStatus
pkEventLink (PKClient *client, const PKChar *event, PKChar *link, PKSize size)
{
  PKChar path[PATH_MAX_API];
  PKStatus status
      = event ? pkPath (client, path, sizeof (path), "/event/%s/link", event)
              : pkFail (client, PK_EARG, 0, "No event");
  cJSON *json = NULL;
  if (status == PK_OK)
    {
      status = pkCall (client, "GET", path, NULL, 200, &json);
    }
  if (status == PK_OK)
    {
      status = pkJsonCopy (client, json, "link", link, size);
    }
  cJSON_Delete (json);

  return status;
}

PKStatus
pkEventQr (PKClient *client, const PKChar *event, const PKChar *path,
           PKBool svg)
{
  PKChar url[PATH_MAX_API];
  PKStatus status
      = event ? pkPath (client, url, sizeof (url), "/event/%s/qr", event)
              : pkFail (client, PK_EARG, 0, "No event");
  if (status == PK_OK && svg)
    {
      status = pkQuery (client, url, sizeof (url), "format", "svg");
    }

  return status == PK_OK ? pkFetch (client, url, path) : status;
}

PKStatus
pkEventCard (PKClient *client, const PKChar *event, const PKChar *path,
             const PKChar *design, const PKChar *locale, const PKChar *layout,
             const PKChar *paper, PKBool jpg, PKBool nobg)
{
  PKChar url[PATH_MAX_API];
  PKStatus status
      = event && design
            ? pkPath (client, url, sizeof (url), "/event/%s/card", event)
            : pkFail (client, PK_EARG, 0, "No event or design");
  if (status == PK_OK)
    {
      status = pkQuery (client, url, sizeof (url), "design", design);
    }
  if (status == PK_OK && locale)
    {
      status = pkQuery (client, url, sizeof (url), "locale", locale);
    }
  if (status == PK_OK && layout)
    {
      status = pkQuery (client, url, sizeof (url), "layout", layout);
    }
  if (status == PK_OK && paper)
    {
      status = pkQuery (client, url, sizeof (url), "paper", paper);
    }
  if (status == PK_OK && jpg)
    {
      status = pkQuery (client, url, sizeof (url), "format", "jpg");
    }
  if (status == PK_OK && nobg)
    {
      status = pkQuery (client, url, sizeof (url), "background", "false");
    }

  return status == PK_OK ? pkFetch (client, url, path) : status;
}

PKStatus
pkMediaList (PKClient *client, const PKChar *event, PKMediaList *list)
{
  *list = (PKMediaList){ 0 };

  PKChar path[PATH_MAX_API];
  PKStatus status
      = event ? pkPath (client, path, sizeof (path), "/event/%s/media", event)
              : pkFail (client, PK_EARG, 0, "No event");
  cJSON *json = NULL;
  if (status == PK_OK)
    {
      status = pkCall (client, "GET", path, NULL, 200, &json);
    }
  if (status != PK_OK)
    {
      return status;
    }

  const cJSON *items = cJSON_GetObjectItemCaseSensitive (json, "items");
  if (!cJSON_IsArray (items))
    {
      cJSON_Delete (json);
      return pkFail (client, PK_EDATA, 0, "Response without 'items'");
    }

  PKSize count = (PKSize)cJSON_GetArraySize (items);
  list->media = pkMemZallocArray (count, sizeof (PKMedia));
  if (!list->media)
    {
      cJSON_Delete (json);
      return pkFailMem (client);
    }

  const cJSON *item = NULL;
  cJSON_ArrayForEach (item, items)
  {
    if (!pkParseMedia ((cJSON *)item, &list->media[list->count]))
      {
        status = pkFail (client, PK_EDATA, 0, "Malformed media at index %lu",
                         (unsigned long)list->count);
        break;
      }
    ++list->count;
  }

  cJSON_Delete (json);
  if (status != PK_OK)
    {
      pkMediaListFree (list);
    }

  return status;
}

PKVoid
pkMediaListFree (PKMediaList *list)
{
  pkMemFree (list->media);
  list->media = NULL;
  list->count = 0;
}

PKStatus
pkMediaDownload (PKClient *client, const PKChar *event, const PKChar *media,
                 const PKChar *path)
{
  PKChar url[PATH_MAX_API];
  PKStatus status = event && media
                        ? pkPath (client, url, sizeof (url),
                                  "/event/%s/media/%s", event, media)
                        : pkFail (client, PK_EARG, 0, "No event or media");

  return status == PK_OK ? pkFetch (client, url, path) : status;
}

PKStatus
pkMediaUpdate (PKClient *client, const PKChar *event, const PKChar *media,
               PKInt favorite)
{
  PKChar path[PATH_MAX_API];
  PKStatus status = event && media
                        ? pkPath (client, path, sizeof (path),
                                  "/event/%s/media/%s", event, media)
                        : pkFail (client, PK_EARG, 0, "No event or media");
  if (status != PK_OK)
    {
      return status;
    }

  cJSON *body = cJSON_CreateObject ();
  if (!body
      || (favorite != -1
          && !cJSON_AddBoolToObject (body, "favorite", favorite)))
    {
      cJSON_Delete (body);
      return pkFailMem (client);
    }

  status = pkCall (client, "PATCH", path, body, 200, NULL);
  cJSON_Delete (body);

  return status;
}

PKStatus
pkMediaApprove (PKClient *client, const PKChar *event, const PKChar *media,
                PKBool approved)
{
  PKChar path[PATH_MAX_API];
  PKStatus status = event && media
                        ? pkPath (client, path, sizeof (path),
                                  "/event/%s/media/%s/approve", event, media)
                        : pkFail (client, PK_EARG, 0, "No event or media");
  if (status != PK_OK)
    {
      return status;
    }

  status
      = pkCall (client, approved ? "POST" : "DELETE", path, NULL, 200, NULL);
  if (status == PK_EHTTP && !approved && pkError (client)->code == 422)
    {
      status = pkFail (client, PK_EHTTP, 422,
                       "Event is not moderated, so approval cannot be "
                       "revoked");
    }

  return status;
}

PKStatus
pkUploadStart (PKClient *client, const PKChar *event, const PKChar *type,
               PKLong length, PKChar *session, PKSize size, PKInt *chunk,
               PKInt *total)
{
  PKChar path[PATH_MAX_API];
  PKStatus status = event && type
                        ? pkPath (client, path, sizeof (path),
                                  "/event/%s/media/upload", event)
                        : pkFail (client, PK_EARG, 0, "No event or type");
  if (status != PK_OK)
    {
      return status;
    }

  cJSON *body = cJSON_CreateObject ();
  if (!body || !cJSON_AddStringToObject (body, "type", type)
      || !cJSON_AddNumberToObject (body, "size", (double)length))
    {
      cJSON_Delete (body);
      return pkFailMem (client);
    }

  cJSON *json = NULL;
  status = pkCall (client, "POST", path, body, 200, &json);
  cJSON_Delete (body);
  if (status == PK_OK)
    {
      status = pkJsonCopy (client, json, "session", session, size);
    }
  if (status == PK_OK)
    {
      const cJSON *jsize = cJSON_GetObjectItemCaseSensitive (json, "size");
      const cJSON *jtotal = cJSON_GetObjectItemCaseSensitive (json, "total");
      if (!cJSON_IsNumber (jsize) || !cJSON_IsNumber (jtotal)
          || jsize->valueint <= 0 || jtotal->valueint < 0)
        {
          status = pkFail (client, PK_EDATA, 0,
                           "Malformed upload session response");
        }
      else
        {
          *chunk = jsize->valueint;
          *total = jtotal->valueint;
        }
    }
  cJSON_Delete (json);

  return status;
}

PKStatus
pkUploadChunk (PKClient *client, const PKChar *event, const PKChar *session,
               PKInt index, const PKByte *data, PKSize length)
{
  PKChar path[PATH_MAX_API];
  PKStatus status
      = event && session && data
            ? pkPath (client, path, sizeof (path),
                      "/event/%s/media/upload/%s/chunk/%d", event, session,
                      index)
            : pkFail (client, PK_EARG, 0, "No event, session or data");
  if (status != PK_OK)
    {
      return status;
    }

  PKRequest request = { 0 };
  request.method = "PUT";
  request.url = path;
  request.api = PK_TRUE;
  request.type = "application/octet-stream";
  request.body = data;
  request.length = length;

  PKResponse response;
  status = pkRequest (client, &request, &response);
  if (status == PK_OK)
    {
      status = pkExpect (client, &response, 200);
    }
  pkResponseFree (&response);

  return status;
}

PKStatus
pkUploadComplete (PKClient *client, const PKChar *event, const PKChar *session,
                  PKChar *id, PKSize size)
{
  PKChar path[PATH_MAX_API];
  PKStatus status
      = event && session
            ? pkPath (client, path, sizeof (path),
                      "/event/%s/media/upload/%s/complete", event, session)
            : pkFail (client, PK_EARG, 0, "No event or session");
  cJSON *json = NULL;
  if (status == PK_OK)
    {
      status = pkCall (client, "POST", path, NULL, 200, &json);
    }
  if (status == PK_OK)
    {
      status = pkJsonCopy (client, json, "id", id, size);
    }
  cJSON_Delete (json);

  return status;
}

PKVoid
pkUploadCancel (PKClient *client, const PKChar *event, const PKChar *session)
{
  // The failure that led here stays the one reported.
  PKError error = client->error;

  PKChar path[PATH_MAX_API];
  PKStatus status = event && session
                        ? pkPath (client, path, sizeof (path),
                                  "/event/%s/media/upload/%s", event, session)
                        : pkFail (client, PK_EARG, 0, "No event or session");
  if (status == PK_OK)
    {
      status = pkCall (client, "DELETE", path, NULL, 200, NULL);
    }
  if (status != PK_OK)
    {
      pkLog (client, PK_LOG_WARNING, "Cannot cancel the upload: %s",
             client->error.message);
    }

  client->error = error;
}
