#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p tests/out
for name in engine textures hud; do
  "${CC:-cc}" -std=c11 -W -Wall -Werror -pedantic -O2 \
    $(pkg-config --cflags cairo) -Isrc \
    "src/$name.c" "tests/${name}_test.c" \
    -o "tests/out/${name}_test" $(pkg-config --libs cairo) -lm
  "./tests/out/${name}_test" > "tests/out/${name}_test.log" 2>&1
  cat "tests/out/${name}_test.log"
  if ! grep -qx "${name}_test: OK" "tests/out/${name}_test.log" ||
     grep -q 'FAIL' "tests/out/${name}_test.log"; then
    exit 1
  fi
done
