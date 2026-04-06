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

#ifndef API_H
#define API_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "types.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct PLEvent
{
  PLChar id[64];
  PLChar name[256];
  PLChar created[32];
  PLChar expires[32];
  PLChar from[32];
  PLChar to[32];
  PLBool owner;
  PLInt guests;
  PLInt media;
  PLBool favorite;
  PLBool pub;
} PLEvent;

typedef struct PLEventList
{
  PLEvent *events;
  PLSize count;
} PLEventList;

typedef struct PLMedia
{
  PLChar id[64];
  PLChar type[32];
  PLChar uploaded[32];
  PLChar taken[32];
  PLBool owner;
  PLBool favorite;
  PLInt favorites;
} PLMedia;

typedef struct PLMediaList
{
  PLMedia *media;
  PLSize count;
} PLMediaList;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Fetches the list of events from the API.
 *
 * @param base  API base URL.
 * @param token OAuth bearer token.
 * @param query Optional search query string, or NULL.
 * @return Heap-allocated event list, or NULL on failure.
 *         Caller must free with plFreeEventList.
 */
PLEventList *plApiEventList (const PLChar *base, const PLChar *token,
                             const PLChar *query);

/**
 * Creates a new event on the API.
 *
 * @param event  Output structure populated with the created event data.
 * @param base   API base URL.
 * @param token  OAuth bearer token.
 * @param name   Display name for the new event.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plApiEventCreate (PLEvent *event, const PLChar *base,
                        const PLChar *token, const PLChar *name);

/**
 * Releases all memory owned by an event list.
 *
 * @param list Event list to free.
 */
void plFreeEventList (PLEventList *list);

/**
 * Fetches the list of media items for a specific event.
 *
 * @param base  API base URL.
 * @param token OAuth bearer token.
 * @param event UUID of the event.
 * @return Heap-allocated media list, or NULL on failure.
 *         Caller must free with plFreeMediaList.
 */
PLMediaList *plApiMediaList (const PLChar *base, const PLChar *token,
                             const PLChar *event);

/**
 * Releases all memory owned by a media list.
 *
 * @param list Media list to free.
 */
void plFreeMediaList (PLMediaList *list);

/**
 * Downloads a single media file to a local path.
 *
 * @param base  API base URL.
 * @param token OAuth bearer token.
 * @param event UUID of the event that owns the media.
 * @param media UUID of the media item.
 * @param path  Destination file path.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plApiMediaFetch (const PLChar *base, const PLChar *token,
                       const PLChar *event, const PLChar *media,
                       const PLChar *path);

/**
 * Downloads the QR code image for an event.
 *
 * @param base  API base URL.
 * @param token OAuth bearer token.
 * @param event UUID of the event.
 * @param path  Destination file path.
 * @param svg   When true, requests SVG format instead of PNG.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plApiQrFetch (const PLChar *base, const PLChar *token,
                    const PLChar *event, const PLChar *path, PLBool svg);

/**
 * Partially updates an event via PATCH.
 *
 * Only non-NULL string parameters and non-(-1) integer parameters are
 * included in the request body.
 *
 * @param base  API base URL.
 * @param token OAuth bearer token.
 * @param id    UUID of the event to update.
 * @param name  New name, or NULL to leave unchanged.
 * @param from  New start date, or NULL to leave unchanged.
 * @param to    New end date, or NULL to leave unchanged.
 * @param pub   Public flag: 1 to set public, 0 to set private, -1 to leave
 *              unchanged.
 * @param fav   Favorite flag: 1 to favorite, 0 to unfavorite, -1 to leave
 *              unchanged.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plApiEventUpdate (const PLChar *base, const PLChar *token,
                        const PLChar *id, const PLChar *name,
                        const PLChar *from, const PLChar *to, PLInt pub,
                        PLInt fav);

/**
 * Fetches the primary invite link URL for an event.
 *
 * @param link  Output buffer to receive the URL string.
 * @param size  Capacity of the output buffer in bytes.
 * @param base  API base URL.
 * @param token OAuth bearer token.
 * @param event UUID of the event.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plApiLinkFetch (PLChar *link, PLSize size, const PLChar *base,
                      const PLChar *token, const PLChar *event);

#endif
