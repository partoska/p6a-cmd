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

#ifndef PK_CONFIG_H
#define PK_CONFIG_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "partoska.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct PKCfgGlobal
{
  PKChar *version;
  PKChar *endpoint;
} PKCfgGlob;

typedef struct PKCfgOAuth
{
  PKChar *client;
  PKChar *device;
  PKChar *token;
  PKChar *scope;
} PKCfgOAuth;

typedef struct PKCfgLogin
{
  PKChar *access;
  PKChar *refresh;
  PKTime expires;
} PKCfgLogin;

typedef struct PKCfg
{
  PKCfgGlob *glob;
  PKCfgOAuth *oauth;
  PKCfgLogin *login;
} PKCfg;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/** Name of the configuration file within the working directory. */
extern const PKChar PK_CONFIG_INI[16];

/**
 * Allocates a configuration with the built-in defaults.
 *
 * @return New PKCfg instance, or NULL on allocation failure.
 */
PKCfg *pkCfgInit (void);

/**
 * Reads a configuration file over the values of cfg. Section and key names
 * match in any case.
 *
 * @param client Client to record failures in.
 * @param cfg    Configuration to update.
 * @param file   Path of the INI file.
 * @return PK_OK on success, PK_EFS when unreadable, PK_EDATA when
 *         malformed, PK_EMEM when out of memory.
 */
PKStatus pkCfgLoad (PKClient *client, PKCfg *cfg, const PKChar *file);

/**
 * Writes a configuration file, readable only by its owner.
 *
 * @param client Client to record failures in.
 * @param cfg    Configuration to write.
 * @param file   Path of the INI file.
 * @return PK_OK on success, PK_EARG for a value spanning lines, PK_EFS
 *         when the file cannot be written.
 */
PKStatus pkCfgSave (PKClient *client, const PKCfg *cfg, const PKChar *file);

/**
 * Stores a login.
 *
 * @param cfg     Configuration to update.
 * @param access  Access token.
 * @param refresh Refresh token.
 * @param expires Expiry of the access token (seconds since the epoch).
 * @return PK_OK on success, PK_EARG or PK_EMEM.
 */
PKStatus pkCfgSetLogin (PKCfg *cfg, const PKChar *access,
                        const PKChar *refresh, PKTime expires);

/**
 * Forgets the login.
 *
 * @param cfg Configuration to update.
 */
PKVoid pkCfgUnsetLogin (PKCfg *cfg);

/**
 * Checks that every setting a client needs is present.
 *
 * @param cfg Configuration to check.
 * @return PK_TRUE when usable.
 */
PKBool pkCfgCheck (const PKCfg *cfg);

/**
 * Releases a configuration. Accepts NULL.
 *
 * @param cfg Configuration to release.
 */
PKVoid pkCfgDestroy (PKCfg *cfg);

#endif
