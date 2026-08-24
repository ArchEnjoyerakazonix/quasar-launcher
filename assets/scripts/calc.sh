#!/usr/bin/env bash
# Quasar pipe plugin: calculator.
# First call ("input": "query"):  calc.sh "2+2*10"      → prints "= 22"
# Selection round-trip:            calc.sh "= 22"        → copies 22 to the
# clipboard and prints nothing (the launcher closes).
set -u

arg="${1:-}"

if [[ -z "$arg" ]]; then
    echo "Type an expression after /calc"
    echo "Example: /calc (1920-80)/2*3"
    exit 0
fi

copy_to_clipboard() {
    if command -v wl-copy >/dev/null 2>&1; then
        printf '%s' "$1" | wl-copy
    elif command -v xclip >/dev/null 2>&1; then
        printf '%s' "$1" | xclip -selection clipboard
    elif command -v xsel >/dev/null 2>&1; then
        printf '%s' "$1" | xsel --clipboard --input
    fi
}

# Round-trip: the user picked "= <result>" — copy the value.
if [[ "$arg" =~ ^=[[:space:]]*(.*)$ ]]; then
    copy_to_clipboard "${BASH_REMATCH[1]}"
    exit 0
fi

# First call: evaluate the expression.
result=$(python3 -c "from math import *; print($arg)" 2>/dev/null) \
    || result=$(echo "scale=10; $arg" | bc 2>/dev/null)

if [[ -n "$result" && "$result" != *"error"* ]]; then
    echo "= ${result}"
else
    echo "= invalid expression"
fi
