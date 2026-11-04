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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Includes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "approve.h"
#include "arg.h"
#include "card.h"
#include "create.h"
#include "download.h"
#include "edit.h"
#include "join.h"
#include "link.h"
#include "list.h"
#include "log.h"
#include "media.h"
#include "partoska.h"
#include "pktypes.h"
#include "plat.h"
#include "qr.h"
#include "report.h"
#include "sync.h"
#include "trusted.h"
#include "update.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Exit codes. */
#define PKC_EXIT_SUCCESS (0)
#define PKC_EXIT_FAILURE (1)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Variables
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* The client of the running command, for the signal handlers. */
static PKClient *pkcClient = NULL;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKVoid
pkcPrintLogo (void)
{
  PKC_INFO ("   ___           __           __");
  PKC_INFO ("  / _ \\___ _____/ /____  ___ / /_____ _");
  PKC_INFO (" / ___/ _ `/ __/ __/ _ \\(_-</  '_/ _ `/");
  PKC_INFO ("/_/   \\_,_/_/  \\__/\\___/___/_/\\_\\\\_,_/");
  PKC_INFO ("");
  PKC_INFO ("Partoska Command-Line Interface (p6a) v" PK_VERSION_STRING);
  PKC_INFO ("(C) 2026 Fabrika Charvat s.r.o., <https://www.partoska.com/p6a>");
  PKC_INFO ("");
  PKC_INFO ("Licensed under the MIT License.");
  PKC_INFO ("");
}

static PKVoid
pkcOnSignal (PKInt signal)
{
  PK_UNUSED (signal);
  if (pkcClient)
    {
      pkStop (pkcClient);
    }
}

static PKVoid
pkcOnLog (PKVoid *user, PKLogLevel level, const PKChar *message)
{
  PK_UNUSED (user);
  switch (level)
    {
    case PK_LOG_DEBUG:
      PKC_DEBUG ("%s", message);
      break;
    case PK_LOG_INFO:
      PKC_INFO ("%s", message);
      break;
    default:
      PKC_WARN ("%s", message);
      break;
    }
}

/* Shows where to approve the login; user points to the PKCArgFmt. */
static PKVoid
pkcOnPrompt (PKVoid *user, const PKChar *uri, const PKChar *code,
             PKLong expires)
{
  PKChar quri[2048];
  PKChar qcode[128];
  switch (*(const PKCArgFmt *)user)
    {
    case PKC_FMTJSON:
      pkcJsonEsc (quri, sizeof (quri), uri);
      pkcJsonEsc (qcode, sizeof (qcode), code);
      PKC_INFO ("{\"uri\":\"%s\",\"code\":\"%s\",\"expires\":%ld}", quri,
                qcode, expires);
      break;
    case PKC_FMTCSV:
      pkcCsvQuote (quri, sizeof (quri), uri);
      pkcCsvQuote (qcode, sizeof (qcode), code);
      PKC_INFO ("uri,code,expires");
      PKC_INFO ("%s,%s,%ld", quri, qcode, expires);
      break;
    case PKC_FMTONE:
      PKC_INFO ("%s", uri);
      break;
    default:
      PKC_INFO ("Open this URL in a browser, on this or any other device:");
      PKC_INFO ("");
      PKC_INFO ("  %s", uri);
      PKC_INFO ("");
      PKC_INFO ("Check that it shows the code: %s", code);
      PKC_INFO ("");
      PKC_INFO ("Waiting for you to allow access (expires in %d minutes) ...",
                (PKInt)((expires + 59) / 60));
      break;
    }

  // A script or an agent may read this through a pipe while the login waits.
  pkcFlush ();
}

