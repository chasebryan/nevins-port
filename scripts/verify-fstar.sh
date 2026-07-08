#!/usr/bin/env bash
set -euo pipefail

missing=0

if ! command -v fstar.exe >/dev/null 2>&1; then
  missing=1
fi

if ! command -v krml >/dev/null 2>&1; then
  missing=1
fi

if [[ "${missing}" -ne 0 ]]; then
  printf 'F*/KaRaMeL toolchain unavailable. Do not replace verified core with unverified fallback.\n' >&2
  exit 1
fi

fstar.exe --include core/fstar core/fstar/Nevins.Types.fst
fstar.exe --include core/fstar core/fstar/Nevins.Policy.fst
fstar.exe --include core/fstar core/fstar/Nevins.ScanPlan.fst
fstar.exe --include core/fstar core/fstar/Nevins.CaptureManifest.fst
fstar.exe --include core/fstar core/fstar/Nevins.SignalCard.fst

printf 'F* verification completed. KaRaMeL extraction is not wired in this pass.\n'
