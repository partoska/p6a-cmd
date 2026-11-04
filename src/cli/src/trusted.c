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

#include "trusted.h"
#include "list.h"
#include "log.h"
#include "partoska.h"
#include "pktypes.h"
#include "plat.h"
#include "report.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Macros
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define ENTRY_MAX (256)
#define ESC_MAX (512)
#define SEPARATORS " \t\r\n,;"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Types
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct PKCEntry
{
  PKChar text[ENTRY_MAX];
} PKCEntry;

/* The list being edited. */
/* Entries for one request, deduplicated, with pointers to pass on. */
typedef struct PKCEntries
{
  PKCEntry *items;
  const PKChar **texts;
  PKSize count;
  PKSize capacity;
} PKCEntries;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Private
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static PKBool
pkcIsSeparator (PKChar c)
{
  for (const PKChar *s = SEPARATORS; *s; ++s)
    {
      if (*s == c)
        {
          return PK_TRUE;
        }
    }

  return PK_FALSE;
}

/*
 * Takes the next entry of a value, normalized as the server does it:
 * lowercase, a domain (example.com, *@example.com) as @example.com.
 * Returns the position after it, or NULL when no entry is left; an entry
 * too long leaves out empty.
 */
static const PKChar *
pkcNextEntry (const PKChar *value, PKChar *out)
{
  while (*value && pkcIsSeparator (*value))
    {
      ++value;
    }
  if (!*value)
    {
      return NULL;
    }

  const PKChar *start = value;
  while (*value && !pkcIsSeparator (*value))
    {
      ++value;
    }
  PKSize length = (PKSize)(value - start);
  if (length >= 2 && start[0] == '*' && start[1] == '@')
    {
      ++start;
      --length;
    }

  PKBool domain = PK_TRUE;
  for (PKSize i = 0; i < length; ++i)
    {
      domain = domain && start[i] != '@';
    }
  if (length + (domain ? 1 : 0) >= ENTRY_MAX)
    {
      out[0] = '\0';
      return value;
    }

  PKSize n = 0;
  if (domain)
    {
      out[n++] = '@';
    }
  for (PKSize i = 0; i < length; ++i)
    {
      PKChar c = start[i];
      out[n++] = (c >= 'A' && c <= 'Z') ? (PKChar)(c - 'A' + 'a') : c;
    }
  out[n] = '\0';

  return value;
}

/* Counts the entries of values, an upper bound of what they add. */
static PKSize
pkcCountEntries (const PKChar *const *values, PKSize count)
{
  PKSize total = 0;
  PKChar entry[ENTRY_MAX];
  for (PKSize i = 0; i < count; ++i)
    {
      for (const PKChar *v = values[i]; (v = pkcNextEntry (v, entry));)
        {
          ++total;
        }
    }

  return total;
}

static PKSize
pkcFindEntry (const PKCEntries *entries, const PKChar *text)
{
  for (PKSize i = 0; i < entries->count; ++i)
    {
      if (pkcStrEq (entries->items[i].text, text))
        {
          return i;
        }
    }

  return entries->count;
}

static PKVoid
pkcEntriesFree (PKCEntries *entries)
{
  pkcMemFree (entries->items);
  pkcMemFree ((PKVoid *)entries->texts);
  *entries = (PKCEntries){ 0 };
}

/* Makes room for capacity entries. */
static PKInt
pkcEntriesInit (PKCEntries *entries, PKSize capacity)
{
  *entries = (PKCEntries){ 0 };
  PKSize slots = capacity ? capacity : 1;
  entries->items = pkcMemZalloc (slots * sizeof (PKCEntry));
  entries->texts = pkcMemZalloc (slots * sizeof (PKChar *));
  if (!entries->items || !entries->texts)
    {
      PKC_ERROR ("Out of memory");
      pkcEntriesFree (entries);
      return PK_EMEM;
    }
  entries->capacity = capacity;

  return PK_OK;
}

/* Appends an entry unless it is there already. */
static PKVoid
pkcEntriesPut (PKCEntries *entries, const PKChar *text)
{
  if (pkcFindEntry (entries, text) < entries->count
      || entries->count >= entries->capacity)
    {
      return;
    }
  PKCEntry *entry = &entries->items[entries->count];
  pkcStrFmt (entry->text, ENTRY_MAX, "%s", text);
  entries->texts[entries->count++] = entry->text;
}

