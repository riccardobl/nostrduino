#!/usr/bin/env bash
set -euo pipefail

build_dir="$(mktemp -d "${TMPDIR:-/tmp}/nostrduino-native-tests.XXXXXX")"
trap 'rm -rf "$build_dir"' EXIT
g++ -std=c++17 -Wall -Wextra -Werror -Wno-unused-parameter \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I"$(dirname "$0")/../../src" \
  "$(dirname "$0")/test_encoding.cpp" \
  "$(dirname "$0")/../../src/NostrEncoding.cpp" \
  "$(dirname "$0")/../../src/NostrString.cpp" \
  -o "$build_dir/test_encoding"
"$build_dir/test_encoding"

ubitcoin_src="$(dirname "$0")/../../src/uBitcoin/src"
if [[ ! -f "$ubitcoin_src/Bitcoin.h" ]]; then
  ubitcoin_src="$(find "$(dirname "$0")/../../.pio/libdeps" -path '*/uBitcoin/src' -type d -print -quit 2>/dev/null || true)"
fi
if [[ -z "$ubitcoin_src" || ! -f "$ubitcoin_src/Bitcoin.h" ]]; then
  echo "uBitcoin 0.2.0 sources are required for native crypto coverage" >&2
  exit 1
fi
ubitcoin_objects=()
while IFS= read -r source; do
  relative="${source#"$ubitcoin_src"/}"
  object="$build_dir/ubitcoin_${relative//\//_}.o"
  object="${object%.*}.o"
  if [[ "$source" == *.cpp ]]; then
    g++ -std=c++17 -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
      -DUSE_STDONLY -I"$ubitcoin_src" -c "$source" -o "$object"
  else
    gcc -std=c99 -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
      -I"$ubitcoin_src" -c "$source" -o "$object"
  fi
  ubitcoin_objects+=("$object")
done < <(find "$ubitcoin_src" -type f \( -name '*.c' -o -name '*.cpp' \) -print | sort)
g++ -std=c++17 -Wall -Wextra -Werror -Wno-unused-parameter \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -DUSE_STDONLY -I"$(dirname "$0")/../../src" -I"$ubitcoin_src" \
  "$(dirname "$0")/test_crypto.cpp" \
  "$(dirname "$0")/../../src/crypto/UBitcoinBackend.cpp" \
  "${ubitcoin_objects[@]}" \
  -o "$build_dir/test_crypto"
"$build_dir/test_crypto" "$(dirname "$0")/bip340_test_vectors.csv"

json_include="$(find "$(dirname "$0")/../../.pio/libdeps" -path '*/ArduinoJson/src' -type d -print -quit 2>/dev/null || true)"
if [[ -z "$json_include" ]]; then
  echo "ArduinoJson headers are required for native parser coverage" >&2
  exit 1
fi

native_flags=(
  -std=c++17 -Wall -Wextra -Werror
  -fsanitize=address,undefined -fno-omit-frame-pointer
  -ffunction-sections -fdata-sections
  -I"$(dirname "$0")/mocks"
  -I"$(dirname "$0")/../../src"
  -I"$json_include"
)
g++ "${native_flags[@]}" \
  "$(dirname "$0")/test_parser.cpp" \
  "$(dirname "$0")/../../src/NostrEvent.cpp" \
  "$(dirname "$0")/../../src/NostrEncoding.cpp" \
  "$(dirname "$0")/../../src/NostrString.cpp" \
  -Wl,--gc-sections -o "$build_dir/test_parser"
"$build_dir/test_parser"

g++ "${native_flags[@]}" \
  "$(dirname "$0")/test_nip04.cpp" \
  "$(dirname "$0")/../../src/Nip04.cpp" \
  "$(dirname "$0")/../../src/NostrAES.cpp" \
  "$(dirname "$0")/../../src/NostrString.cpp" \
  -o "$build_dir/test_nip04"
"$build_dir/test_nip04"

g++ -std=c++17 -Wall -Wextra -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I"$(dirname "$0")/../../src" \
  "$(dirname "$0")/test_transport_helpers.cpp" \
  "$(dirname "$0")/../../src/NostrUrl.cpp" \
  "$(dirname "$0")/../../src/NostrString.cpp" \
  -o "$build_dir/test_transport_helpers"
"$build_dir/test_transport_helpers"
