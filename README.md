# Nevins Port

Make the spectrum readable.

Nevins Port is a lawful radio operations dashboard for RTL-SDR that helps users scan, map, identify, record, replay, and understand radio signals with minimal prior RF knowledge. It is designed as a serious native listening post, signal notebook, and spectrum interpretation station for public, amateur, educational, and replayable IQ research workflows.

Nevins Port is receive-only. It does not transmit, jam, spoof, decrypt protected communications, extract credentials, or identify private persons from RF activity.

## Language Policy

The native application, dashboard, RTL-SDR integration, visualization, CLI, capture handling, and runtime shell are C++23. The verified core is specified in F*. C is reserved for future KaRaMeL output, narrow ABI bindings, and external C library interfaces such as librtlsdr. Shell scripts are limited to small build and verification glue.

This repository does not use Python, Rust, Go, JavaScript, TypeScript, Electron, Node, Java, C#, notebooks, or web-app scaffolding.

## Build

Preferred local build:

```sh
cmake -S . -B build -G Ninja -DNEVINS_BUILD_TESTS=ON -DNEVINS_BUILD_GUI=ON -DNEVINS_ENABLE_RTLSDR=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

If Ninja is unavailable, a local development build can use Unix Makefiles:

```sh
cmake -S . -B build -G "Unix Makefiles" -DNEVINS_BUILD_TESTS=ON -DNEVINS_BUILD_GUI=ON -DNEVINS_ENABLE_RTLSDR=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

The F*/KaRaMeL verification script is explicit:

```sh
./scripts/verify-fstar.sh
```

If `fstar.exe` or `krml` is unavailable, it fails with the required message and does not substitute unverified generated C.

## Mock-Mode Demo

```sh
./build/apps/nevins/nevins --version
./build/apps/nevins/nevins probe --mock
./build/apps/nevins/nevins survey --mock --duration 5 --out build/demo.nvcap
./build/apps/nevins/nevins explain build/demo.nvcap
./build/apps/nevins/nevins replay build/demo.nvcap
```

The dashboard binary runs against deterministic mock data:

```sh
./build/apps/nevins-console/nevins-console --mock
```

## Current Status

This first implementation pass provides the repository scaffold, native C++ CLI, GLFW/OpenGL mock dashboard with terminal fallback, mock SDR source, capture pack writer/reader, signal-card generation, conservative policy checks, F* specification modules, C++ tests, shell guards, and CI wiring. RTL-SDR hardware mode compiles behind `NEVINS_ENABLE_RTLSDR` and is not required for CI.

## Roadmap

1. Wire F*/KaRaMeL extraction into `generated/` and replace the temporary C++ mirror bridge with generated C-facing verified logic.
2. Add real librtlsdr capture behind explicit runtime selection.
3. Expand native dashboard rendering with Dear ImGui and ImPlot panels on top of the current GLFW/OpenGL mock dashboard.
4. Add richer DSP summaries while preserving conservative classification and receive-only policy boundaries.
5. Add capture search, comparison, and notebook workflows.
