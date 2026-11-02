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

#ifndef PKC_INI_H
#define PKC_INI_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "partoska.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* INI document, a Bikini document matching names case-insensitively. */
typedef struct BKIni PKCIni;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Allocates and initializes an empty INI document.
 *
 * @return New PKCIni instance, or NULL on allocation failure.
 */
PKCIni *pkcIniInit (void);

/**
 * Releases all memory owned by an INI document.
 *
 * @param ini INI document to destroy.
 */
PKVoid pkcIniDestroy (PKCIni *ini);

/**
 * Parses an INI file from disk into an empty document. Malformed lines,
 * keys outside of a section and repeated keys are errors, logged with the
 * file name and line.
 *
 * @param ini      INI document to populate.
 * @param filename Path to the INI file to load.
 * @return PK_OK on success, PK_EFS when the file cannot be read, PK_EMEM
 *         when out of memory, or PK_EDATA when the file is malformed.
 */
PKInt pkcIniLoad (PKCIni *ini, const PKChar *filename);

/**
 * Writes an INI document to disk, after the p6a header comment, as
 * key=value lines.
 *
 * @param ini      INI document to serialize.
 * @param filename Destination file path.
 * @return PK_OK on success, or a PK_E* error code on failure.
 */
PKInt pkcIniSave (const PKCIni *ini, const PKChar *filename);

/**
 * Looks up a value by section and key.
 *
 * @param ini     INI document to search.
 * @param section Section name.
 * @param key     Key name.
 * @return Pointer to the value string, or NULL if not found.
 */
const PKChar *pkcIniGet (const PKCIni *ini, const PKChar *section,
                         const PKChar *key);

/**
 * Sets or inserts a key/value pair within a section.
 *
 * @param ini     INI document to modify.
 * @param section Section name.
 * @param key     Key name; non-empty, without surrounding blanks, line
 *                breaks, = or :, not starting with [, ; or #.
 * @param value   Value string to store; without line breaks.
 * @return PK_OK on success, or a PK_E* error code on failure.
 */
PKInt pkcIniSet (PKCIni *ini, const PKChar *section, const PKChar *key,
                 const PKChar *value);

/**
 * Tells whether any section holds a key with exactly the given value.
 *
 * @param ini   INI document to search.
 * @param key   Key name.
 * @param value Value to look for.
 * @return PK_TRUE if found, PK_FALSE otherwise.
 */
PKBool pkcIniHasValue (const PKCIni *ini, const PKChar *key,
                       const PKChar *value);

#endif
