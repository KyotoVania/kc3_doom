#!/bin/sh
set -e
cd "$(dirname "$0")/.."

var () {
  grep "^$1 =" config.mk | cut -d= -f2-
}

mkdir -p tests/out
cc -shared $(var CPPFLAGS) $(var CFLAGS) \
   tests/window_backend_test.c -o tests/out/window_backend_test.so
echo "window_backend_test: built tests/out/window_backend_test.so"

if [ "${1:-}" = "run" ]; then
  LD_PRELOAD="$PWD/tests/out/window_backend_test.so" ./kmx_doom
fi
