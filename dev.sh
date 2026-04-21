#!/usr/bin/env bash
# dev.sh - Build and optionally run a target
# Usage:
#   ./dev.sh            -> Build + run tests
#   ./dev.sh tui        -> Build + run TUI demo
#   ./dev.sh bench      -> Build + run performance benchmark
#   ./dev.sh clean      -> Clean build directory

set -e

BUILD_DIR="build"
TARGET="${1:-test}"

case "$TARGET" in
    clean)
        echo "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
        cmake -B "$BUILD_DIR" -DCMAKE_CXX_FLAGS="-Wall -Wextra -Werror"
        ;;
    tui)
        cmake --build "$BUILD_DIR" -j"$(nproc)" --target lotus_tui 2>&1
        exec "$BUILD_DIR/lotus_tui"
        ;;
    bench)
        cmake --build "$BUILD_DIR" -j"$(nproc)" --target lotus_bench 2>&1
        exec "$BUILD_DIR/lotus_bench"
        ;;
    test|*)
        cmake --build "$BUILD_DIR" -j"$(nproc)" 2>&1
        echo ""
        exec "$BUILD_DIR/lotus_tests"
        ;;
esac
