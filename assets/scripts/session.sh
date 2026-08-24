#!/usr/bin/env bash
# Quasar pipe plugin: session menu.
# First call: prints Lock / Log Out / Suspend / Reboot / Power Off.
# Selection round-trip: performs the action and prints nothing.
set -u

arg="${1:-}"

if [[ -z "$arg" ]]; then
    echo "Lock Screen"
    echo "Log Out"
    echo "Suspend"
    echo "Reboot"
    echo "Power Off"
    exit 0
fi

case "$arg" in
    "Lock Screen")
        if command -v loginctl >/dev/null 2>&1; then
            loginctl lock-session
        elif command -v hyprctl >/dev/null 2>&1; then
            hyprctl dispatch exec swaylock
        fi
        ;;
    "Log Out")
        if [[ -n "${HYPRLAND_INSTANCE_SIGNATURE:-}" ]] && command -v hyprctl >/dev/null 2>&1; then
            hyprctl dispatch exit
        elif [[ -n "${SWAYSOCK:-}" ]] && command -v swaymsg >/dev/null 2>&1; then
            swaymsg exit
        elif command -v loginctl >/dev/null 2>&1; then
            loginctl terminate-session "${XDG_SESSION_ID:-}"
        fi
        ;;
    "Suspend")
        systemctl suspend 2>/dev/null || loginctl suspend 2>/dev/null || true
        ;;
    "Reboot")
        systemctl reboot 2>/dev/null || loginctl reboot 2>/dev/null || true
        ;;
    "Power Off")
        systemctl poweroff 2>/dev/null || loginctl poweroff 2>/dev/null || true
        ;;
esac
