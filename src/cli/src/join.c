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

#include "join.h"
#include "list.h"
#include "log.h"
#include "partoska.h"
#include "pktypes.h"
#include "report.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define LINK_MAX (512)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Tells whether the user may upload to a joined event. */
static const PKChar *
pkcAccess (const PKEvent *event)
{
  if (event->owner)
    {
      return "you administer this event";
    }
  if (event->contributor)
    {
      return "you can upload";
    }
  if (event->pub)
    {
      return "you can upload (public event)";
    }

  // Joining a private event without access asks the organizer for it;
  // until then its photos are hidden too.
  return "waiting for the organizer to let you in";
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKInt
pkcJoin (PKClient *client, const PKChar *invite, PKCArgFmt fmt)
{
  if (!client || !invite)
    {
      PKC_ERROR ("Client, and/or invite are invalid");
      return PK_EARG;
    }

  PKEvent event;
  PKChar link[LINK_MAX];
  PKInt result = pkEventJoin (client, invite, &event, link, sizeof (link));
  if (result != PK_OK)
    {
      pkcReport (client, "Failed to join event");
      return result;
    }

  pkcPrintEventHeader (fmt);
  pkcPrintEventRow (&event, fmt, PK_TRUE);
  pkcPrintEventFooter (fmt);
  if (fmt == PKC_FMTPLAIN)
    {
      PKC_INFO ("");
      PKC_INFO ("Invite link: %s", link);
      PKC_INFO ("Access:      %s", pkcAccess (&event));
    }

  return PK_OK;
}
