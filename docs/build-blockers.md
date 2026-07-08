# Build Notes

The current environment could not install system packages because `sudo` requires an interactive password.

Ninja was installed user-locally at `/home/chasebryan/.local/bin/ninja` from the official Ninja Linux release zip, so the requested Ninja CMake acceptance command can run.

F* v2026.07.05 and KaRaMeL were installed user-locally under `/home/chasebryan/.local/opt/fstar`, with `fstar.exe` and `krml` linked into `/home/chasebryan/.local/bin`. F* verification runs for this repository. KaRaMeL extraction is not wired into the build yet.

PackageKit installed or confirmed the native development packages needed for this pass:

- GLFW development headers
- OpenGL development headers
- librtlsdr development package

Current impact:

- F* verification runs. KaRaMeL extraction remains a project wiring task and does not generate C in this pass.
- `nevins-console` now builds as a native GLFW/OpenGL mock dashboard and has a terminal fallback for headless sessions.
- The current Codex execution session does not expose a usable display, so the OpenGL smoke run falls back to terminal rendering here.
- Real RTL-SDR mode compiles and links when `NEVINS_ENABLE_RTLSDR=ON`, but real capture remains scaffolded and is not required for tests.

No web app or alternate-language dashboard has been added.
