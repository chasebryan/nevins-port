#!/usr/bin/env bash
set -euo pipefail

generator="${NEVINS_CMAKE_GENERATOR:-Unix Makefiles}"

cmake -S . -B build -G "${generator}" \
  -DNEVINS_BUILD_TESTS=ON \
  -DNEVINS_BUILD_GUI=ON \
  -DNEVINS_ENABLE_RTLSDR=OFF

cmake --build build
ctest --test-dir build --output-on-failure
