# F* Core

The verified core is specified under `core/fstar/`.

Responsibilities:

- validate bounded frequencies, sample rates, durations, scan steps, gain ranges, and confidence scores
- define safe and restricted signal categories
- enforce conservative action routing
- validate finite scan plans
- validate NV1 capture manifests
- validate signal-card guidance, especially for restricted/private categories

The first pass provides F* modules as source-of-truth specifications and invariants. The C++ bridge is a temporary mirror used only until generated C from KaRaMeL is available.

Run:

```sh
./scripts/verify-fstar.sh
```

If `fstar.exe` or `krml` is unavailable, the script fails clearly with:

```text
F*/KaRaMeL toolchain unavailable. Do not replace verified core with unverified fallback.
```

The build must not silently replace verified core behavior with generated-looking unverified C.
