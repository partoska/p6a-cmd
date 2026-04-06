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

#ifndef FS_H
#define FS_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "types.h"
#include <stdio.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct PLDir PLDir;
typedef FILE PLFile;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Performs platform-specific console initialization (e.g., UTF-8 mode on
 * Windows).
 */
void plConsoleInit (void);

/**
 * Returns whether the given path refers to an existing directory.
 *
 * @param path Path to test.
 * @return PL_TRUE if path is a directory, PL_FALSE otherwise.
 */
PLBool plIsDir (const PLChar *path);

/**
 * Returns whether the given path refers to an existing regular file.
 *
 * @param path Path to test.
 * @return PL_TRUE if path is a file, PL_FALSE otherwise.
 */
PLBool plIsFile (const PLChar *path);

/**
 * Creates a directory, including any missing parent directories.
 *
 * @param path Directory path to create.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plCreateDir (const PLChar *path);

/**
 * Sets file permissions to owner-read/write only (mode 0600).
 *
 * @param path Path to the file.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plChmod600 (const PLChar *path);

/**
 * Opens a file at the given path with the specified mode.
 *
 * @param path Path to the file.
 * @param mode fopen-compatible mode string (e.g. "rb", "wb").
 * @return Pointer to an open PLFile, or NULL on failure.
 */
PLFile *plFileOpen (const PLChar *path, const PLChar *mode);

/**
 * Closes a previously opened file.
 *
 * @param file File to close.
 */
void plFileClose (PLFile *file);

/**
 * Opens a directory for iteration.
 *
 * @param path Directory path to open.
 * @return Opaque PLDir handle, or NULL on failure.
 */
PLDir *plOpenDir (const PLChar *path);

/**
 * Returns the name of the next entry in an open directory.
 *
 * @param dir Open directory handle.
 * @return Pointer to the next entry name, or NULL when the directory is
 * exhausted.
 */
const PLChar *plReadDir (PLDir *dir);

/**
 * Closes a directory handle opened with plOpenDir.
 *
 * @param dir Directory handle to close.
 */
void plCloseDir (PLDir *dir);

#endif