/* Checks the arguments a command needs, before anything else happens. */
static PKStatus
pkcValidate (const PKCArg *args)
{
  const PKChar *missing = NULL;
  switch (args->cmd)
    {
    case PKC_CSYNC:
      missing = !args->c.sync.target ? "Invalid target directory" : NULL;
      break;
    case PKC_CCREATE:
      missing = !args->c.create.name ? "Event name is required" : NULL;
      break;
    case PKC_CUPDATE:
      missing = !args->c.update.event ? "Event ID is required" : NULL;
      break;
    case PKC_CQR:
      missing = !args->c.qr.event ? "Event ID is required" : NULL;
      break;
    case PKC_CLINK:
      missing = !args->c.link.event ? "Event ID is required" : NULL;
      break;
    case PKC_CMEDIA:
      missing = !args->c.media.event ? "Event ID is required" : NULL;
      break;
    case PKC_CDOWNLOAD:
      missing = !args->c.download.event ? "Event ID is required" : NULL;
      break;
    case PKC_CEDIT:
      missing = !args->c.edit.event || !args->c.edit.media
                    ? "Event ID and media ID are required"
                    : NULL;
      break;
    case PKC_CAPPROVE:
      missing = !args->c.approve.event || !args->c.approve.media
                    ? "Event ID and media ID are required"
                    : NULL;
      break;
    case PKC_CCARD:
      missing = !args->c.card.event    ? "Event ID is required"
                : !args->c.card.design ? "Card design is required"
                                       : NULL;
      break;
    case PKC_CUPLOAD:
      missing = !args->c.upload.event || !args->c.upload.source
                    ? "Event ID and source file are required"
                    : NULL;
      break;
    case PKC_CJOIN:
      missing
          = !args->c.join.invite ? "Invite code or link is required" : NULL;
      break;
    case PKC_CTRUSTED:
      missing = !args->c.trusted.event ? "Event ID is required" : NULL;
      break;
    default:
      break;
    }

  if (missing)
    {
      PKC_ERROR ("%s", missing);
      return PK_EARG;
    }

  return PK_OK;
}

/* Creates the client and opens the working directory. */
static PKClient *
pkcOpen (const PKChar *dir)
{
  PKClient *client = pkInit ();
  if (!client)
    {
      PKC_ERROR ("Out of memory");
      return NULL;
    }

  pkLogSetHandler (client, pkcOnLog, NULL);
#ifdef _DEBUG
  pkLogSetLevel (client, PK_LOG_DEBUG);
#endif

  if (pkOpen (client, dir) != PK_OK)
    {
      pkcReport (client, "Failed to open the working directory");
      pkDestroy (client);
      return NULL;
    }

  return client;
}

/* Logs in; formats other than plain print only the login prompt, the exit
 * code tells the outcome. */
static PKStatus
pkcDoLogin (PKClient *client, const PKChar *import, PKCArgFmt fmt)
{
  if (fmt == PKC_FMTPLAIN)
    {
      pkcPrintLogo ();
    }

  if (import)
    {
      PKC_DEBUG ("Importing: %s", import);
      if (pkImport (client, import) != PK_OK)
        {
          pkcReport (client, "Failed to import configuration");
          return pkError (client)->status;
        }
    }

  if (pkLogin (client, pkcOnPrompt, &fmt) != PK_OK)
    {
      PKC_ERROR ("Login failed: %s", pkError (client)->message);
      return pkError (client)->status;
    }

  if (fmt == PKC_FMTPLAIN)
    {
      PKC_INFO ("");
      PKC_INFO ("Login successful!");
    }

  return PK_OK;
}

static PKStatus
pkcDoLogout (PKClient *client)
{
  if (pkLogout (client) != PK_OK)
    {
      pkcReport (client, "Failed to logout");
      return pkError (client)->status;
    }

  return PK_OK;
}

/* Tells whether a login is saved; the exit code carries the answer too. */
static PKStatus
pkcDoStatus (PKClient *client, PKCArgFmt fmt)
{
  PKBool login = pkLoginValid (client);
  const PKChar *value = login ? "true" : "false";
  switch (fmt)
    {
    case PKC_FMTJSON:
      PKC_INFO ("{\"login\":%s}", value);
      break;
    case PKC_FMTCSV:
      PKC_INFO ("login");
      PKC_INFO ("%s", value);
      break;
    case PKC_FMTONE:
      PKC_INFO ("%s", value);
      break;
    default:
      PKC_INFO ("%s", login ? "Logged in" : "Not logged in");
      break;
    }

  return login ? PK_OK : PK_EAUTH;
}

