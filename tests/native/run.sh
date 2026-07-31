#!/usr/bin/env bash
set -euo pipefail

build_dir="${TMPDIR:-/tmp}/nostrduino-native-tests"
mkdir -p "$build_dir"
g++ -std=c++17 -Wall -Wextra -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I"$(dirname "$0")/../../src" \
  "$(dirname "$0")/test_encoding.cpp" \
  "$(dirname "$0")/../../src/NostrEncoding.cpp" \
  "$(dirname "$0")/../../src/NostrString.cpp" \
  -o "$build_dir/test_encoding"
"$build_dir/test_encoding"
