#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
mode=${1:-safe}
case "$mode" in
  safe)
    "${CXX:-g++}" -std=c++17 -O1 -g -Wall -Wextra -pthread cpp/safe.cpp -o build/cpp-safe
    ./build/cpp-safe
    rustc --edition=2021 rust/safe.rs -o build/rust-safe
    ./build/rust-safe
    ./build/rust-safe 1
    ;;
  check-fail)
    command -v rustc >/dev/null
    names=(uaf double_free dangling iterator race rc_thread option constant_bounds)
    expected=(E0382 E0382 E0515 E0502 E0499 E0277 E0308 unconditional_panic)
    for i in "${!names[@]}"; do
      name=${names[$i]}
      if rustc --edition=2021 "rust/compile_fail/$name.rs" -o "build/fail-$name" 2>"build/$name.log"; then
        echo "FAIL: $name unexpectedly compiled"; exit 1
      fi
      if ! grep -q "${expected[$i]}" "build/$name.log"; then
        cat "build/$name.log"; echo "FAIL: expected ${expected[$i]}"; exit 1
      fi
      echo "PASS: $name rejected (${expected[$i]})"
    done
    ;;
  panic)
    rustc --edition=2021 rust/bounds_panic.rs -o build/bounds-panic
    if ./build/bounds-panic 10 >build/panic.log 2>&1; then
      echo 'FAIL: expected panic'; exit 1
    fi
    grep -q 'index out of bounds' build/panic.log
    cat build/panic.log
    echo 'PASS: runtime bounds check triggered'
    ;;
  unsafe)
    demo=${2:-uaf}
    case "$demo" in uaf|double-free|iterator|overflow|race|leak|null) ;; *) echo 'Unknown demo'; exit 2;; esac
    sanitizer=address,undefined
    if [[ "$demo" == race ]]; then sanitizer=thread; fi
    "${CXX:-g++}" -std=c++17 -O0 -g -fno-omit-frame-pointer -pthread \
      "-fsanitize=$sanitizer" cpp/unsafe.cpp -o "build/unsafe-$demo"
    echo "Running intentional undefined behavior: $demo ($sanitizer)"
    # A nonzero exit is expected, but inspect the diagnostic: sanitizer startup
    # failures are not evidence that a bug was detected.
    ASAN_OPTIONS=detect_leaks=1 "./build/unsafe-$demo" "$demo"
    ;;
  *) echo 'Usage: bash scripts/run.sh {safe|check-fail|panic|unsafe [uaf|double-free|iterator|overflow|race|leak|null]}'; exit 2;;
esac
