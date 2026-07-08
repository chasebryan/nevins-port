module Nevins.Types

type frequency_hz = f:nat { 24000000 <= f /\ f <= 1766000000 }
type sample_rate_hz = r:nat { 250000 <= r /\ r <= 3200000 }
type duration_ms = d:nat { 1 <= d /\ d <= 3600000 }
type confidence = c:nat { c <= 100 }
type gain_db_tenths = g:int { -100 <= g /\ g <= 600 }
type bandwidth_hz = b:nat { 1 <= b /\ b <= 10000000 }
type scan_step_hz = s:nat { 1 <= s /\ s <= 10000000 }

type center_frequency = {
  center_hz: frequency_hz
}

type scan_range = {
  start_hz: frequency_hz;
  end_hz: frequency_hz
}

type device_limits = {
  min_frequency_hz: frequency_hz;
  max_frequency_hz: frequency_hz;
  min_sample_rate_hz: sample_rate_hz;
  max_sample_rate_hz: sample_rate_hz;
  max_duration_ms: duration_ms;
  max_steps: nat
}

let valid_frequency_hz (f:nat) : Tot bool =
  24000000 <= f && f <= 1766000000

let valid_sample_rate_hz (r:nat) : Tot bool =
  250000 <= r && r <= 3200000

let valid_duration_ms (d:nat) : Tot bool =
  1 <= d && d <= 3600000

let valid_confidence (c:nat) : Tot bool =
  c <= 100

let valid_gain_db_tenths (g:int) : Tot bool =
  -100 <= g && g <= 600

let valid_bandwidth_hz (b:nat) : Tot bool =
  1 <= b && b <= 10000000

let valid_scan_step_hz (s:nat) : Tot bool =
  1 <= s && s <= 10000000

let valid_scan_range (r:scan_range) : Tot bool =
  r.start_hz < r.end_hz

let within_device_frequency_limits (limits:device_limits) (f:frequency_hz) : Tot bool =
  limits.min_frequency_hz <= f && f <= limits.max_frequency_hz

let within_device_sample_rate_limits (limits:device_limits) (r:sample_rate_hz) : Tot bool =
  limits.min_sample_rate_hz <= r && r <= limits.max_sample_rate_hz
