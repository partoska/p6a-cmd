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

#ifndef PKC_DOWNLOAD_H
#define PKC_DOWNLOAD_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "partoska.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Downloads a single media item to a specified output file.
 *
 * @param client   Logged-in client.
 * @param event  UUID of the event that owns the media.
 * @param media  UUID of the media item to download.
 * @param output Destination file path.
 * @return PK_OK on success, or a PK_E* error code on failure.
 */
PKInt pkcDownloadSingle (PKClient *client, const PKChar *event,
                         const PKChar *media, const PKChar *output);

/**
 * Downloads all media for an event to a local directory.
 *
 * Files are named sequentially (IMG_XXXX.jpg, MOV_XXXX.mp4/mov) based
 * on existing files found in the target directory.
 *
 * @param client   Logged-in client.
 * @param event    UUID of the event to download.
 * @param target   Destination directory path.
 * @param owner    When true, restricts download to media owned by the user.
 * @param favorite When true, restricts download to favorited media.
 * @return PK_OK on success, or a PK_E* error code on failure.
 */
PKInt pkcDownloadAll (PKClient *client, const PKChar *event,
                      const PKChar *target, PKBool owner, PKBool favorite);

#endif
