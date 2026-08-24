#!/usr/bin/env bash
# Quasar pipe plugin: clipboard history browser (cliphist).
# First call: prints the history; selecting an entry decodes it back into
# the clipboard and prints nothing (the launcher closes).
set -u

arg="${1:-}"

if [[ -z "$arg" ]]; then
    if ! command -v cliphist >/dev/null 2>&1; then
        echo "cliphist is not installed"
        exit 0
    fi
    cliphist list | head -50
    exit 0
fi

# Round-trip: decode the selected entry into the clipboard.
if command -v wl-copy >/dev/null 2>&1; then
    cliphist decode <<< "$arg" | wl-copy
elif command -v xclip >/dev/null 2>&1; then
    cliphist decode <<< "$arg" | xclip -selection clipboard
elif command -v xsel >/dev/null 2>&1; then
    cliphist decode <<< "$arg" | xsel --clipboard --input
fi
