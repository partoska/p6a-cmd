<p align="center">
  <img src="img/logo.svg" width="160" alt="p6a logo">
</p>

# Partoska Command-Line Interface (`p6a`)

Manage and sync your event photos from [Partoska](https://www.partoska.com) — right from your terminal.

[![Version](https://img.shields.io/badge/version-2.1.0-blue.svg)](https://github.com/partoska/p6a-cmd/releases/tag/v2.1.0) [![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE.txt) [![Build](https://github.com/partoska/p6a-cmd/actions/workflows/build.yml/badge.svg)](https://github.com/partoska/p6a-cmd/actions/workflows/build.yml)
[![npm](https://img.shields.io/npm/v/@partoska/p6a.svg)](https://www.npmjs.com/package/@partoska/p6a)

## What is this?

`p6a` is a lightweight command-line tool that helps you download and organize your event photos from Partoska. It's perfect for backing up your memories or working with your photos offline.

It is built on `libpartoska`, a small C library for the Partoska API that you can use in your own programs too; see [Library](#library).

**Key features:**

- Simple authentication via OAuth.
- Smart sync that doesn't re-download existing files.
- Automatic folder organization by event and date.
- Filter by events you own or have favorited.
- Upload photos directly to an event from the command line.
- Get or download invite links and QR codes for sharing events.
- Works on macOS, Linux, and Windows.
- [Agent skill](#agent-skill) included for use with AI assistants.
- [C library](#library) (`libpartoska`) for your own programs.

## Quick Start

### 1. Install

**Via Homebrew** (macOS/Linux):

```bash
brew install partoska/tap/p6a
```

**Via npm**:

```bash
npm install -g @partoska/p6a
```

**Via Scoop** (Windows):

```powershell
scoop bucket add partoska https://github.com/partoska/scoop-bucket
scoop install p6a
```

Or run without installing:

```bash
npx @partoska/p6a login
npx @partoska/p6a sync -t ~/my-photos
```

**From source** (requires CMake 3.20+, GCC or Clang, libcurl, and [Bikini](https://github.com/partoska/bikini) 2.x):

```bash
# Clone the repository.
git clone https://github.com/partoska/p6a-cmd.git
cd p6a-cmd

# Build and install the tool.
./build.sh
sudo ./install.sh

# The binary will be at: build/src/cli/p6a
```

### 2. Login

```bash
p6a login
```

You'll see a link and a short code. Open the link in a browser on this or any other device, check that it shows the same code, and allow access.

### 3. Sync Your Photos

```bash
p6a sync -t ~/my-photos
```

That's it! Your photos will be organized into folders like:

```
my-photos/
├── 2025-01-05 Birthday Party/
│   ├── IMG_0001.jpg
│   ├── IMG_0002.jpg
│   └── MOV_0001.mp4
└── 2025-02-14 Wedding/
    ├── IMG_0001.jpg
    └── IMG_0002.jpg
```

## Commands

| Command    | Description                                |
| ---------- | ------------------------------------------ |
| `login`    | Authenticate with Partoska                 |
| `logout`   | Remove saved credentials                   |
| `status`   | Tell whether you are logged in             |
| `sync`     | Download and organize event photos         |
| `list`     | List events                                |
| `create`   | Create a new event                         |
| `update`   | Update an existing event                   |
| `join`     | Join an event with an invite code or link  |
| `trusted`  | Show or change trusted guests              |
| `edit`     | Update a media item                        |
| `approve`  | Approve or revoke approval of a media item |
| `qr`       | Download QR code for an event              |
| `card`     | Download share card for an event           |
| `link`     | Get invite link for an event               |
| `media`    | List media items for an event              |
| `download` | Download media for an event                |
| `upload`   | Upload a photo to an event                 |
| `help`     | Show usage information                     |
| `version`  | Print version information                  |

### `login` - Authenticate with Partoska

Connect your Partoska account.

```bash
p6a login
```

The tool prints a link and a short code such as `BCDF-GHJK`. Open the link in
a browser on this or any other device, such as your phone, check that it shows
the same code, and allow access. The tool waits until you do, then saves the
login. The code expires after 15 minutes.

**Options:**

- `-D, --dir <path>` - Use a custom config directory (default: `~/.p6a`).
- `-i, --import <file>` - Import config from an INI file.
- `-1` - Print the login URL only (same as `-F one`).
- `-F, --format <fmt>` - Output format: `plain` (default), `json`
  (`{"uri":"…","code":"BCDF-GHJK","expires":900}`), `csv` (a
  `uri,code,expires` header and one row), `one` (the URL only). Other formats
  than `plain` print only the link and code, as soon as they are known, for a
  script or an agent to pass on; the exit code tells whether the login
  succeeded. The URL already carries the code.

**Examples:**

```bash
# Standard login.
p6a login

# Use custom config location.
p6a login --dir /data/p6a-config

# Import configuration from file.
p6a login --import /path/to/config.ini

# Machine-readable link and code.
p6a login -F json
```

### `status` - Check the Login

Tell whether you are logged in: prints `Logged in` and exits with 0, or prints
`Not logged in` and exits with 1, so scripts can test it. The exit code is the
same in every output format. It only looks at the saved login and does not
contact Partoska; a login that Partoska no longer accepts shows up on the next
command.

```bash
p6a status
```

**Options:**

- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).
- `-1` - Print `true` or `false` only (same as `-F one`).
- `-F, --format <fmt>` - Output format: `plain` (default), `json`
  (`{"login":true}`), `csv` (a `login` header, then `true` or `false`), `one`
  (`true` or `false`).

**Examples:**

```bash
# Log in only when needed.
p6a status >/dev/null || p6a login

# Machine-readable answer.
p6a status -F json | jq .login

# Check a custom config location.
p6a status --dir /data/p6a-config
```

### `sync` - Download Your Photos

Sync events and photos from Partoska to your computer.

```bash
p6a sync -t <target-directory>
```

Events whose photos you may not see yet are skipped with a warning, and the rest still sync. That happens with a private event you joined while the organizer hasn't let you in yet. Folders synced earlier are kept.

**Options:**

- `-t, --target <path>` - Where to save photos (required).
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).
- `-o, --owner-only` - Only sync events you created.
- `-f, --favorite-only` - Only sync your favorite events.

**Examples:**

```bash
# Sync all events.
p6a sync -t ~/photos

# Sync only events you own.
p6a sync -t ~/my-events --owner-only

# Sync only favorites.
p6a sync -t ~/favorites --favorite-only

# Combine filters (own AND favorite).
p6a sync -t ~/my-favorites -o -f
```

**What happens when you sync:**

- Events are organized into folders named `YYYY-MM-DD Event Name`.
- Photos are named sequentially (`IMG_0001.jpg`, `IMG_0002.jpg`, etc.).
- Videos are named with `MOV_` prefix (`MOV_0001.mp4`).
- Already-downloaded files are automatically skipped.
- If event names change, folders are automatically renamed.

### `list` - List Events

List events the authenticated user has access to.

```bash
p6a list
```

**Options:**

- `-q, --query <text>` - Filter events by name.
- `-o, --owner-only` - List only events you own.
- `-f, --favorite-only` - List only favorite events.
- `-c, --contributor-only` - List only events where you have the contributor role, e.g. private events you joined and were let in to upload to. Filters combine: `-o -c` lists your own events you also contribute to.
- `-1` - Print IDs only, one per line (shorthand for `-F one`).
- `-F, --format <fmt>` - Output format: `plain` (default), `json`, `csv`, `one`.
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

**Examples:**

```bash
# List all events.
p6a list

# Search by name.
p6a list -q "Birthday"

# Only your own events.
p6a list --owner-only

# Only favorites matching a query.
p6a list -q "Wedding" --favorite-only

# Only events you may upload to as a contributor.
p6a list --contributor-only

# Print event IDs only.
p6a list -1

# Extract event names using jq.
p6a list -F json | jq '.[].name'

# Download QR codes for all your events.
p6a list -o -1 | xargs -I{} p6a qr -e {}
```

**Default output columns:**

```
NAME                              ID                                    FROM        TO          OWN CON FAV PUB MOD  GUESTS  MEDIA
Birthday Party                    12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7  2026-01-05  2026-01-06  yes  no yes yes  no       5     42
```

`OWN` tells that you own the event, `CON` that you have the contributor role in it (may upload to a private event). In JSON these are `owner` and `contributor`; in CSV, `contributor` is the last column. Servers that predate contributor roles report `false`.

### `create` - Create a New Event

Create a new event on Partoska.

```bash
p6a create -n "Event Name"
```

**Options:**

- `-n, --name <name>` - Event name, 3–48 characters (required).
- `-1` - Print ID only (shorthand for `-F one`).
- `-F, --format <fmt>` - Output format: `plain` (default), `json`, `csv`, `one`.
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

**Examples:**

```bash
p6a create -n "Birthday Party"
p6a create --name "Summer Wedding 2026"
p6a create -n "Birthday Party" -F json | jq '.id'
```

On success, the created event is printed in the same table format as `list`:

```
NAME                              ID                                    FROM        TO          OWN CON FAV PUB MOD  GUESTS  MEDIA
Birthday Party                    12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7  2026-06-01  2026-06-01  yes yes  no yes  no       1      0

1 event(s)
```

### `update` - Update an Event

Modify an existing event's name, dates, or visibility.

```bash
p6a update -e <id> [options]
```

At least one field to update must be provided.

**Options:**

- `-e, --event <id>` - Event ID (required).
- `-n, --name <name>` - New event name, 3–48 characters.
- `-S, --from <dt>` - Event start date-time (ISO 8601).
- `-E, --to <dt>` - Event end date-time (ISO 8601).
- `-p, --public` - Make the event public.
- `-P, --private` - Make the event private.
- `-f, --favorite` - Mark the event as a favorite.
- `-F, --no-favorite` - Unmark the event as a favorite.
- `-m, --moderated` - Enable upload moderation (new uploads require approval before becoming visible).
- `-M, --no-moderated` - Disable upload moderation.
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

**Examples:**

```bash
# Rename an event.
p6a update -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -n "New Name"

# Update the event dates.
p6a update -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -S 2026-06-01T10:00:00Z -E 2026-06-01T22:00:00Z

# Make an event private.
p6a update -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 --private

# Rename and make public in one call.
p6a update -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -n "Summer Party" --public
```

### `join` - Join an Event

Join an event as a guest with an invite code or link, e.g. one read off a printed share card. Accepts every format the invite page does: a UUID, the grouped printed code (with or without the `PARTOSKA.COM` prefix; common OCR mistakes are corrected), a base64 code, a Partoska `/i/` or `/q/` link, or an `app.partoska.com` link with an `invite` parameter. Joining keeps any roles you already have, and joining again is harmless.

In a private event you become a contributor straight away when your email or its domain is on the event's [trusted list](#trusted---trusted-guests); otherwise the organizer gets a request to let you in, and until they approve it the event's photos stay hidden from you too (`media` and `download` fail with HTTP 403, `sync` skips the event). In a public event every guest can see and upload photos.

```bash
p6a join -i <code-or-link>
```

**Options:**

- `-i, --invite <code>` - Invite code or link (required). Quote it when it holds spaces.
- `-1` - Print the event ID only (same as `-F one`).
- `-F, --format <fmt>` - Output format: `plain` (default), `json`, `csv`, `one`.
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

**Examples:**

```bash
# Join with the code printed on a share card.
p6a join -i "6024 8910 2150 4542 9CCA B150 959F 1FEE"

# Join with an invite link and keep the event ID for later commands.
EID=$(p6a join -i https://partoska.com/i/YCSJEC... -1)
p6a media -e "$EID"
```

The joined event is printed as `list` and `create` print events, in every format; plain output adds the invite link and whether you may upload:

```
NAME                              ID                                    FROM        TO          OWN CON FAV PUB MOD  GUESTS  MEDIA
Summer BBQ                        12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7  2026-06-01  2026-06-01   no  no  no  no yes      12     40

Invite link: https://partoska.com/i/YCSJEC...
Access:      waiting for the organizer to let you in
```

In a private event, `"contributor": false` (and `"owner": false`) after joining means the organizer still has to approve your request to upload. A revoked or unknown invite fails with HTTP 404, a ban with HTTP 403.

### `trusted` - Trusted Guests

Show or change the trusted list of a private event: emails and whole domains whose guests become contributors as soon as they join, without waiting for your approval. A domain covers only that exact domain, never its subdomains. Requires admin permission.

```bash
p6a trusted -e <id> [-c] [-r <entry>]... [-a <entry>]...
```

Without changes, the command prints the saved list. With changes, it empties the list (`-c`), removes entries (`-r`), adds entries (`-a`) and prints the list the server saved. Removals and additions each take one request, applied by the server in one step, so two organizers changing the list at the same time don't overwrite each other. Removing an entry that is not on the list is ignored. `-c` reads the list first and removes every entry except the ones it adds again, so those stay on the list throughout. Entries are lowercased, and domains are written `@example.com` (`example.com` and `*@example.com` mean the same). Every value may hold several entries separated by commas, semicolons or spaces. Adding entries also approves at once the pending requests of guests the list matches. Removing an entry does not take away access already granted. The list holds at most 200 entries.

**Options:**

- `-e, --event <id>` - Event ID (required).
- `-a, --add <entry>` - Add emails or domains; may be repeated.
- `-r, --remove <entry>` - Remove emails or domains; may be repeated.
- `-c, --clear` - Empty the list first (alone, it clears the list).
- `-1` - Print the entries only, one per line (same as `-F one`).
- `-F, --format <fmt>` - Output format: `plain` (default), `json`, `csv`, `one`.
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

**Examples:**

```bash
# Show the list.
p6a trusted -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7

# Let everybody from your company in, plus one guest.
p6a trusted -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -a @example.com -a anna@firm.cz

# Replace the list with addresses from a file.
p6a trusted -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -c -a "$(cat guests.txt)"

# Remove an entry.
p6a trusted -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -r anna@firm.cz

# Trust nobody (every guest waits for approval).
p6a trusted -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -c
```

Output:

```
@example.com
anna@firm.cz

2 trusted entry(ies)
```

With `-F json`, the output is an array of the entries, e.g. `["@example.com","anna@firm.cz"]`. The server does not tell which pending guests a change approved; `p6a list` shows your own access, organizers see the guests in Partoska.

### `edit` - Update a Media Item

Update properties of a media item within an event.

```bash
p6a edit -e <event-id> -m <media-id> [options]
```

At least one field to update must be provided.

**Options:**

- `-e, --event <id>` - Event ID (required).
- `-m, --media <id>` - Media ID (required).
- `-f, --favorite` - Mark the media item as a favorite.
- `-F, --no-favorite` - Unmark the media item as a favorite.
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

**Examples:**

```bash
# Mark a media item as favorite.
p6a edit -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -m 7a8b9c0d-f00d-4a3b-8c5d-e6f700010203 -f

# Unmark a media item as favorite.
p6a edit -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -m 7a8b9c0d-f00d-4a3b-8c5d-e6f700010203 -F

# Favorite all media you uploaded in an event.
EID=12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7
p6a media -e "$EID" -o -1 | xargs -I{} p6a edit -e "$EID" -m {} -f
```

### `approve` - Approve a Media Item

Approve a media item in a moderated event, making it visible to all guests. Has no effect on non-moderated events. With `-r`, revoke the approval instead: the item goes back on hold, hidden from everyone except its author and moderators; revoking fails on non-moderated events, whose uploads are never reviewed. Requires moderator permission.

```bash
p6a approve -e <event-id> -m <media-id> [-r]
```

**Options:**

- `-e, --event <id>` - Event ID (required).
- `-m, --media <id>` - Media ID (required).
- `-r, --revoke` - Revoke approval, putting the item back on hold.
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

**Examples:**

```bash
# Approve a single media item.
p6a approve -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -m 7a8b9c0d-f00d-4a3b-8c5d-e6f700010203

# Approve all pending media in a moderated event.
EID=12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7
p6a media -e "$EID" -F json | \
  jq -r '.[] | select(.approved == false) | .id' | \
  xargs -I{} p6a approve -e "$EID" -m {}

# Revoke approval, putting a media item back on hold.
p6a approve -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -m 7a8b9c0d-f00d-4a3b-8c5d-e6f700010203 -r
```

### `qr` - Download Event QR Code

Download the QR code for an event's primary invite link (PNG or SVG).

```bash
p6a qr -e <id>
```

**Options:**

- `-e, --event <id>` - Event ID (required).
- `-t, --target <file>` - Output file path (default: `<id>-qr.png` or `<id>-qr.svg`).
- `-F, --format <fmt>` - Output format: `png` (default), `svg`.
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

**Examples:**

```bash
# Download QR code as PNG (saved as <id>-qr.png in current directory).
p6a qr -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7

# Download QR code as SVG.
p6a qr -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -F svg

# Download with a custom output path.
p6a qr --event 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 --target my-event-qr.png
```

### `card` - Download Event Share Card

Download a printable share card for an event as a PDF or JPEG. The card contains the event's QR code and is suitable for printing on table stands, posters, or handouts. Requires organizer (admin) access.

```bash
p6a card -e <id> -d <design>
```

**Options:**

- `-e, --event <id>` - Event ID (required).
- `-d, --design <name>` - Card design theme (required): `bday`, `tech`, `match`, `forest`, `garden`, `gold`, `romantic`, `silver`, `neon`, `nineties`, `cottage`, `wish`, `velvet`, `hallows`.
- `-t, --target <file>` - Output file path (default: `<id>-card.pdf` or `<id>-card.jpg`).
- `-F, --format <fmt>` - Output format: `pdf` (default), `jpg`.
- `-l, --locale <loc>` - Language for card text: `en` (default), `cs`, `sk`, `pl`, `ru`, `es`, `de`, `fr`, `it`, `nl`, `uk`.
- `-L, --layout <lay>` - Card layout: `single` (default, one large card per page), `business` (business-card-sized tiles).
- `-p, --paper <pap>` - Paper size: `a4` (default), `letter`.
- `-b, --no-background` - White background, suitable for black-and-white printing.
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

**Examples:**

```bash
# Download a birthday-themed share card as PDF.
p6a card -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -d bday

# Download as JPEG with a custom output path.
p6a card -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -d tech -F jpg -t card.jpg

# Business-card layout on letter paper, Czech locale.
p6a card -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -d forest -L business -p letter -l cs

# White background for black-and-white printing.
p6a card -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -d match --no-background

# Generate share cards for all your events.
p6a list -o -1 | xargs -I{} p6a card -e {} -d bday
```

### `link` - Get Event Invite Link

Print the primary invite link URL for an event to stdout.

```bash
p6a link -e <id>
```

**Options:**

- `-e, --event <id>` - Event ID (required).
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

**Examples:**

```bash
# Print the invite link.
p6a link -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7

# Copy the link to the clipboard (macOS).
p6a link -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 | pbcopy

# Get links for all your events.
p6a list -o -1 | xargs -I{} p6a link -e {}
```

### `media` - List Event Media

List media items for a given event.

```bash
p6a media -e <id>
```

**Options:**

- `-e, --event <id>` - Event ID (required).
- `-o, --owner-only` - List only media you uploaded.
- `-f, --favorite-only` - List only media you have favorited.
- `-1` - Print IDs only, one per line (shorthand for `-F one`).
- `-F, --format <fmt>` - Output format: `plain` (default), `json`, `csv`, `one`.
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

**Examples:**

```bash
# List all media for an event.
p6a media -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7

# List only media you uploaded.
p6a media -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 --owner-only

# List only favorited media.
p6a media -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 --favorite-only

# Output as JSON.
p6a media -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -F json

# Print media IDs only.
p6a media -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -1
```

**Default output columns:**

```
ID                                    TYPE              TAKEN       UPLOADED    OWN FAV  FAVS  APR
12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7  image/jpeg        2026-01-05  2026-01-06   no  no     3  yes
```

### `download` - Download Event Media

Download media for a given event. Supports two mutually exclusive modes.

**All-media mode** — download all media to a directory:

```bash
p6a download -e <id> -t <directory>
```

**Single-media mode** — download one media item to a file:

```bash
p6a download -e <id> -m <media-id> -t <file>
```

**Options:**

- `-e, --event <id>` - Event ID (required in both modes).
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

All-media options:

- `-t, --target <path>` - Target directory (required). Created if it doesn't exist.
- `-o, --owner-only` - Download only media you uploaded.
- `-f, --favorite-only` - Download only media you have favorited.

Single-media options:

- `-m, --media <id>` - Media ID (required).
- `-t, --target <file>` - Output file path (required).

**Examples:**

```bash
# Download all media for an event.
p6a download -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -t ./photos

# Download only media you uploaded.
p6a download -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -t ./photos --owner-only

# Download only favorited media.
p6a download -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -t ./favorites --favorite-only

# Download a single media item.
p6a download -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -m 7a8b9c0d-f00d-4a3b-8c5d-e6f700010203 -t photo.jpg
```

In all-media mode, files are named sequentially (`IMG_0001.jpg`, `MOV_0001.mp4`, etc.) based on what already exists in the target directory.

### `upload` - Upload a Media File

Upload a media file to an event. On moderated events, uploaded files are created as unapproved and require moderator approval before becoming visible to other guests.

```bash
p6a upload -e <event-id> -s <file>
```

Prints the uploaded media ID on success.

**Options:**

- `-e, --event <id>` - Event ID (required).
- `-s, --source <file>` - Path to the local media file to upload (required).
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

**Examples:**

```bash
# Upload a single photo.
p6a upload -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 -s photo.jpg

# Upload multiple photos in a loop.
EID=12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7
for f in *.jpg; do
  p6a upload -e "$EID" -s "$f"
done
```

### `logout` - Remove Credentials

Clear your saved login credentials.

```bash
p6a logout
```

**Options:**

- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

### `help` - Show Help

Display usage information.

```bash
p6a help
# or
p6a --help
```

### `version` - Print Version

Print the installed version of `p6a`.

```bash
p6a version
# or
p6a --version
```

## Common Workflows

### First-time Setup

```bash
# 1. Build and install the tool.
./build.sh
./install.sh

# 2. Login to Partoska.
p6a login

# 3. Download all your photos.
p6a sync -t ~/partoska-backup
```

### Daily Backup

```bash
# Run this anytime to get new photos
# (existing files won't be re-downloaded).
p6a sync -t ~/partoska-backup
```

### Multiple Sync Targets

```bash
# Sync different subsets to different folders.
p6a sync -t ~/all-events                 # Everything
p6a sync -t ~/my-events --owner-only     # Just yours
p6a sync -t ~/favorites --favorite-only  # Just favorites
```

### Switch Accounts

```bash
# Logout from current account.
p6a logout

# Login with different account.
p6a login

# Sync with new account.
p6a sync -t ~/photos
```

## Agent Skill

The `p6a` agent skill teaches AI assistants (Claude, Codex, and other skill-aware agents) how to use this tool. Describe what you want and the assistant runs or writes the right `p6a` commands for you:

- `/p6a sync my photos to ~/backup` - get the exact command to run
- `/p6a create an event for Saturday's BBQ and give me the QR code` - create the event and download its QR
- `/p6a moderate the pending photos at the company party` - review and approve uploads for you
- `/p6a script to back up all my favorite events nightly` - get a shell script or cron job
- `/p6a make a collage from the best photos of the BBQ` - download full-quality originals and design around them

Install it with the [Skills CLI](https://www.npmjs.com/package/skills):

```bash
# Install the skill for all your projects.
npx skills add partoska/p6a-cmd@p6a -g
```

The skill lives in [`skills/p6a`](skills/p6a) and is also attached to every [release](https://github.com/partoska/p6a-cmd/releases) as `p6a_<version>_skill.zip` for manual installation.

If you want to use Partoska from an assistant without the command-line tool, see the [Partoska skill](https://github.com/partoska/p6a-skills), which works through the Partoska MCP server.

## Configuration

The tool stores configuration in `~/.p6a/p6a.ini` by default. You can customize this location:

**Environment variable:**

```bash
export P6A_HOME=/custom/path
p6a login
```

**Command-line flag:**

```bash
p6a login --dir /custom/path
p6a sync --dir /custom/path -t ~/photos
```

### What's stored in the config?

- **OAuth tokens** - Your login credentials (access & refresh tokens) and when the access token expires.
- **API endpoint** - The Partoska API URL.
- **OAuth settings** - Client ID, scope, and the device and token endpoints.

You can view your config file at `~/.p6a/p6a.ini`. It holds your login, so it is readable only by you.

## Technical Details

### Requirements

- **Build dependencies:**
  - CMake 3.20 or later
  - C compiler (GCC or Clang)
  - libcurl
  - [Bikini](https://github.com/partoska/bikini) 2.x, the INI library (linked statically)
- **Runtime dependencies:**
  - libcurl (no deps on Windows)

### How It Works

1. **Authentication:** Uses the OAuth 2.0 device authorization grant (RFC 8628): you approve the login in a browser, on this or any other device.
2. **Token Management:** Automatically renews the login when the access token has expired or is about to, and saves it.
3. **Syncing:** Fetches events via REST API, downloads media files, and tracks what's been synced.
4. **Event Tracking:** Uses IDs to identify events, even if names change.
5. **Interrupting:** Ctrl-C stops a running command cleanly; a file being downloaded is removed, finished files stay.

### Supported Media Types

- **Images:** JPEG (`.jpg`)
- **Videos:** MP4 (`.mp4`), QuickTime (`.mov`)

### Per-Event Metadata

Each event folder contains a `.event.p6a.ini` file that tracks:

- Event UUID (for sync tracking).
- Event name.
- Last sync timestamp.
- Downloaded media files and their types.

This ensures the sync is idempotent and handles event renames gracefully.

## Troubleshooting

### "Login failed" error

The message says why, e.g. the login was denied in the browser or the code expired. Make sure you:

1. Opened the link and checked that the page shows the same code.
2. Allowed access in your browser, within 15 minutes.
3. Have an active internet connection.

### "Request failed" errors

Errors name what failed and why, e.g. `error: Failed to fetch events: Request failed: Couldn't resolve host name`.

- Check your internet connection.
- `Request failed with HTTP ...` is an answer from Partoska; HTTP 403 or 404 usually means a wrong event or media ID.
- Errors ending in `please (re-)login with 'p6a login'` mean the login is missing or was refused: run `p6a logout` then `p6a login`.

### Files aren't downloading

- Make sure the target directory exists and is writable.
- Check disk space.
- Try running with a fresh login: `p6a logout && p6a login`.

### Custom config directory not working

Always use the same `--dir` flag for all commands:

```bash
p6a login --dir /custom/path
p6a sync --dir /custom/path -t ~/photos
p6a logout --dir /custom/path
```

## Library

The tool is a thin layer over `libpartoska`, a small C99 library for the Partoska API: events, media, invite links, QR codes, share cards, downloads and chunked uploads, with the OAuth device login and renewal of the login built in. Its API is in [`src/lib/inc/partoska.h`](src/lib/inc/partoska.h).

```c
#include <partoska.h>
#include <stdio.h>

static PKVoid
prompt (PKVoid *user, const PKChar *uri, const PKChar *code, PKLong expires)
{
  PK_UNUSED (user);
  printf ("Open %s and check the code %s (%ld s left)\n", uri, code, expires);
  fflush (stdout);
}

int
main (void)
{
  if (pkGlobalInit () != PK_OK)
    {
      fprintf (stderr, "Failed to initialize libcurl\n");
      return 1;
    }

  PKClient *client = pkInit ();
  if (!client)
    {
      fprintf (stderr, "Out of memory\n");
      pkGlobalCleanup ();
      return 1;
    }

  // The working directory of the p6a tool, ~/.p6a: log in once.
  PKStatus status = pkOpen (client, NULL);
  if (status == PK_OK && !pkLoginValid (client))
    {
      status = pkLogin (client, prompt, NULL);
    }

  // A failed call leaves the list empty, with nothing to release.
  PKEventList events;
  if (status == PK_OK)
    {
      status = pkEventList (client, NULL, &events);
    }
  if (status == PK_OK)
    {
      for (PKSize i = 0; i < events.count; ++i)
        {
          printf ("%s  %s\n", events.events[i].id, events.events[i].name);
        }
      pkEventListFree (&events);
    }
  else
    {
      fprintf (stderr, "%s\n", pkError (client)->message);
    }

  pkDestroy (client);
  pkGlobalCleanup ();

  return status == PK_OK ? 0 : 1;
}
```

- **Client.** Every call runs within a `PKClient`: its working directory and configuration (`pkOpen`), the login (`pkLogin`, `pkImport`, `pkLogout`, `pkLoginValid`), a throttle that paces the requests, the last error, a log handler and stop requests. The library keeps no global state and never prints. Use a client from one thread at a time.
- **Errors.** Calls return a `PKStatus` (`PK_ENET` transport, `PK_EHTTP` unexpected HTTP status, `PK_EAUTH` login missing or refused, `PK_EDATA` malformed data, `PK_EFS` files, `PK_ESTOPPED`, …); `pkError (client)` holds the code (errno, libcurl code or HTTP status) and a message.
- **Logging.** `pkLogSetHandler` and `pkLogSetLevel` route the messages of a client; without a handler it stays silent.
- **Stopping.** `pkStop (client)` is async-signal-safe: call it from a `SIGINT` or `SIGTERM` handler, and requests in progress and waits end with `PK_ESTOPPED`.
- **Calls.** `pkEventList`, `pkEventCreate`, `pkEventUpdate`, `pkEventLink`, `pkEventQr`, `pkEventCard`, `pkMediaList`, `pkMediaDownload`, `pkMediaUpdate`, `pkMediaApprove`, `pkMediaUpload`, and the steps of an upload session (`pkUploadStart`, `pkUploadChunk`, `pkUploadComplete`, `pkUploadCancel`). An expired access token is renewed on the way, and downloads that fail leave no partial file.
- **Process.** Call `pkGlobalInit` once before the first client (it initializes libcurl) and `pkGlobalCleanup` after the last.

Every package of `p6a` (and `make install` or `cmake --install`) installs the static library `libpartoska.a`, its headers `partoska.h` and `pktypes.h`, and `partoska.pc`. Link with pkg-config:

```bash
cc app.c $(pkg-config --cflags --libs partoska) -o app
```

The library depends on libcurl and [Bikini](https://github.com/partoska/bikini) 2.x, which `partoska.pc` names, so pkg-config has to find them too: install Bikini from its release packages (the `.deb` and `.rpm` of `p6a` suggest `bikini`) and the libcurl development files. With a static libcurl, as on Windows, add `--static`.

Without installing, add this repository to a CMake project and link the `partoska` target:

```cmake
add_subdirectory(path/to/p6a-cmd p6a)
target_link_libraries(app PRIVATE partoska)
```

## Building from Source

### Standard Build and Install

```bash
# Build the project.
./build.sh

# Install to system (requires appropriate permissions).
./install.sh
```

The binary will be created at `build/src/cli/p6a` and the library at `build/src/lib/libpartoska.a`. Installing puts the binary, the library, its headers and `partoska.pc` under the prefix (typically `/usr/local/bin/p6a`, `/usr/local/lib`, `/usr/local/include`). The library is static; `-DBUILD_SHARED_LIBS=ON` builds a shared one instead (`libpartoska.so.2` / `libpartoska.2.dylib`), which the installed `p6a` finds through its RPATH.

Bikini 2.x is found through pkg-config or the default paths. To use an unpacked release archive instead, fetch the pinned one with `.github/scripts/fetch-bikini.sh <platform> deps/bikini` and pass `-DCMAKE_PREFIX_PATH=$PWD/deps/bikini` to CMake, or `BIKINI_PREFIX=$PWD/deps/bikini` to `make`.

### Installation Options

Installation typically requires write permissions to `/usr/local/bin`. You may need to use `sudo`:

```bash
sudo ./install.sh
```

Additional installation options:

```bash
# Install without stripping debug symbols.
./install.sh --no-strip

# Show installation help.
./install.sh --help
```

### Development Build

For development and testing, you can use the build script options:

```bash
# Debug build.
./build.sh -d

# Debug build with verbose logging.
./build.sh -d -v

# Build with sanitizers (address, undefined behavior).
./build.sh -s

# Build and register unit tests.
./build.sh -t

# Use parallel jobs (faster builds).
./build.sh -j 8
```

To run the unit tests:

```bash
./build.sh -t
./test.sh
```

### Manual CMake Build

For advanced users:

```bash
# Configure and build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Install
cmake --install build --strip
```

Available sanitizers: `address`, `undefined`, `float-cast-overflow` (plus `integer` on Clang).

Without CMake, `make` builds the same library and tool (`make test` runs the tests, `make help` lists the options).

## Contributing

Contributions, bug reports, and feature requests are welcome!

[CONTRIBUTING.md](CONTRIBUTING.md) covers everything you need: build setup, code style, running tests, and how to submit a pull request. If you've found a bug or have an idea, feel free to open an issue. Even a detailed bug report is a valuable contribution.

## License

MIT License. See [LICENSE](LICENSE.txt) file for details.

Copyright (C) 2026 Fabrika Charvat s.r.o.
Developed by [Partoska Laboratory](https://lab.partoska.com) team.

## Links

- **Partoska Website:** [https://www.partoska.com](https://www.partoska.com)
- **Project Website:** [https://www.partoska.com/p6a](https://www.partoska.com/p6a)
- **API Documentation:** [https://api.partoska.com/rest/v1/swagger.yml](https://api.partoska.com/rest/v1/swagger.yml)