static PKStatus
pkcDoUpload (PKClient *client, const PKChar *event, const PKChar *path)
{
  PKChar id[64];
  if (pkMediaUpload (client, event, path, id, sizeof (id)) != PK_OK)
    {
      pkcReport (client, "Failed to upload media");
      return pkError (client)->status;
    }

  PKC_INFO ("%s", id);

  return PK_OK;
}

/* Runs a command that works on the user's events. */
static PKInt
pkcDoCommand (PKClient *client, const PKCArg *args)
{
  if (!pkLoginValid (client))
    {
      PKC_ERROR ("Not logged in, please login first");
      return PK_EAUTH;
    }

  PKBool owner = (args->flags & PKC_FOWN) ? PK_TRUE : PK_FALSE;
  PKBool favorite = (args->flags & PKC_FFAV) ? PK_TRUE : PK_FALSE;
  PKCArgFmt fmt = PKC_ARGFMT (args->flags);
  switch (args->cmd)
    {
    case PKC_CSYNC:
      return pkcSync (client, args->c.sync.target, owner, favorite);
    case PKC_CLIST:
      return pkcList (client, args->c.list.query, owner, favorite,
                      (args->flags & PKC_FCON) ? PK_TRUE : PK_FALSE, fmt);
    case PKC_CCREATE:
      return pkcCreate (client, args->c.create.name, fmt);
    case PKC_CUPDATE:
      return pkcUpdate (client, args->c.update.event, args->c.update.name,
                        args->c.update.from, args->c.update.to,
                        PKC_ARGPUB (args->flags), PKC_ARGFAV (args->flags),
                        PKC_ARGMOD (args->flags));
    case PKC_CQR:
      return pkcQr (client, args->c.qr.event, args->c.qr.out,
                    fmt == PKC_FMTSVG);
    case PKC_CLINK:
      return pkcLink (client, args->c.link.event);
    case PKC_CMEDIA:
      return pkcMediaList (client, args->c.media.event, owner, favorite, fmt);
    case PKC_CDOWNLOAD:
      if (args->c.download.media)
        {
          return pkcDownloadSingle (client, args->c.download.event,
                                    args->c.download.media,
                                    args->c.download.target);
        }
      return pkcDownloadAll (client, args->c.download.event,
                             args->c.download.target, owner, favorite);
    case PKC_CUPLOAD:
      return pkcDoUpload (client, args->c.upload.event, args->c.upload.source);
    case PKC_CEDIT:
      return pkcEdit (client, args->c.edit.event, args->c.edit.media,
                      PKC_ARGFAV (args->flags));
    case PKC_CAPPROVE:
      return pkcApprove (client, args->c.approve.event, args->c.approve.media,
                         (args->flags & PKC_FREVOKE) ? PK_FALSE : PK_TRUE);
    case PKC_CCARD:
      return pkcCard (
          client, args->c.card.event, args->c.card.out, args->c.card.design,
          args->c.card.locale, args->c.card.layout, args->c.card.paper,
          fmt == PKC_FMTJPG, (args->flags & PKC_FNOBG) ? PK_TRUE : PK_FALSE);
    case PKC_CJOIN:
      return pkcJoin (client, args->c.join.invite, fmt);
    case PKC_CTRUSTED:
      return pkcTrusted (client, args->c.trusted.event, args->c.trusted.add,
                         args->c.trusted.adds, args->c.trusted.remove,
                         args->c.trusted.removes,
                         (args->flags & PKC_FCLEAR) ? PK_TRUE : PK_FALSE, fmt);
    default:
      PKC_ERROR ("Unknown command");
      return PK_EARG;
    }
}

static PKInt
pkcDoVersion (void)
{
  PKC_INFO ("p6a v" PK_VERSION_STRING);
  return PK_OK;
}

