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

#ifndef DOWNLOAD_H
#define DOWNLOAD_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "types.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Downloads a single media item to a specified output file.
 *
 * @param base   API base URL.
 * @param bearer OAuth bearer token.
 * @param event  UUID of the event that owns the media.
 * @param media  UUID of the media item to download.
 * @param output Destination file path.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plDownloadSingle (const PLChar *base, const PLChar *bearer,
                        const PLChar *event, const PLChar *media,
                        const PLChar *output);

/**
 * Downloads all media for an event to a local directory.
 *
 * Files are named sequentially (IMG_XXXX.jpg, MOV_XXXX.mp4/mov) based
 * on existing files found in the target directory.
 *
 * @param base     API base URL.
 * @param bearer   OAuth bearer token.
 * @param event    UUID of the event to download.
 * @param target   Destination directory path.
 * @param owner    When true, restricts download to media owned by the user.
 * @param favorite When true, restricts download to favorited media.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plDownloadAll (const PLChar *base, const PLChar *bearer,
                     const PLChar *event, const PLChar *target, PLBool owner,
                     PLBool favorite);

#endif
