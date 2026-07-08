# Capture Format NV1

Nevins Port uses directory-based capture packs with the `.nvcap` extension.

```text
capture-name.nvcap/
manifest.nv1
iq.u8
signal-card.md
spectrum.csv
notes.txt
```

## manifest.nv1

The manifest is a stable line-oriented key-value format intended for future F* parsing and validation.

Required initial fields:

```text
nv1.version=1
name=mock-survey
center_frequency_hz=433920000
sample_rate_hz=2048000
duration_ms=5000
confidence=68
category=ISM
modulation_guess=ook_ask_burst_like
iq_file=iq.u8
spectrum_file=spectrum.csv
signal_card_file=signal-card.md
notes_file=notes.txt
```

Unknown keys should be ignored by older readers unless they conflict with required fields. Required numeric fields must be bounded by the verified core policy.

## iq.u8

`iq.u8` stores raw interleaved unsigned 8-bit IQ samples for real RTL-SDR captures. Mock captures use deterministic IQ-like data with the same file shape.

## signal-card.md

`signal-card.md` contains the conservative plain-English signal card generated for the capture.

## spectrum.csv

`spectrum.csv` stores a small derived spectrum summary suitable for replay previews and future comparison tools.

## notes.txt

`notes.txt` is user-readable plain text. It must not claim private identity or content.