/* Collects the entries of option values; PK_EARG for one too long. */
static PKInt
pkcEntriesParse (PKCEntries *entries, const PKChar *const *values,
                 PKSize count)
{
  PKInt result = pkcEntriesInit (entries, pkcCountEntries (values, count));
  PKChar entry[ENTRY_MAX];
  for (PKSize i = 0; result == PK_OK && i < count; ++i)
    {
      for (const PKChar *v = values[i]; (v = pkcNextEntry (v, entry));)
        {
          if (!entry[0])
            {
              PKC_ERROR ("Trusted entry too long");
              pkcEntriesFree (entries);
              return PK_EARG;
            }
          pkcEntriesPut (entries, entry);
        }
    }

  return result;
}

/* Prints the list: JSON is an array of strings, as list prints events. */
static PKVoid
pkcPrintTrusted (const PKStringList *list, PKCArgFmt fmt)
{
  PKChar esc[ESC_MAX];
  switch (fmt)
    {
    case PKC_FMTJSON:
      if (!list->count)
        {
          PKC_INFO ("[]");
          break;
        }
      PKC_INFO ("[");
      for (PKSize i = 0; i < list->count; ++i)
        {
          pkcJsonEsc (esc, sizeof (esc), list->items[i]);
          PKC_INFO ("  \"%s\"%s", esc, i + 1 < list->count ? "," : "");
        }
      PKC_INFO ("]");
      break;
    case PKC_FMTCSV:
      PKC_INFO ("entry");
      for (PKSize i = 0; i < list->count; ++i)
        {
          pkcCsvQuote (esc, sizeof (esc), list->items[i]);
          PKC_INFO ("%s", esc);
        }
      break;
    case PKC_FMTONE:
      for (PKSize i = 0; i < list->count; ++i)
        {
          PKC_INFO ("%s", list->items[i]);
        }
      break;
    default:
      for (PKSize i = 0; i < list->count; ++i)
        {
          PKC_INFO ("%s", list->items[i]);
        }
      if (list->count)
        {
          PKC_INFO ("");
        }
      PKC_INFO ("%u trusted entry(ies)", (PKDword)list->count);
      break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Definitions - Public
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PKInt
pkcTrusted (PKClient *client, const PKChar *event, const PKChar *const *add,
            PKSize adds, const PKChar *const *remove, PKSize removes,
            PKBool clear, PKCArgFmt fmt)
{
  if (!client || !event)
    {
      PKC_ERROR ("Client, and/or event are invalid");
      return PK_EARG;
    }

  // Reading is needed only to show the list or to clear it; additions and
  // removals are each one request, applied by the server in one step.
  PKStringList list = { 0 };
  if ((clear || !(adds || removes))
      && pkEventTrusted (client, event, &list) != PK_OK)
    {
      pkcReport (client, "Failed to fetch the trusted list");
      return pkError (client)->status;
    }
  if (!clear && !adds && !removes)
    {
      pkcPrintTrusted (&list, fmt);
      pkStringListFree (&list);
      return PK_OK;
    }

  // A clear removes everything but what is added again right after, so
  // those entries never leave the list.
  PKCEntries adding;
  PKCEntries removing;
  PKInt result = pkcEntriesParse (&adding, add, adds);
  if (result == PK_OK && clear)
    {
      result = pkcEntriesInit (&removing, list.count);
      for (PKSize i = 0; result == PK_OK && i < list.count; ++i)
        {
          if (pkcFindEntry (&adding, list.items[i]) == adding.count)
            {
              pkcEntriesPut (&removing, list.items[i]);
            }
        }
    }
  else if (result == PK_OK)
    {
      result = pkcEntriesParse (&removing, remove, removes);
    }
  if (result != PK_OK)
    {
      pkcEntriesFree (&adding);
      pkStringListFree (&list);
      return result;
    }

  // Removals first, then additions; each answer is the resulting list.
  if (removing.count)
    {
      pkStringListFree (&list);
      result = pkEventTrustedRemove (client, event, removing.texts,
                                     removing.count, &list);
      if (result != PK_OK)
        {
          pkcReport (client, "Failed to remove trusted entries");
        }
    }
  if (result == PK_OK && adding.count)
    {
      pkStringListFree (&list);
      result = pkEventTrustedAdd (client, event, adding.texts, adding.count,
                                  &list);
      if (result != PK_OK)
        {
          pkcReport (client, "Failed to add trusted entries");
        }
    }

  // Values without entries change nothing; show the list as it is.
  if (result == PK_OK && !clear && !removing.count && !adding.count
      && pkEventTrusted (client, event, &list) != PK_OK)
    {
      pkcReport (client, "Failed to fetch the trusted list");
      result = pkError (client)->status;
    }
  if (result == PK_OK)
    {
      pkcPrintTrusted (&list, fmt);
    }

  pkcEntriesFree (&adding);
  pkcEntriesFree (&removing);
  pkStringListFree (&list);

  return result;
}
