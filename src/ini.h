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

#ifndef INI_H
#define INI_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "types.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct
{
  PLChar *section;
  PLChar *key;
  PLChar *value;
} PLIniEntry;

typedef struct
{
  PLIniEntry *entries;
  PLSize count;
  PLSize capacity;
} PLIni;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Allocates and initializes an empty INI document.
 *
 * @return New PLIni instance, or NULL on allocation failure.
 */
PLIni *plIniInit (void);

/**
 * Releases all memory owned by an INI document.
 *
 * @param ini INI document to destroy.
 */
void plIniDestroy (PLIni *ini);

/**
 * Parses an INI file from disk into an existing document.
 *
 * @param ini      INI document to populate.
 * @param filename Path to the INI file to load.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plIniLoad (PLIni *ini, const PLChar *filename);

/**
 * Writes an INI document to disk.
 *
 * @param ini      INI document to serialize.
 * @param filename Destination file path.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plIniSave (PLIni *ini, const PLChar *filename);

/**
 * Looks up a value by section and key.
 *
 * @param ini     INI document to search.
 * @param section Section name.
 * @param key     Key name.
 * @return Pointer to the value string, or NULL if not found.
 */
const PLChar *plIniGet (PLIni *ini, const PLChar *section, const PLChar *key);

/**
 * Sets or inserts a key/value pair within a section.
 *
 * @param ini     INI document to modify.
 * @param section Section name.
 * @param key     Key name.
 * @param value   Value string to store.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plIniSet (PLIni *ini, const PLChar *section, const PLChar *key,
                const PLChar *value);

#endif
