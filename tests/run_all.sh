#!/bin/sh
set -e
cd "$(dirname "$0")/.."
mkdir -p tests/out

check_output () {
  name=$1
  cat "tests/out/$name.log"
  if ! grep -qx "$name: OK" "tests/out/$name.log" ||
     grep -q 'FAIL' "tests/out/$name.log"; then
    echo "$name: validation failed" >&2
    exit 1
  fi
}

for name in engine textures hud; do
  cc -std=c11 -W -Wall -Werror -pedantic -O2 \
     $(pkg-config --cflags cairo) -Isrc \
     "src/$name.c" "tests/${name}_test.c" \
     -o "tests/out/${name}_test" $(pkg-config --libs cairo) -lm
  "./tests/out/${name}_test" > "tests/out/${name}_test.log" 2>&1
  check_output "${name}_test"
done

cd ..
. ./env
cd kc3_doom

for name in data_test game_test view_check motion_test app_test; do
  timeout 30s ../kc3s/kc3s --load "tests/$name.kc3" --quit \
    > "tests/out/$name.log" 2>&1
  check_output "$name"
done

make
sh tests/window_backend_test.sh
preload="$PWD/tests/out/window_backend_test.so"
timeout 15s env LD_PRELOAD="$preload" ./kmx_doom \
  > tests/out/window_backend_test.log 2>&1
check_output window_backend_test

timeout 15s env WINDOW_BACKEND_TEST_WM_CLOSE=1 LD_PRELOAD="$preload" ./kmx_doom \
  > tests/out/window_close_test.log 2>&1
cat tests/out/window_close_test.log
grep -q 'window_backend_test: WM_CLOSE' tests/out/window_close_test.log
if grep -q 'FAIL' tests/out/window_close_test.log; then
  exit 1
fi

if timeout 15s env WINDOW_BACKEND_TEST_FAIL_LOAD=1 LD_PRELOAD="$preload" ./kmx_doom \
   > tests/out/window_failure_test.log 2>&1; then
  echo 'window_failure_test: expected a nonzero exit' >&2
  exit 1
else
  status=$?
  if [ "$status" -ne 1 ]; then
    echo "window_failure_test: unexpected exit $status" >&2
    exit 1
  fi
fi
cat tests/out/window_failure_test.log
grep -q 'window_backend_test: FAIL_LOAD' tests/out/window_failure_test.log
echo 'window_lifecycle_test: OK'

timeout 15s env LD_PRELOAD="$preload" ./kmx_doom \
  --load tests/window_contract_test.kc3 \
  > tests/out/window_contract_test.log 2>&1
grep '^window_contract_test:' tests/out/window_contract_test.log
grep -qx 'window_contract_test: OK' tests/out/window_contract_test.log
if grep -Eq '^window_contract_test:.*FAIL|^FAIL ' \
   tests/out/window_contract_test.log; then
  exit 1
fi
