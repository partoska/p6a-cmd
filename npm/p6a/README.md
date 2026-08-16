# @partoska/p6a

Manage and sync your event photos from [Partoska](https://www.partoska.com) —
right from your terminal.

[![npm version](https://img.shields.io/npm/v/@partoska/p6a.svg)](https://www.npmjs.com/package/@partoska/p6a)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](https://github.com/partoska/p6a-cmd/blob/main/LICENSE.txt)

## What is this?

`p6a` is a lightweight command-line tool that helps you download and organize
your event photos from Partoska. It's perfect for backing up your memories or
working with your photos offline.

**Key features:**

- Simple authentication via OAuth.
- Smart sync that doesn't re-download existing files.
- Automatic folder organization by event and date.
- Filter by events you own or have favorited.
- Create and update events, moderate uploads, and manage media.
- Upload photos directly to an event from the command line.
- Get or download invite links, QR codes, and printable share cards.
- Works on macOS, Linux, and Windows.
- [Agent skills available](https://github.com/partoska/p6a-skills) for use with
  AI assistants.

## Install

```bash
npm install -g @partoska/p6a
```

This installs a small launcher plus the prebuilt native binary for your
platform, selected automatically. No compiler or build tools are required — see
[How this package works](#how-this-package-works) for the details.

You can also run it without a global install using `npx`:

```bash
npx @partoska/p6a login
```

### Supported platforms

| OS      | Architectures           |
| ------- | ----------------------- |
| macOS   | Intel & Apple Silicon   |
| Linux   | x64, arm64              |
| Windows | x64, arm64              |

## Quick Start

### 1. Login

```bash
p6a login
```

You'll see a device code and URL. Open the URL in your browser, enter the code,
and authorize the app.

### 2. Sync your photos

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

Run it again anytime — already-downloaded files are skipped, and folders are
renamed automatically if an event's name changes.

## Commands

| Command    | Description                        |
| ---------- | ---------------------------------- |
| `login`    | Authenticate with Partoska         |
| `logout`   | Remove saved credentials           |
| `sync`     | Download and organize event photos |
| `list`     | List events                        |
| `create`   | Create a new event                 |
| `update`   | Update an existing event           |
| `edit`     | Update a media item                |
| `approve`  | Approve a media item               |
| `qr`       | Download QR code for an event      |
| `card`     | Download share card for an event   |
| `link`     | Get invite link for an event       |
| `media`    | List media items for an event      |
| `download` | Download media for an event        |
| `upload`   | Upload a photo to an event         |
| `help`     | Show usage information             |
| `version`  | Print version information          |

Run `p6a help` or `p6a <command> --help` at any time for full usage.

### `login` — Authenticate with Partoska

```bash
p6a login                          # Standard device-flow login.
p6a login --dir /data/p6a-config   # Use a custom config location.
p6a login --import /path/config.ini  # Import config from an INI file.
```

### `sync` — Download your photos

Sync events and photos from Partoska to your computer.

```bash
p6a sync -t ~/photos                  # Sync all events.
p6a sync -t ~/my-events --owner-only  # Only events you created.
p6a sync -t ~/favorites --favorite-only  # Only your favorites.
p6a sync -t ~/my-favorites -o -f      # Combine filters (own AND favorite).
```

**What happens when you sync:**

- Events are organized into folders named `YYYY-MM-DD Event Name`.
- Photos are named sequentially (`IMG_0001.jpg`, `IMG_0002.jpg`, …).
- Videos are named with a `MOV_` prefix (`MOV_0001.mp4`).
- Already-downloaded files are automatically skipped.
- If event names change, folders are renamed automatically.

### `list` — List events

```bash
p6a list                          # List all events.
p6a list -q "Birthday"            # Search by name.
p6a list --owner-only             # Only your own events.
p6a list -1                       # Print event IDs only, one per line.
p6a list -F json | jq '.[].name'  # Machine-readable output.
```

Output formats (`-F`): `plain` (default), `json`, `csv`, `one`.

### `create` / `update` — Manage events

```bash
# Create a new event (name must be 3–48 characters).
p6a create -n "Birthday Party"

# Rename an event.
p6a update -e <event-id> -n "New Name"

# Change the event dates (ISO 8601).
p6a update -e <event-id> -S 2026-06-01T10:00:00Z -E 2026-06-01T22:00:00Z

# Make an event public, or private.
p6a update -e <event-id> --public
p6a update -e <event-id> --private

# Enable upload moderation (new uploads require approval before they appear).
p6a update -e <event-id> --moderated
```

### `media`, `edit` & `approve` — Manage media

```bash
# List media for an event.
p6a media -e <event-id>
p6a media -e <event-id> --owner-only       # Only media you uploaded.
p6a media -e <event-id> -F json            # Machine-readable output.

# Mark / unmark a media item as a favorite.
p6a edit -e <event-id> -m <media-id> -f
p6a edit -e <event-id> -m <media-id> -F

# Approve a pending item in a moderated event (requires moderator permission).
p6a approve -e <event-id> -m <media-id>
```

### `download` — Download media

```bash
# Download all media for an event into a directory.
p6a download -e <event-id> -t ./photos
p6a download -e <event-id> -t ./photos --owner-only
p6a download -e <event-id> -t ./favorites --favorite-only

# Download a single media item to a file.
p6a download -e <event-id> -m <media-id> -t photo.jpg
```

### `upload` — Upload photos

Upload a local media file to an event. Prints the new media ID on success. On
moderated events, uploads are created unapproved and require moderator approval
before they appear to other guests.

```bash
# Upload a single photo.
p6a upload -e <event-id> -s photo.jpg

# Upload every JPEG in the current directory.
EID=<event-id>
for f in *.jpg; do p6a upload -e "$EID" -s "$f"; done
```

### `link`, `qr` & `card` — Share events

```bash
# Print the primary invite link to stdout.
p6a link -e <event-id>
p6a link -e <event-id> | pbcopy        # Copy to clipboard (macOS).

# Download the event QR code (PNG by default, or SVG).
p6a qr -e <event-id>
p6a qr -e <event-id> -F svg

# Download a printable share card (PDF by default, or JPEG).
# Designs: bday, tech, match, forest, garden, gold, romantic, silver, neon, nineties, cottage.
p6a card -e <event-id> -d bday
p6a card -e <event-id> -d forest -L business -p letter -l cs
```

### `logout`, `help` & `version`

```bash
p6a logout      # Clear saved credentials.
p6a help        # Show usage information (also: p6a --help).
p6a version     # Print the installed version (also: p6a --version).
```

## Common Workflows

### First-time setup

```bash
npm install -g @partoska/p6a
p6a login
p6a sync -t ~/partoska-backup
```

### Daily backup

```bash
# Run anytime to grab new photos — existing files are not re-downloaded.
p6a sync -t ~/partoska-backup
```

### Multiple sync targets

```bash
p6a sync -t ~/all-events                 # Everything.
p6a sync -t ~/my-events --owner-only     # Just yours.
p6a sync -t ~/favorites --favorite-only  # Just favorites.
```

### Scripting with IDs

```bash
# Download QR codes for all the events you own.
p6a list -o -1 | xargs -I{} p6a qr -e {}

# Get invite links for all your events.
p6a list -o -1 | xargs -I{} p6a link -e {}

# Approve all pending media in a moderated event.
EID=<event-id>
p6a media -e "$EID" -F json \
  | jq -r '.[] | select(.approved == false) | .id' \
  | xargs -I{} p6a approve -e "$EID" -m {}
```

## Configuration

The tool stores configuration in `~/.p6a/p6a.ini` by default (OAuth tokens, API
endpoints, and OAuth settings). You can customize the location with either an
environment variable or a flag:

```bash
export P6A_HOME=/custom/path
p6a login

# Or per-command (use the same --dir for every command):
p6a login --dir /custom/path
p6a sync  --dir /custom/path -t ~/photos
```

## How this package works

`@partoska/p6a` follows the per-platform `optionalDependencies` model. The
package you install ships only a tiny Node launcher; the actual native binary
comes from one of these platform packages, and npm installs only the one
matching your OS and CPU:

- `@partoska/p6a-darwin` — macOS universal (Intel + Apple Silicon)
- `@partoska/p6a-linux-x64`
- `@partoska/p6a-linux-arm64`
- `@partoska/p6a-win32-x64`
- `@partoska/p6a-win32-arm64`

When you run `p6a`, the launcher detects your platform, resolves the matching
binary, and executes it — forwarding all arguments and its exit status. There is
no build step and no runtime dependency on Node beyond launching.

## Troubleshooting

### "Unsupported platform" or "could not find the native binary"

This usually means optional dependencies were skipped during install (for
example with `npm install --no-optional`, or behind a strict CI mirror).
Reinstall with optional dependencies enabled:

```bash
npm install -g @partoska/p6a
```

### "Failed to login"

- Make sure you entered the device code correctly and authorized the app in
  your browser.
- Check that you have an active internet connection.

### "Network error" during sync

- Verify your internet connection.
- Refresh your session: `p6a logout` then `p6a login`.

### Files aren't downloading

- Make sure the target directory exists and is writable, and that you have
  enough disk space.
- Try a fresh login: `p6a logout && p6a login`.

## License

MIT. Copyright (C) 2026 Fabrika Charvat s.r.o. Developed by the
[Partoska Laboratory](https://lab.partoska.com) team.

## Links

- **Project website:** <https://www.partoska.com/p6a>
- **Partoska website:** <https://www.partoska.com>
- **Source & issues:** <https://github.com/partoska/p6a-cmd>
- **Agent skills:** <https://github.com/partoska/p6a-skills>
