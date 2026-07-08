# Architecture

Nevins Port is a native C++ radio operations console with a verified F* policy and validation core.

## Native C++ Shell

The `nevins` CLI is the operational shell for probing devices, running mock surveys, identifying conservative signal classes, explaining capture packs, and replaying captures through the same analysis pipeline. It is C++23 and does not require RTL-SDR hardware for mock-mode operation.

## Native Dashboard

The `nevins-console` dashboard is a native C++ binary. This first pass renders deterministic mock device, spectrum, waterfall, signal-card, capture-library, policy, and event-log panes through GLFW/OpenGL without web technologies. It includes a small built-in bitmap text renderer for pane labels and status values, plus a terminal fallback for headless sessions. Dear ImGui and ImPlot remain planned for richer controls.

## F* Verified Core

The F* core owns the safety-critical rules:

- bounded frequencies, sample rates, durations, gains, confidence scores, and scan plans
- public, amateur, weather, satellite, ISM, aviation-public, unknown, and restricted/private categories
- routing rules that prevent restricted/private or unknown signals from being sent to content decoding
- capture manifest and signal-card invariants

The C++ bridge currently mirrors these rules only as a temporary integration seam for tests and runtime behavior until KaRaMeL extraction is wired. It does not claim to be verified.

## KaRaMeL Extraction Path

F* modules under `core/fstar/` are the source-of-truth specifications. Future extraction will produce C under `generated/`, expose a narrow C ABI, and replace the temporary C++ bridge implementation. The repository intentionally does not contain fake generated C.

## RTL-SDR Device Adapter

The real device adapter is isolated behind `NEVINS_ENABLE_RTLSDR` and a runtime mode decision. It will use librtlsdr directly from C++ and will remain receive-only.

## Mock SDR Adapter

The mock SDR adapter generates deterministic IQ-like unsigned 8-bit interleaved samples. It supports CI, local development without hardware, capture generation, replay, and dashboard motion.

## Capture/Replay Pipeline

Capture packs use the `.nvcap` directory layout:

```text
capture-name.nvcap/
manifest.nv1
iq.u8
signal-card.md
spectrum.csv
notes.txt
```

Replay loads the manifest and IQ file, validates the basic metadata, and regenerates a signal card through the same analysis path used by live or mock capture.

## Signal Card Pipeline

Signal observations are converted into plain-English signal cards that include frequency, bandwidth estimate, SNR or power estimate, analysis window, modulation-class guess, confidence, reasons, safe next steps, non-assumptions, category, and policy status.

## Safety/Policy Classifier

The classifier is conservative. It may label broad signal shape classes such as `wide_fm_like`, `narrowband_like`, `ook_ask_burst_like`, `fsk_like`, `adsb_pulse_like`, `satellite_pass_like`, `unknown`, or `restricted_private_possible`. It does not decode protected content, infer owners, or route restricted/private classes to content decoding.
