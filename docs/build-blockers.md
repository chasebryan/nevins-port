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
- The OpenGL console smoke run succeeds in the current desktop session.
- Real RTL-SDR mode compiles and links when `NEVINS_ENABLE_RTLSDR=ON`. Receive-only hardware capture has been smoke-tested with an RTL-SDR Blog V4 device.

No web app or alternate-language dashboard has been added.
