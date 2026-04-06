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

#ifndef CONFIG_H
#define CONFIG_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "types.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct PLCfgGlobal
{
  PLChar *version;
  PLChar *endpoint;
} PLCfgGlob;

typedef struct PLCfgOAuth
{
  PLChar *client;
  PLChar *redirect;
  PLChar *authorize;
  PLChar *token;
  PLChar *scope;
} PLCfgOAuth;

typedef struct PLCfgLogin
{
  PLChar *access;
  PLChar *refresh;
  PLTime expires;
} PLCfgLogin;

typedef struct PLCfg
{
  PLCfgGlob *glob;
  PLCfgOAuth *oauth;
  PLCfgLogin *login;
} PLCfg;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** Filename of the per-working-directory configuration INI file. */
extern const PLChar PL_CONFIG_INI[16];

/**
 * Allocates and zero-initializes a configuration structure.
 *
 * @return New PLCfg instance, or NULL on allocation failure.
 */
PLCfg *plCfgInit (void);

/**
 * Loads configuration values from an INI file into an existing structure.
 *
 * @param cfg  Configuration structure to populate.
 * @param file Path to the INI file to load.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plCfgLoad (PLCfg *cfg, const PLChar *file);

/**
 * Writes the current configuration to an INI file.
 *
 * @param cfg  Configuration structure to serialize.
 * @param file Destination file path.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plCfgSave (const PLCfg *cfg, const PLChar *file);

/**
 * Stores OAuth login tokens and their expiry in the configuration.
 *
 * @param cfg     Configuration structure to update.
 * @param access  Access token string.
 * @param refresh Refresh token string.
 * @param expires Token expiry as a Unix timestamp.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plCfgSetLogin (PLCfg *cfg, const PLChar *access, const PLChar *refresh,
                     PLLong expires);

/**
 * Clears all stored login tokens from the configuration.
 *
 * @param cfg Configuration structure to clear.
 * @return PL_EOK on success, or a PL_E* error code on failure.
 */
PLInt plCfgUnsetLogin (PLCfg *cfg);

/**
 * Returns whether the configuration structure is fully initialized.
 *
 * Validates that all sub-structures are allocated and that the global
 * and OAuth required fields are non-NULL.
 *
 * @param cfg Configuration structure to inspect.
 * @return PL_TRUE if all required fields are present, PL_FALSE otherwise.
 */
PLBool plCfgCheck (const PLCfg *cfg);

/**
 * Releases all memory owned by a configuration structure.
 *
 * @param cfg Configuration structure to destroy.
 */
void plCfgDestroy (PLCfg *cfg);

#endif
