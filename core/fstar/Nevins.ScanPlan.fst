module Nevins.ScanPlan

open Nevins.Types

type scan_plan = {
  range: scan_range;
  step_hz: scan_step_hz;
  sample_rate: sample_rate_hz;
  dwell_ms: duration_ms
}

let span_hz (p:scan_plan) : Tot nat =
  if p.range.start_hz < p.range.end_hz
  then p.range.end_hz - p.range.start_hz
  else 0

let planned_steps (p:scan_plan) : Tot nat =
  1 + (span_hz p / p.step_hz)

let valid_scan_plan (limits:device_limits) (p:scan_plan) : Tot bool =
  valid_scan_range p.range &&
  within_device_frequency_limits limits p.range.start_hz &&
  within_device_frequency_limits limits p.range.end_hz &&
  within_device_sample_rate_limits limits p.sample_rate &&
  p.dwell_ms <= limits.max_duration_ms &&
  0 < limits.max_steps &&
  planned_steps p <= limits.max_steps
