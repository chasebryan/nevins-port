#pragma once

#include <cstdint>

extern "C" {

typedef enum NvSignalCategory {
  NV_SIGNAL_PUBLIC_BROADCAST = 0,
  NV_SIGNAL_AMATEUR = 1,
  NV_SIGNAL_WEATHER = 2,
  NV_SIGNAL_SATELLITE = 3,
  NV_SIGNAL_ISM = 4,
  NV_SIGNAL_AVIATION_PUBLIC = 5,
  NV_SIGNAL_UNKNOWN = 6,
  NV_SIGNAL_RESTRICTED_PRIVATE = 7
} NvSignalCategory;

typedef enum NvActionCategory {
  NV_ACTION_EXPLAIN_ONLY = 0,
  NV_ACTION_RECORD_IQ = 1,
  NV_ACTION_REPLAY = 2,
  NV_ACTION_ROUTE_TO_EXTERNAL_TOOL = 3,
  NV_ACTION_DECODE_CONTENT = 4
} NvActionCategory;

typedef struct NvDeviceLimits {
  std::uint64_t min_frequency_hz;
  std::uint64_t max_frequency_hz;
  std::uint32_t min_sample_rate_hz;
  std::uint32_t max_sample_rate_hz;
  std::uint32_t max_duration_ms;
  std::uint32_t max_steps;
} NvDeviceLimits;

typedef struct NvScanPlan {
  std::uint64_t start_frequency_hz;
  std::uint64_t end_frequency_hz;
  std::uint32_t step_hz;
  std::uint32_t sample_rate_hz;
  std::uint32_t dwell_ms;
} NvScanPlan;

const char* nv_core_version(void);
NvDeviceLimits nv_default_device_limits(void);

int nv_validate_frequency_hz(std::uint64_t frequency_hz);
int nv_validate_sample_rate_hz(std::uint32_t sample_rate_hz);
int nv_validate_duration_ms(std::uint32_t duration_ms);
int nv_validate_confidence(int confidence);
int nv_validate_gain_db_tenths(int gain_db_tenths);
int nv_validate_scan_plan(const NvScanPlan* plan, const NvDeviceLimits* limits);

int nv_policy_allows_action(NvSignalCategory category, NvActionCategory action);
const char* nv_signal_category_name(NvSignalCategory category);
const char* nv_action_category_name(NvActionCategory action);
NvSignalCategory nv_signal_category_from_name(const char* name);

}
