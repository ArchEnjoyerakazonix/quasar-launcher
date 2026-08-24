#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

DISTROS=("ubuntu2404" "fedora40" "archlinux" "fallback")
TARGET="${1:-all}"

echo "=========================================================="
echo " Quasar Multi-Container / Sandbox Matrix Test Runner"
echo " Target(s): $TARGET"
echo "=========================================================="

run_test_container() {
    local distro="$1"
    local dockerfile="${SCRIPT_DIR}/containers/Dockerfile.${distro}"
    local tag="localhost/quasar-test:${distro}"

    if [ ! -f "$dockerfile" ]; then
        echo "Error: Dockerfile for $distro not found at $dockerfile"
        return 1
    fi

    echo ""
    echo "=========================================================="
    echo " [1/2] Building container for: $distro"
    echo "=========================================================="
    podman build -t "$tag" -f "$dockerfile" "$REPO_ROOT"

    echo ""
    echo "=========================================================="
    echo " [2/2] Running verification suite in container: $distro"
    echo "=========================================================="
    podman run --rm --pull=never "$tag"
    echo "✓ [PASS] $distro passed all tests!"
}

if [ "$TARGET" = "all" ]; then
    for d in "${DISTROS[@]}"; do
        run_test_container "$d"
    done
else
    run_test_container "$TARGET"
fi

echo ""
echo "=========================================================="
echo " ALL REQUESTED CONTAINER TESTS COMPLETED SUCCESSFULLY! "
echo "=========================================================="
