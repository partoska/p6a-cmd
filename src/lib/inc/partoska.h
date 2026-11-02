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

#ifndef PARTOSKA_H
#define PARTOSKA_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "pktypes.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define PK_VERSION_MAJOR 2
#define PK_VERSION_MINOR 0
#define PK_VERSION_PATCH 0
#define PK_VERSION_STRING                                                     \
  PK_XSTR (PK_VERSION_MAJOR)                                                  \
  "." PK_XSTR (PK_VERSION_MINOR) "." PK_XSTR (PK_VERSION_PATCH)

#define PK_ERROR_MAX (1024)

#ifdef __cplusplus
extern "C"
{
#endif

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
   * Types
   * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  /** Result of a call: PK_OK, or the category of the failure. */
  typedef enum PKStatus
  {
    PK_OK = 0,       /**< Success. */
    PK_EARG = -1,    /**< Invalid argument. */
    PK_EMEM = -2,    /**< Out of memory. */
    PK_EFS = -3,     /**< File system; code is errno. */
    PK_ENET = -4,    /**< Transport failed; code is the libcurl code. */
    PK_EHTTP = -5,   /**< Unexpected HTTP status; code is the status. */
    PK_EAUTH = -6,   /**< Not logged in, or the login was refused. */
    PK_EDATA = -7,   /**< Malformed response or configuration file. */
    PK_ESTOPPED = -8 /**< A stop was requested, see pkStop(). */
  } PKStatus;

  /** Details of the most recent failure within a client; see pkError(). */
  typedef struct PKError
  {
    PKStatus status;              /**< PK_OK until something fails. */
    PKLong code;                  /**< Underlying error code, or 0. */
    PKChar message[PK_ERROR_MAX]; /**< Readable, with context, e.g.
                                       "Cannot start the upload: Request
                                       failed with HTTP 404". */
  } PKError;

  /** Severity of a log message, and the threshold of a client. */
  typedef enum PKLogLevel
  {
    PK_LOG_DEBUG = 0, /**< Requests, responses and decisions. */
    PK_LOG_INFO,      /**< Progress worth showing a user. */
    PK_LOG_WARNING,   /**< Problems the call recovered from. */
    PK_LOG_SILENT     /**< Threshold only: suppresses every message. */
  } PKLogLevel;

  /**
   * Receives the log messages of a client.
   *
   * @param user    Pointer given to pkLogSetHandler().
   * @param level   Severity of the message.
   * @param message Text without trailing newline.
   */
  typedef PKVoid (*PKLogFn) (PKVoid *user, PKLogLevel level,
                             const PKChar *message);

  /**
   * Shows the user where to approve a login (OAuth 2.0 device authorization
   * grant, RFC 8628); pkLogin() then waits for the decision.
   *
   * @param user    Pointer given to pkLogin().
   * @param uri     Page to open in a browser, on any device.
   * @param code    Code the page shows, for the user to check.
   * @param expires Seconds until the code expires.
   */
  typedef PKVoid (*PKLoginCodeFn) (PKVoid *user, const PKChar *uri,
                                   const PKChar *code, PKLong expires);

  /** Opaque client, from pkInit(); released by pkDestroy(). */
  typedef struct PKClient PKClient;

  /** An event, as listed, created or updated. */
  typedef struct PKEvent
  {
    PKChar id[64];      /**< UUID. */
    PKChar name[256];   /**< Name, UTF-8. */
    PKChar created[32]; /**< Creation time, or empty. */
    PKChar expires[32]; /**< Time the event expires, or empty. */
    PKChar from[32];    /**< Start time. */
    PKChar to[32];      /**< End time. */
    PKBool owner;       /**< The user owns the event. */
    PKInt guests;       /**< Number of guests. */
    PKInt media;        /**< Number of media items. */
    PKBool favorite;    /**< The user marked it as a favorite. */
    PKBool pub;         /**< Public: visible without an invitation. */
    PKBool moderated;   /**< Uploads need approval before they show. */
  } PKEvent;

  /** Events of pkEventList(); release with pkEventListFree(). */
  typedef struct PKEventList
  {
    PKEvent *events; /**< count events. */
    PKSize count;    /**< Number of events. */
  } PKEventList;

  /** A media item (photo or video) of an event. */
  typedef struct PKMedia
  {
    PKChar id[64];       /**< UUID. */
    PKChar type[32];     /**< MIME type, e.g. "image/jpeg", "video/mp4". */
    PKChar uploaded[32]; /**< Upload time. */
    PKChar taken[32];    /**< Capture time, or empty when unknown. */
    PKBool owner;        /**< The user uploaded it. */
    PKBool favorite;     /**< The user marked it as a favorite. */
    PKInt favorites;     /**< Number of users who marked it. */
    PKBool approved;     /**< Approved, or the event is not moderated. */
  } PKMedia;

  /** Media of pkMediaList(); release with pkMediaListFree(). */
  typedef struct PKMediaList
  {
    PKMedia *media; /**< count media items. */
    PKSize count;   /**< Number of media items. */
  } PKMediaList;

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
   * Declarations
   * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  /**
   * Initializes the libraries the client relies on (libcurl) for the whole
   * process. Call it once, before any other thread runs and before the
   * first client is created.
   *
   * @return PK_OK on success, or PK_ENET.
   */
  PKStatus pkGlobalInit (void);

  /**
   * Releases what pkGlobalInit() set up, after the last client is gone.
   */
  PKVoid pkGlobalCleanup (void);

  /**
   * Allocates a client with the built-in configuration (Partoska.com
   * endpoints), not logged in and without a working directory; see
   * pkOpen().
   *
   * @return New client, or NULL when out of memory.
   */
  PKClient *pkInit (void);

  /**
   * Releases a client. Accepts NULL.
   *
   * @param client Client to release.
   */
  PKVoid pkDestroy (PKClient *client);

  /**
   * Gets the details of the last failure.
   *
   * @param client Client to inspect.
   * @return Error record owned by the client; status PK_OK when nothing
   *         has failed yet.
   */
  const PKError *pkError (const PKClient *client);

  /**
   * Describes a status.
   *
   * @param status Status to describe.
   * @return Static, human-readable text.
   */
  const PKChar *pkStatusText (PKStatus status);

  /**
   * Routes the log messages of a client.
   *
   * @param client  Client to configure.
   * @param handler Receives the messages, or NULL to drop them.
   * @param user    Passed to the handler.
   */
  PKVoid pkLogSetHandler (PKClient *client, PKLogFn handler, PKVoid *user);

  /**
   * Sets the least severity passed to the handler; PK_LOG_INFO by default.
   *
   * @param client Client to configure.
   * @param level  Threshold, PK_LOG_SILENT to pass nothing.
   */
  PKVoid pkLogSetLevel (PKClient *client, PKLogLevel level);

  /**
   * Requests a stop. Async-signal-safe, meant to be called from SIGINT and
   * SIGTERM handlers: requests in progress are aborted, waits end early and
   * calls return PK_ESTOPPED.
   *
   * @param client Client to stop.
   */
  PKVoid pkStop (PKClient *client);

  /**
   * Checks for a stop request.
   *
   * @param client Client to check.
   * @return PK_TRUE once pkStop() was called.
   */
  PKBool pkStopping (const PKClient *client);

  /**
   * Opens the working directory, which holds the configuration and login
   * (p6a.ini), and reads its configuration when there is one.
   *
   * @param client Client to open.
   * @param dir    Existing working directory, or NULL for $P6A_HOME, else
   *               ~/.p6a (created when missing), else the current directory.
   * @return PK_OK on success, PK_EFS when the directory is unusable or the
   *         configuration unreadable, PK_EDATA when it is malformed.
   */
  PKStatus pkOpen (PKClient *client, const PKChar *dir);

  /**
   * Gets the working directory chosen by pkOpen().
   *
   * @param client Client to inspect.
   * @return Path, empty before pkOpen().
   */
  const PKChar *pkDir (const PKClient *client);

  /**
   * Tells whether the client holds a usable login: an access token, or a
   * refresh token to renew it with. The server may still refuse it.
   *
   * @param client Client to inspect.
   * @return PK_TRUE when the login is valid.
   */
  PKBool pkLoginValid (const PKClient *client);

  /**
   * Replaces the configuration, login included, with the one in a file and
   * saves it to the working directory.
   *
   * @param client Opened client.
   * @param path   Configuration file to import.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkImport (PKClient *client, const PKChar *path);

  /**
   * Logs in with the OAuth 2.0 device authorization grant (RFC 8628): gets
   * a code, passes it to prompt, then waits until the user approves or
   * denies it in a browser, or it expires. The login is saved to the
   * working directory.
   *
   * @param client Opened client.
   * @param prompt Shows the user the page and code.
   * @param user   Passed to prompt.
   * @return PK_OK on success, PK_EAUTH when denied or expired, or another
   *         PK_E* status.
   */
  PKStatus pkLogin (PKClient *client, PKLoginCodeFn prompt, PKVoid *user);

  /**
   * Forgets the login and saves the configuration.
   *
   * @param client Opened client.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkLogout (PKClient *client);

  /**
   * Lists the events of the user.
   *
   * @param client Logged-in client.
   * @param query  Search query, or NULL for all events.
   * @param list   Receives the events; release with pkEventListFree().
   *               Empty on failure, with nothing to release.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkEventList (PKClient *client, const PKChar *query,
                        PKEventList *list);

  /**
   * Releases the events of a list. Accepts an empty list.
   *
   * @param list List to empty.
   */
  PKVoid pkEventListFree (PKEventList *list);

  /**
   * Creates an event.
   *
   * @param client Logged-in client.
   * @param name   Name of the event.
   * @param event  Receives the created event.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkEventCreate (PKClient *client, const PKChar *name,
                          PKEvent *event);

  /**
   * Partially updates an event; only the given fields change.
   *
   * @param client    Logged-in client.
   * @param event     UUID of the event.
   * @param name      New name, or NULL.
   * @param from      New start date, or NULL.
   * @param to        New end date, or NULL.
   * @param pub       1 public, 0 private, -1 unchanged.
   * @param favorite  1 favorite, 0 not, -1 unchanged.
   * @param moderated 1 moderated, 0 not, -1 unchanged.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkEventUpdate (PKClient *client, const PKChar *event,
                          const PKChar *name, const PKChar *from,
                          const PKChar *to, PKInt pub, PKInt favorite,
                          PKInt moderated);

  /**
   * Gets the primary invite link of an event.
   *
   * @param client Logged-in client.
   * @param event  UUID of the event.
   * @param link   Receives the URL.
   * @param size   Capacity of link in bytes.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkEventLink (PKClient *client, const PKChar *event, PKChar *link,
                        PKSize size);

  /**
   * Downloads the QR code of an event's invite link.
   *
   * @param client Logged-in client.
   * @param event  UUID of the event.
   * @param path   Destination file; removed again on failure.
   * @param svg    PK_TRUE for SVG, PK_FALSE for PNG.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkEventQr (PKClient *client, const PKChar *event,
                      const PKChar *path, PKBool svg);

  /**
   * Downloads a printable share card of an event.
   *
   * @param client Logged-in client.
   * @param event  UUID of the event.
   * @param path   Destination file; removed again on failure.
   * @param design Design (bday, tech, match, forest, garden, gold, romantic,
   *               silver, neon, nineties, cottage, wish, velvet, hallows).
   * @param locale Language (en, cs, sk, pl, ru, es, de, fr, it, nl, uk), or
   *               NULL for the server default.
   * @param layout Layout (single, business), or NULL for the default.
   * @param paper  Paper size (a4, letter), or NULL for the default.
   * @param jpg    PK_TRUE for JPEG, PK_FALSE for PDF.
   * @param nobg   PK_TRUE for a white background.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkEventCard (PKClient *client, const PKChar *event,
                        const PKChar *path, const PKChar *design,
                        const PKChar *locale, const PKChar *layout,
                        const PKChar *paper, PKBool jpg, PKBool nobg);

  /**
   * Lists the media of an event.
   *
   * @param client Logged-in client.
   * @param event  UUID of the event.
   * @param list   Receives the media; release with pkMediaListFree().
   *               Empty on failure, with nothing to release.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkMediaList (PKClient *client, const PKChar *event,
                        PKMediaList *list);

  /**
   * Releases the media of a list. Accepts an empty list.
   *
   * @param list List to empty.
   */
  PKVoid pkMediaListFree (PKMediaList *list);

  /**
   * Downloads a media file.
   *
   * @param client Logged-in client.
   * @param event  UUID of the event.
   * @param media  UUID of the media item.
   * @param path   Destination file; removed again on failure.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkMediaDownload (PKClient *client, const PKChar *event,
                            const PKChar *media, const PKChar *path);

  /**
   * Partially updates a media item.
   *
   * @param client   Logged-in client.
   * @param event    UUID of the event.
   * @param media    UUID of the media item.
   * @param favorite 1 favorite, 0 not, -1 unchanged.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkMediaUpdate (PKClient *client, const PKChar *event,
                          const PKChar *media, PKInt favorite);

  /**
   * Approves a media item of a moderated event, or puts it back on hold.
   * Revoking is refused (PK_EHTTP, 422) on events that are not moderated.
   *
   * @param client   Logged-in client.
   * @param event    UUID of the event.
   * @param media    UUID of the media item.
   * @param approved PK_TRUE to approve, PK_FALSE to revoke the approval.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkMediaApprove (PKClient *client, const PKChar *event,
                           const PKChar *media, PKBool approved);

  /**
   * Uploads a media file through a chunked upload session, cancelled on
   * failure. The type follows the extension: .jpg, .jpeg, .mp4 or .mov.
   *
   * @param client Logged-in client.
   * @param event  UUID of the event.
   * @param path   Media file to upload.
   * @param id     Receives the UUID of the uploaded media.
   * @param size   Capacity of id in bytes.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkMediaUpload (PKClient *client, const PKChar *event,
                          const PKChar *path, PKChar *id, PKSize size);

  /**
   * Starts a chunked upload session.
   *
   * @param client  Logged-in client.
   * @param event   UUID of the event.
   * @param type    MIME type of the whole file.
   * @param length  Size of the whole file in bytes.
   * @param session Receives the UUID of the session.
   * @param size    Capacity of session in bytes.
   * @param chunk   Receives the size of every chunk but the last.
   * @param total   Receives the number of chunks.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkUploadStart (PKClient *client, const PKChar *event,
                          const PKChar *type, PKLong length, PKChar *session,
                          PKSize size, PKInt *chunk, PKInt *total);

  /**
   * Uploads one chunk of a session.
   *
   * @param client  Logged-in client.
   * @param event   UUID of the event.
   * @param session UUID of the session.
   * @param index   Zero-based index of the chunk.
   * @param data    Bytes of the chunk.
   * @param length  Number of bytes.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkUploadChunk (PKClient *client, const PKChar *event,
                          const PKChar *session, PKInt index,
                          const PKByte *data, PKSize length);

  /**
   * Completes a session once every chunk was uploaded.
   *
   * @param client  Logged-in client.
   * @param event   UUID of the event.
   * @param session UUID of the session.
   * @param id      Receives the UUID of the uploaded media.
   * @param size    Capacity of id in bytes.
   * @return PK_OK on success, or a PK_E* status.
   */
  PKStatus pkUploadComplete (PKClient *client, const PKChar *event,
                             const PKChar *session, PKChar *id, PKSize size);

  /**
   * Cancels a session, freeing its reserved space at once. Keeps the error
   * that led to it: a failure here is only logged.
   *
   * @param client  Logged-in client.
   * @param event   UUID of the event.
   * @param session UUID of the session.
   */
  PKVoid pkUploadCancel (PKClient *client, const PKChar *event,
                         const PKChar *session);

#ifdef __cplusplus
}
#endif

#endif
