#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

bad_files="$(find "${root}" \
  \( -path "${root}/.git" -o -path "${root}/build" -o -path "${root}/.cache" \) -prune -o \
  \( -name '*.py' -o -name 'requirements.txt' -o -name 'pyproject.toml' -o -name 'Pipfile' -o -name 'poetry.lock' -o -name '*.ipynb' \) \
  -print)"

if [[ -n "${bad_files}" ]]; then
  printf 'Forbidden language or notebook files found:\n%s\n' "${bad_files}" >&2
  exit 1
fi

build_refs="$(find "${root}" \
  \( -path "${root}/.git" -o -path "${root}/build" -o -path "${root}/.cache" \) -prune -o \
  -type f \
  \( -name 'CMakeLists.txt' -o -path "${root}/cmake/*" -o -path "${root}/scripts/*" -o -path "${root}/.github/workflows/*" \) \
  ! -path "${root}/scripts/verify-no-python.sh" \
  -print0 | xargs -0 grep -InEi 'python|conda' || true)"

if [[ -n "${build_refs}" ]]; then
  printf 'Forbidden build or CI references found:\n%s\n' "${build_refs}" >&2
  exit 1
fi

printf 'Repository language guard passed.\n'