static PKInt
pkcDoHelp (void)
{
  pkcPrintLogo ();

  PKC_INFO ("Usage: p6a <command> [options]");
  PKC_INFO ("");
  PKC_INFO ("Commands:");
  PKC_INFO ("  login               Setup connection with Partoska");
  PKC_INFO ("  logout              Remove authentication credentials");
  PKC_INFO ("  status              Tell whether you are logged in");
  PKC_INFO ("  sync                Synchronize photos from Partoska");
  PKC_INFO ("  list                List events");
  PKC_INFO ("  create              Create a new event");
  PKC_INFO ("  update              Update an existing event");
  PKC_INFO ("  join                Join an event with an invite code or link");
  PKC_INFO ("  trusted             Show or change trusted guests");
  PKC_INFO ("  edit                Update a media item");
  PKC_INFO (
      "  approve             Approve or revoke approval of a media item");
  PKC_INFO ("  qr                  Download QR code for an event");
  PKC_INFO (
      "  card                Download share card (PDF/JPG) for an event");
  PKC_INFO ("  link                Get invite link for an event");
  PKC_INFO ("  media               List media items for an event");
  PKC_INFO ("  download            Download media for an event");
  PKC_INFO ("  upload              Upload a media file to an event");
  PKC_INFO ("  help                Display this help message");
  PKC_INFO ("  version             Print version information");
  PKC_INFO ("");
  PKC_INFO ("Login options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -i, --import <file> Import configuration from file");
  PKC_INFO ("  -1                  Print the login URL only (same as -F one)");
  PKC_INFO (
      "  -F, --format <fmt>  Output format: plain (default), json, csv, one");
  PKC_INFO ("");
  PKC_INFO ("Logout options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("");
  PKC_INFO ("Status options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -1                  Print true or false only (same as -F one)");
  PKC_INFO (
      "  -F, --format <fmt>  Output format: plain (default), json, csv, one");
  PKC_INFO ("");
  PKC_INFO ("Synchronize options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -t, --target <path> Target directory for sync (required)");
  PKC_INFO ("  -o, --owner-only    Synchronize only events owned by user");
  PKC_INFO ("  -f, --favorite-only Synchronize only favorite events");
  PKC_INFO ("");
  PKC_INFO ("List events options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -q, --query <text>  Filter events by name");
  PKC_INFO ("  -o, --owner-only    List only events owned by user");
  PKC_INFO ("  -f, --favorite-only List only favorite events");
  PKC_INFO ("  -c, --contributor-only");
  PKC_INFO ("                      List only events you contribute to");
  PKC_INFO (
      "  -1                  Print IDs only, one per line (same as -F one)");
  PKC_INFO (
      "  -F, --format <fmt>  Output format: plain (default), json, csv, one");
  PKC_INFO ("");
  PKC_INFO ("Create event options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -n, --name <name>   Event name, 3-48 characters (required)");
  PKC_INFO ("  -1                  Print ID only (same as -F one)");
  PKC_INFO (
      "  -F, --format <fmt>  Output format: plain (default), json, csv, one");
  PKC_INFO ("");
  PKC_INFO ("Update event options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -e, --event <id>    Event ID (required)");
  PKC_INFO ("  -n, --name <name>   New event name, 3-48 characters");
  PKC_INFO ("  -S, --from <dt>     Event start date-time (ISO 8601)");
  PKC_INFO ("  -E, --to <dt>       Event end date-time (ISO 8601)");
  PKC_INFO ("  -p, --public        Make event public");
  PKC_INFO ("  -P, --private       Make event private");
  PKC_INFO ("  -f, --favorite      Mark event as favorite");
  PKC_INFO ("  -F, --no-favorite   Unmark event as favorite");
  PKC_INFO ("  -m, --moderated     Enable upload moderation");
  PKC_INFO ("  -M, --no-moderated  Disable upload moderation");
  PKC_INFO ("");
  PKC_INFO ("Join event options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -i, --invite <code> Invite code or link (required)");
  PKC_INFO ("  -1                  Print event ID only (same as -F one)");
  PKC_INFO (
      "  -F, --format <fmt>  Output format: plain (default), json, csv, one");
  PKC_INFO ("");
  PKC_INFO ("Trusted guests options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -e, --event <id>    Event ID (required)");
  PKC_INFO ("  -a, --add <ent>     Add emails or @domains (repeatable)");
  PKC_INFO ("  -r, --remove <ent>  Remove emails or @domains (repeatable)");
  PKC_INFO ("  -c, --clear         Empty the list first");
  PKC_INFO ("  -1                  Print entries only, one per line (same as "
            "-F one)");
  PKC_INFO (
      "  -F, --format <fmt>  Output format: plain (default), json, csv, one");
  PKC_INFO ("");
  PKC_INFO ("Edit media options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -e, --event <id>    Event ID (required)");
  PKC_INFO ("  -m, --media <id>    Media ID (required)");
  PKC_INFO ("  -f, --favorite      Mark media as favorite");
  PKC_INFO ("  -F, --no-favorite   Unmark media as favorite");
  PKC_INFO ("");
  PKC_INFO ("Approve media options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -e, --event <id>    Event ID (required)");
  PKC_INFO ("  -m, --media <id>    Media ID (required)");
  PKC_INFO ("  -r, --revoke        Revoke approval (put media back on hold)");
  PKC_INFO ("");
  PKC_INFO ("QR code options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -e, --event <id>    Event ID (required)");
  PKC_INFO (
      "  -t, --target <file> Output file path (default: <id>-qr.png/svg)");
  PKC_INFO ("  -F, --format <fmt>  Output format: png (default), svg");
  PKC_INFO ("");
  PKC_INFO ("Share card options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -e, --event <id>    Event ID (required)");
  PKC_INFO (
      "  -d, --design <name> Card design: bday, tech, match, forest, garden, "
      "gold, romantic, silver, neon, nineties, cottage, wish, velvet, "
      "hallows (required)");
  PKC_INFO (
      "  -t, --target <file> Output file path (default: <id>-card.pdf/jpg)");
  PKC_INFO (
      "  -l, --locale <loc>  Language: en (default), cs, sk, pl, ru, es, de, "
      "fr, it, nl, uk");
  PKC_INFO ("  -L, --layout <lay>  Layout: single (default), business");
  PKC_INFO ("  -p, --paper <pap>   Paper size: a4 (default), letter");
  PKC_INFO ("  -F, --format <fmt>  Output format: pdf (default), jpg");
  PKC_INFO ("  -b, --no-background White background (omits colour fill)");
  PKC_INFO ("");
  PKC_INFO ("Invite link options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -e, --event <id>    Event ID (required)");
  PKC_INFO ("");
  PKC_INFO ("List media options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -e, --event <id>    Event ID (required)");
  PKC_INFO ("  -o, --owner-only    List only media uploaded by user");
  PKC_INFO ("  -f, --favorite-only List only favorited media");
  PKC_INFO (
      "  -1                  Print IDs only, one per line (same as -F one)");
  PKC_INFO (
      "  -F, --format <fmt>  Output format: plain (default), json, csv, one");
  PKC_INFO ("");
  PKC_INFO ("Download media options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -e, --event <id>    Event ID (required)");
  PKC_INFO ("");
  PKC_INFO ("  All-media mode (mutually exclusive with single-media):");
  PKC_INFO ("  -t, --target <path> Target directory (required)");
  PKC_INFO ("  -o, --owner-only    Download only media uploaded by user");
  PKC_INFO ("  -f, --favorite-only Download only favorited media");
  PKC_INFO ("");
  PKC_INFO ("  Single-media mode (mutually exclusive with all-media):");
  PKC_INFO ("  -m, --media <id>    Media ID (required)");
  PKC_INFO ("  -t, --target <file> Output file path (required)");
  PKC_INFO ("");
  PKC_INFO ("Upload media options:");
  PKC_INFO (
      "  -D, --dir <path>    Specify working directory (default: ~/.p6a)");
  PKC_INFO ("  -e, --event <id>    Event ID (required)");
  PKC_INFO ("  -s, --source <file> Path to local media file (required)");
  PKC_INFO ("");
  PKC_INFO ("Examples:");
  PKC_INFO ("  p6a login");
  PKC_INFO ("  p6a login -F json");
  PKC_INFO ("  p6a status || p6a login");
  PKC_INFO ("  p6a status -F json | jq .login");
  PKC_INFO ("  p6a list");
  PKC_INFO ("  p6a list -q \"Birthday\" --favorite-only");
  PKC_INFO ("  p6a list -F json | jq '.[].name'");
  PKC_INFO ("  p6a list -1 | xargs -I{} p6a qr -e {}");
  PKC_INFO ("  p6a create -n \"Birthday Party\"");
  PKC_INFO (
      "  p6a update -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -n \"New Name\"");
  PKC_INFO ("  p6a join -i \"6024 8910 2150 4542 9CCA B150 959F 1FEE\"");
  PKC_INFO ("  p6a trusted -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7");
  PKC_INFO ("  p6a trusted -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 "
            "-a @example.com -a anna@firm.cz");
  PKC_INFO ("  p6a sync -t ./photos -f");
  PKC_INFO ("  p6a sync --target ./media --owner-only");
  PKC_INFO ("  p6a qr -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7");
  PKC_INFO ("  p6a qr --event 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -F svg -t "
            "qr.svg");
  PKC_INFO ("  p6a card -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -d bday");
  PKC_INFO (
      "  p6a card -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -d tech -F jpg "
      "-t card.jpg");
  PKC_INFO ("  p6a link -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7");
  PKC_INFO ("  p6a media -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7");
  PKC_INFO ("  p6a media -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -F json");
  PKC_INFO (
      "  p6a download -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -t ./photos");
  PKC_INFO (
      "  p6a download -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -t ./photos "
      "--owner-only");
  PKC_INFO ("  p6a download -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 "
            "-m 7a8b9c0d-f00d-4a3b-8c5d-e6f700010203 -t photo.jpg");
  PKC_INFO (
      "  p6a upload -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -s photo.jpg");
  PKC_INFO ("  p6a edit -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 "
            "-m 7a8b9c0d-f00d-4a3b-8c5d-e6f700010203 -f");
  PKC_INFO ("  p6a edit -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 "
            "-m 7a8b9c0d-f00d-4a3b-8c5d-e6f700010203 -F");
  PKC_INFO ("  p6a approve -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 "
            "-m 7a8b9c0d-f00d-4a3b-8c5d-e6f700010203");
  PKC_INFO ("  p6a approve -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 "
            "-m 7a8b9c0d-f00d-4a3b-8c5d-e6f700010203 -r");
  PKC_INFO ("  p6a logout");
  PKC_INFO ("");

  return PK_OK;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKInt
main (PKInt argc, PKChar **argv)
{
  pkcConsoleInit ();

  PKCArg args;
  if (pkcArgParse (&args, argc, argv) != PK_OK)
    {
      PKC_ERROR ("Invalid program parameters, use -h for help");
      return PKC_EXIT_FAILURE;
    }

  switch (args.cmd)
    {
    case PKC_CHELP:
      return pkcDoHelp () == PK_OK ? PKC_EXIT_SUCCESS : PKC_EXIT_FAILURE;
    case PKC_CVERSION:
      return pkcDoVersion () == PK_OK ? PKC_EXIT_SUCCESS : PKC_EXIT_FAILURE;
    default:
      break;
    }

  if (pkcValidate (&args) != PK_OK)
    {
      return PKC_EXIT_FAILURE;
    }

  if (pkGlobalInit () != PK_OK)
    {
      PKC_ERROR ("Failed to initialize the network library");
      return PKC_EXIT_FAILURE;
    }

  PKInt result = PK_EMEM;
  PKClient *client = pkcOpen (args.dir);
  if (client)
    {
      pkcClient = client;
      pkcSignals (pkcOnSignal);

      switch (args.cmd)
        {
        case PKC_CLOGIN:
          result = pkcDoLogin (client, args.c.login.path,
                               PKC_ARGFMT (args.flags));
          break;
        case PKC_CLOGOUT:
          result = pkcDoLogout (client);
          break;
        case PKC_CSTATUS:
          result = pkcDoStatus (client, PKC_ARGFMT (args.flags));
          break;
        default:
          result = pkcDoCommand (client, &args);
          break;
        }

      pkcSignals (NULL);
      pkcClient = NULL;
      pkDestroy (client);
    }

  pkGlobalCleanup ();

  return result == PK_OK ? PKC_EXIT_SUCCESS : PKC_EXIT_FAILURE;
}
