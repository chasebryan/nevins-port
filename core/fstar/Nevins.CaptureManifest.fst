module Nevins.CaptureManifest

open Nevins.Types
open Nevins.Policy

type capture_format_version =
  | NV1

type iq_format =
  | Unsigned8InterleavedIQ

type manifest = {
  version: capture_format_version;
  name: string;
  center_frequency_hz: frequency_hz;
  sample_rate_hz: sample_rate_hz;
  duration_ms: duration_ms;
  confidence: confidence;
  category: signal_category;
  iq_format: iq_format
}

let valid_category_name (s:string) : Tot bool =
  s = "PublicBroadcast" ||
  s = "Amateur" ||
  s = "Weather" ||
  s = "Satellite" ||
  s = "ISM" ||
  s = "AviationPublic" ||
  s = "Unknown" ||
  s = "RestrictedPrivate"

let valid_version_name (s:string) : Tot bool =
  s = "1"

let manifest_is_supported (m:manifest) : Tot bool =
  match m.version, m.iq_format with
  | NV1, Unsigned8InterleavedIQ -> true
