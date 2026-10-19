# Helping a Host Around a Live Event

Use this reference when the user is hosting an event that is happening now, or about to: checking how photo collection is going, nudging guests to upload, or keeping moderation running while they enjoy the party. See `../SKILL.md` for the command reference and `review.md` for how to look at and judge photos.

---

## Workflow: Coverage Check and Guest Nudge

"How are the photos going?", "Did anyone take pictures of the speeches?", "Remind people to upload."

```bash
EID=$(p6a list -q "Summer Party" -1 | head -1)

# Event window, guest and photo counts, moderation
p6a list -F json | jq --arg id "$EID" '.[] | select(.id == $id) | {name, from, to, guests, media, moderated}'

# Photos per hour of taken time
p6a media -e "$EID" -F json | jq -r 'map(.taken[0:13]) | group_by(.) | .[] | "\(.[0]):00  \(length)"'

# Moderation backlog
p6a media -e "$EID" -F json | jq '[.[] | select(.approved == false)] | length'
```

What to report, briefly:

- **Volume.** Photos so far versus guests who joined, and how recent the latest uploads are.
- **Gaps in time.** Empty or thin stretches across the event window — "nothing between 21:00 and 22:30." When hours are too coarse, group by ten-minute prefixes (`.taken[0:15]`) instead. Taken timestamps come from the camera and may be in UTC or off by a time zone; say so if the gaps look suspicious.
- **Gaps in moments.** If the user names moments that matter (speeches, first dance, award ceremony), check whether photos exist around those times. Look at a few to confirm (see `review.md`) — don't download the whole event.
- **Moderation backlog.** In a moderated event, the number of photos still pending. Other guests can't see those uploads until they are approved, and a gallery that looks empty discourages more uploads — offer to moderate (`review.md`).

`p6a media` only tells you whether *you* uploaded a photo (`owner`), not which guests did, so don't name people who haven't.

Then offer a nudge the host can send or show:

- **A message** for the event's group chat: short, warm, in the event's language, with the invite link from `p6a link -e "$EID"`. If a moment is missing, ask for it specifically — "Anyone got photos from the first dance? Add them here: <link>".
- **A screen slide or table card** with the QR (`p6a qr -e "$EID" -F svg`), when guests are in the room — see the slideshow slide and QR stand guidance in `design.md`.

Draft the message; don't send it yourself unless the user asks and confirms the final text and recipients.

---

## Workflow: Live Moderation

Keep a moderated event flowing during the party: review new uploads every 15–30 minutes, so guests see their photos quickly without the host checking a phone all evening. Each pass needs your judgment, so it runs as an agent task, not a plain script — either a recurring task inside the agent session (if your environment supports loops or scheduled tasks), or a headless agent run started by cron or a scheduler (if the user's agent has a non-interactive mode). If neither is available, offer to run a pass whenever the user asks.

**Set up — with the user, before the first run:**

1. Resolve the event and check `moderated` in `p6a list -F json`; on an unmoderated event uploads are already visible, so there's nothing to moderate. Confirm you can view images ("Before You Start" in `review.md`).
2. Agree on the rules (see "Ground Rules" in `review.md`). Nobody will be around to answer questions mid-run, so settle them now — and for a headless run, write them into the prompt or a rules file it reads. Whatever the rules don't clearly cover stays pending.
3. Agree on the interval, when to stop (default: the event's `to` time), and how to report — by default a short note only when something was flagged, plus a summary at the end.

**Each run:**

```bash
STATE="$HOME/.cache/p6a-review/$EID.tsv"   # media id <TAB> decision <TAB> reason
mkdir -p "$(dirname "$STATE")" && touch "$STATE"

# Pending images not judged in an earlier run
p6a media -e "$EID" -F json | \
  jq -r --rawfile seen "$STATE" '($seen | split("\n") | map(split("\t")[0])) as $s
    | .[] | select(.approved == false and .type == "image/jpeg" and ((.id | IN($s[])) | not)) | .id'

# Download, view, and judge each as in "Seeing a Photo" (review.md); approve clear passes
p6a approve -e "$EID" -m <media-id>

# Record every decision so the next run skips it
printf '%s\t%s\t%s\n' <media-id> approved "" >> "$STATE"
printf '%s\t%s\t%s\n' <media-id> pending "screen with personal messages" >> "$STATE"
```

- **Stop on trouble.** If `p6a approve` fails with a permission error, or the event stops being moderated, stop the schedule and tell the user.
- **Fixing a slip.** If the user spots a photo that shouldn't have been approved, revoke its approval with `p6a approve -r` (see `review.md`) — and tighten the rules for the remaining runs.
- **Wrap up.** At the end, summarize the whole run from the state file — approved, still pending with reasons, videos waiting for the user — stop the schedule, and offer to delete the state file.
