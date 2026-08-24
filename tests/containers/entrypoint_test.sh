#!/bin/bash
set -euo pipefail

echo "========================================================"
echo " Running Quasar End-to-End Verification in Container"
echo " Environment: $(cat /etc/os-release | grep PRETTY_NAME | cut -d= -f2)"
echo "========================================================"

# 1. Verify installed binary paths
echo "--> Checking installed binaries and files..."
test -x /usr/bin/quasar
test -x /usr/bin/quasar-theme-selector
test -f /usr/share/applications/quasar.desktop
test -f /usr/share/applications/quasar-theme-selector.desktop
test -f /usr/share/quasar/presets.json
test -x /usr/share/quasar/scripts/calc.sh
test -x /usr/share/quasar/scripts/session.sh
test -f /usr/share/icons/hicolor/scalable/apps/quasar.svg
echo "✓ All files installed in correct locations!"

# 2. Verify bundled scripts functionality
echo "--> Testing bundled scripts..."
CALC_RES=$(/usr/share/quasar/scripts/calc.sh "2 + 3 * 4" | head -n 1)
if [ "$CALC_RES" != "= 14" ]; then
    echo "ERROR: calc.sh expected '= 14', got '$CALC_RES'"
    exit 1
fi
echo "✓ calc.sh evaluated correctly: 2 + 3 * 4 -> $CALC_RES"

# 3. Verify running unit tests
echo "--> Running ctest..."
ctest --test-dir /src/build --output-on-failure
echo "✓ All ctests passed!"

# 4. Verify D-Bus session and Headless launcher execution
echo "--> Testing Headless Runtime under D-Bus and Xvfb..."
export DISPLAY=:99
Xvfb :99 -screen 0 1024x768x24 -nolisten tcp &
XVFB_PID=$!

trap "kill -9 $XVFB_PID 2>/dev/null || true" EXIT

# Wait for Xvfb
for i in {1..20}; do
    if xset q &>/dev/null || [ -e /tmp/.X11-unix/X99 ]; then
        break
    fi
    sleep 0.1
done

# Run quasar inside dbus-run-session
dbus-run-session bash << 'EOF'
set -euo pipefail
export QT_QPA_PLATFORM=xcb
export DISPLAY=:99

echo "Starting quasar in background..."
/usr/bin/quasar &
QUASAR_PID=$!

# Give it a moment to initialize and register on D-Bus
sleep 1.5

# Check if process is still running
if ! kill -0 $QUASAR_PID 2>/dev/null; then
    echo "ERROR: quasar process died unexpectedly!"
    exit 1
fi

echo "Testing D-Bus communication with /usr/bin/quasar --toggle..."
/usr/bin/quasar --toggle

# Check if D-Bus service is registered
if command -v qdbus &>/dev/null; then
    qdbus com.quasar.launcher /Main com.quasar.launcher.toggle || true
fi

echo "Sending SIGTERM to clean up..."
kill -15 $QUASAR_PID 2>/dev/null || true
wait $QUASAR_PID 2>/dev/null || true
echo "✓ Quasar launched and handled D-Bus successfully!"
EOF

echo "========================================================"
echo " SUCCESS: All tests passed for this container!"
echo "========================================================"
