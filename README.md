# Partoska Command-Line Interface (`p6a`)

Manage and sync your event photos from [Partoska](https://www.partoska.com) — right from your terminal.

[![Version](https://img.shields.io/badge/version-1.7.0-blue.svg)](https://github.com/partoska/p6a-cmd/releases/tag/v1.7.0)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE.txt)

## What is this?

`p6a` is a lightweight command-line tool that helps you download and organize your event photos from Partoska. It's perfect for backing up your memories or working with your photos offline.

**Key features:**

- Simple authentication via OAuth.
- Smart sync that doesn't re-download existing files.
- Automatic folder organization by event and date.
- Filter by events you own or have favorited.
- Get or download invite links and QR codes for sharing events.
- Works on macOS, Linux, and Windows.
- [Agent skills available](https://github.com/partoska/p6a-skills) for use with AI assistants.

## Quick Start

**Prerequisites:** CMake 3.13+, GCC or Clang, and libcurl (e.g. `sudo apt install libcurl4-openssl-dev` on Ubuntu or `brew install curl` on macOS).

### 1. Install

```bash
# Clone the repository.
git clone https://github.com/partoska/p6a-cmd.git
cd p6a-cmd

# Build and install the tool.
./build.sh
sudo ./install.sh

# The binary will be at: build/src/p6a
```

### 2. Login

```bash
p6a login
```

You'll see a device code and URL. Open the URL in your browser, enter the code, and authorize the app.

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

| Command    | Description                        |
| ---------- | ---------------------------------- |
| `login`    | Authenticate with Partoska         |
| `logout`   | Remove saved credentials           |
| `sync`     | Download and organize event photos |
| `list`     | List events                        |
| `create`   | Create a new event                 |
| `update`   | Update an existing event           |
| `qr`       | Download QR code for an event      |
| `link`     | Get invite link for an event       |
| `media`    | List media items for an event      |
| `download` | Download media for an event        |
| `help`     | Show usage information             |
| `version`  | Print version information          |

### `login` - Authenticate with Partoska

Connect your Partoska account.

```bash
p6a login
```

**Options:**

- `-D, --dir <path>` - Use a custom config directory (default: `~/.p6a`).
- `-i, --import <file>` - Import config from an INI file.

**Examples:**

```bash
# Standard login.
p6a login

# Use custom config location.
p6a login --dir /data/p6a-config

# Import configuration from file.
p6a login --import /path/to/config.ini
```

### `sync` - Download Your Photos

Sync events and photos from Partoska to your computer.

```bash
p6a sync -t <target-directory>
```

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

# Print event IDs only.
p6a list -1

# Extract event names using jq.
p6a list -F json | jq '.[].name'

# Download QR codes for all your events.
p6a list -o -1 | xargs -I{} p6a qr -e {}
```

**Default output columns:**

```
NAME                              ID                                    FROM        TO          OWN FAV PUB  GUESTS  MEDIA
Birthday Party                    12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7  2026-01-05  2026-01-06  yes yes yes       5     42
```

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
NAME                              ID                                    FROM        TO          OWN FAV PUB  GUESTS  MEDIA
Birthday Party                    12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7  2026-06-01  2026-06-01  yes  no yes       1      0

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

### `qr` - Download Event QR Code

Download the QR code for an event's primary invite link (PNG or SVG).

```bash
p6a qr -e <id>
```

**Options:**

- `-e, --event <id>` - Event ID (required).
- `-t, --target <file>` - Output file path (default: `<id>-qr.png` or `<id>-qr.svg`).
- `-s, --svg` - Request SVG format instead of PNG.
- `-D, --dir <path>` - Custom config directory (default: `~/.p6a`).

**Examples:**

```bash
# Download QR code as PNG (saved as <id>-qr.png in current directory).
p6a qr -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7

# Download QR code as SVG.
p6a qr -e 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 --svg

# Download with a custom output path.
p6a qr --event 12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7 --target my-event-qr.png
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
ID                                    TYPE              TAKEN       UPLOADED    OWN FAV  FAVS
12cafe34-5b8a-4d2e-9f01-0203a4b5c6d7  image/jpeg        2026-01-05  2026-01-06   no  no     3
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

- **OAuth tokens** - Your login credentials (access & refresh tokens).
- **API endpoints** - Partoska API URLs.
- **OAuth settings** - Client ID, scopes, redirect URLs.

You can view your config file at `~/.p6a/p6a.ini`.

## Technical Details

### Requirements

- **Build dependencies:**
  - CMake 3.13 or later
  - C compiler (GCC or Clang)
  - libcurl
- **Runtime dependencies:**
  - libcurl (no deps on Windows)

### How It Works

1. **Authentication:** Uses OAuth 2.0 device authorization flow with PKCE for security.
2. **Token Management:** Automatically refreshes expired tokens.
3. **Syncing:** Fetches events via REST API, downloads media files, and tracks what's been synced.
4. **Event Tracking:** Uses IDs to identify events, even if names change.

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

### "Failed to login" error

Make sure you:

1. Entered the device code correctly.
2. Authorized the app in your browser.
3. Have an active internet connection.

### "Network error" during sync

- Check your internet connection.
- Verify your login is still valid: `p6a logout` then `p6a login`.

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

## Building from Source

### Standard Build and Install

```bash
# Build the project.
./build.sh

# Install to system (requires appropriate permissions).
./install.sh
```

The binary will be created at `build/src/p6a` and installed to your system (typically `/usr/local/bin/p6a`).

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
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# Install
cmake --install . --strip
```

Available sanitizers: `address`, `undefined`, `float-cast-overflow` (plus `integer` on Clang).

## Contributing

Contributions, bug reports, and feature requests are welcome!

[CONTRIBUTING.md](CONTRIBUTING.md) covers everything you need: build setup, code style, running tests, and how to submit a pull request. If you've found a bug or have an idea, feel free to open an issue. Even a detailed bug report is a valuable contribution.

## License

MIT License. See [LICENSE](LICENSE.txt) file for details.

Copyright (C) 2026 Fabrika Charvat s.r.o.
Developed by [Partoska Laboratory](https://lab.partoska.com) team.

## Links

- **Partoska Website:** [https://www.partoska.com](https://www.partoska.com)
- **Project Website:** [https://lab.partoska.com/p6a](https://lab.partoska.com/p6a)
- **Agent Skills:** [https://github.com/partoska/p6a-skills](https://github.com/partoska/p6a-skills)
- **API Documentation:** [https://api.partoska.com/rest/v1/swagger.yml](https://api.partoska.com/rest/v1/swagger.yml)
